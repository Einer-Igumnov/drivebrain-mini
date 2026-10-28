#include <FlowmeterProcessor.hpp>
#include <deque>
#include <mutex>

namespace comms {

FlowmeterProcessor::FlowmeterProcessor(core::ThreadSafeDeque<uint64_t> &in): _in(in) {
    _running = true;
    _thread = std::thread(&FlowmeterProcessor::_process_loop, this);
}

FlowmeterProcessor::~FlowmeterProcessor() {
    { 
        std::lock_guard<std::mutex> lock(_in.mtx); 
        _running = false; 
    }

    _in.cv.notify_all();
    if (_thread.joinable()) {
        _thread.join();
    }
}

void FlowmeterProcessor::_process_loop() {
    std::deque<uint64_t> local;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(_in.mtx);

            _in.cv.wait(lock, [this] {
                return !_in.deque.empty() || !_running;
            });

            if (!_running && _in.deque.empty()) {
                break;
            }

            local.swap(_in.deque);
        }

        _pulse_count += local.size();
        _last_pulse_ns = local.back();
        local.clear();
    }
}


}
