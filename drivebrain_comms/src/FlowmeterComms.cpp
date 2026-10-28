#include <FlowmeterComms.hpp>
#include <fcntl.h>      
#include <unistd.h>     
#include <poll.h>       
#include <sys/ioctl.h>  
#include <cstring>      
#include <spdlog/spdlog.h>


namespace comms {

FlowmeterComms::FlowmeterComms(const std::string &chip_path, unsigned int line,
                               core::ThreadSafeDeque<uint64_t> &out)
    : _out(out)
{
    _chip_fd = open(chip_path.c_str(), O_RDWR | O_CLOEXEC);
    if (_chip_fd < 0) {
        spdlog::error("Failed to open {}: {}", chip_path, strerror(errno));
        throw std::runtime_error("could not open gpio chip");
    }
    gpio_v2_line_request req;
    std::memset(&req, 0, sizeof(req));

    req.offsets[0] = line;
    req.num_lines = 1;
    std::strncpy(req.consumer, "flowmeter", sizeof(req.consumer) - 1);
    req.config.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_RISING;

    if (ioctl(_chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0) {
        spdlog::error("Failed to request line {}: {}", line, strerror(errno));
        close(_chip_fd);
        _chip_fd = -1;
        throw std::runtime_error("could not request gpio line");
    }

    _line_fd = req.fd;
    
    _running = true;
    _thread = std::thread(&FlowmeterComms::_read_loop, this);
}

FlowmeterComms::~FlowmeterComms() {
    _running = false;

    if (_thread.joinable()) {
        _thread.join();
    }

    if (_line_fd >= 0) {
        close(_line_fd);
        _line_fd = -1;
    }

    if (_chip_fd >= 0) {
        close(_chip_fd);
        _chip_fd = -1;
    }
}

void FlowmeterComms::_read_loop() {
    pollfd pfd{};
    pfd.fd = _line_fd;
    pfd.events = POLLIN;

    while (_running) {
        int ret = poll(&pfd, 1, 100);

        if (ret < 0) {
            if (errno == EINTR) continue;
            spdlog::error("poll failed: {}", strerror(errno));
            break;
        }

        if (ret == 0) continue;

        gpio_v2_line_event event{};
        ssize_t n = read(_line_fd, &event, sizeof(event));

        if (n != sizeof(event)) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(_out.mtx);
            _out.deque.push_back(event.timestamp_ns);
        }
        
        _out.cv.notify_one();
    }
}

}
