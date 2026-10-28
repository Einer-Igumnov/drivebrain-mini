#pragma once
#include <DriverBus.hpp>

#include <atomic>
#include <cstdint>
#include <thread>

namespace comms {

class FlowmeterProcessor {
public:
    FlowmeterProcessor(core::ThreadSafeDeque<uint64_t> &in);
    ~FlowmeterProcessor();

    FlowmeterProcessor(const FlowmeterProcessor&) = delete;
    FlowmeterProcessor& operator=(const FlowmeterProcessor&) = delete;

    uint64_t pulse_count() const { return _pulse_count.load(); }
    uint64_t last_pulse_ns() const { return _last_pulse_ns.load(); }

private:
    void _process_loop();

    core::ThreadSafeDeque<uint64_t> &_in;

    std::atomic<uint64_t> _pulse_count{0};
    std::atomic<uint64_t> _last_pulse_ns{0};

    std::atomic<bool> _running{false};
    std::thread _thread;
};

}