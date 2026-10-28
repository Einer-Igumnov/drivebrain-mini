#include <FoxgloveServer.hpp>
#include <MCAPLogger.hpp>
#include <drivebrain_msgs.pb.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <DriverBus.hpp>
#include <FlowmeterComms.hpp>
#include <FlowmeterProcessor.hpp>


std::atomic<bool> running{true};

void sig_handler(int signal) {
    if(signal == SIGINT || signal == SIGTERM) {
      spdlog::warn("Interrupted, stopping mini-drivebrain");
      running = false;
    }
}

int main() {
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);

    core::MCAPLogger::create(mcap::McapWriterOptions(""));
    core::FoxgloveServer::create();

    core::MCAPLogger::instance().open_new_mcap();
    core::MCAPLogger::instance().init_logging();

    spdlog::info("Constructed logging singletons");

    auto loop_time = 0.004;
    std::chrono::microseconds loop_time_ms((int) (loop_time * 1000000.0f));
    auto next_tick = std::chrono::steady_clock::now();

    try {

      core::ThreadSafeDeque<uint64_t> pulses;
      comms::FlowmeterProcessor processor(pulses);
      comms::FlowmeterComms flowmeter("/dev/gpiochip0", 12, pulses);


      while(running) {

          next_tick += loop_time_ms;

          auto msg = std::make_shared<drivebrain_msgs::FlowmeterData>();
          msg->set_pulse_count(processor.pulse_count());
          msg->set_last_pulse_ns(processor.last_pulse_ns());

          core::MCAPLogger::instance().log_msg(msg);
          core::FoxgloveServer::instance().send_live_telem_msg(msg);

          auto now = std::chrono::steady_clock::now();
          if(now > next_tick) {
            spdlog::warn("Loop overrun by {}", now-next_tick);
            next_tick = now;
          }

          std::this_thread::sleep_until(next_tick);
      }

    } catch (const std::exception &e) {
        spdlog::error("Flowmeter error: {}", e.what());
    }

    core::MCAPLogger::instance().destroy();
    core::FoxgloveServer::instance().destroy();
    spdlog::info("Logging singletons safely destroyed");

    return 0;
}
