#pragma once
#include <linux/gpio.h>
#include <DriverBus.hpp>

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

namespace comms { 

class FlowmeterComms {
public: 
    FlowmeterComms(const std::string &chip_path, 
                   unsigned int line, 
                   core::ThreadSafeDeque<uint64_t> &out);   
    
    ~FlowmeterComms();

    FlowmeterComms(const FlowmeterComms&) = delete;
    FlowmeterComms& operator=(const FlowmeterComms&) = delete;

private:
    void _read_loop();

    core::ThreadSafeDeque<uint64_t> &_out;
    int _chip_fd = -1;
    int _line_fd = -1;
    std::atomic<bool> _running{false};
    std::thread _thread;
};

}
