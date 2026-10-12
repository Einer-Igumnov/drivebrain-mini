# mini-drivebrain: HyTech Software New Member Project

By now you should be familiar with our main compute module, [Drivebrain](https://github.com/hytech-racing/drivebrain).

## Project Description

You will have access to a Raspberry Pi 4/5 with a simple [GPIO Flowmeter](https://www.amazon.com/SENSTREE-Flowmeter-Counter-Raspberry-Compatible/dp/B0CC949527?crid=3OXKX37CDRH6N&dib=eyJ2IjoiMSJ9.8BK-EnwtzMdGuif8y2VUAxbmTMEjEGgJrQWGqkBSlJ0YqdnkDtuy7OCSTn9lAGr9o9mgnKotv7FiMCUUWdWV9CZEVpA6Nluq87zSjgs3lfmdjvAe9hL7xbKYmUSlEh-P1inXzqv3M2hL8XbQm8VmFpTJTnxVJ7uCM8NxvnYYyUlq3dZXqN3pBb6Yi-G2I8T1ylGRhBbkxLBJ5cKh7M2sb1JE2xYVp4qgRzejxV4jjhs.ofvImoiI2dtu_wUCzpCZE5lS4rRsIglMl8scU9XQMbk&dib_tag=se&keywords=flowmeters+io+arduino&qid=1789286364&sprefix=flowmeters+io+arduoin%2Caps%2C164&sr=8-1).


## Deliverables

- [ ] A completed `FlowmeterComms` interface that reads pulses from a Raspberry Pi GPIO pin of your choice
  - [ ] At least one new Protobuf message that can be passed into the existing `FoxgloveServer` and `MCAPLogger` log methods to view and replay your data on Foxglove Studio
  - [ ] Reads the pin in its own thread. You should have some resemblance of safe construction and deconstruction with respect to spawned threads.
  - [ ] Hands pulses to the rest of your program through a `core::ThreadSafeDeque`: push while holding its mutex, then notify its condition variable
- [ ] A thread that waits on that condition variable (`cv.wait`, not a busy loop) and handles the pulses as they come in. Drivebrain's `drivebrain_comms/src/ETHSendComms.cpp` waits on a `ThreadSafeDeque` the same way
  - [ ] Moves the pulses out of the deque while holding the lock, and handles them after releasing it
- [ ] A completed `CMakeLists.txt` to properly link and compile your Flowmeter driver code
- [ ] A loop in `main.cpp` to actively call your Flowmeter code
  - [ ] Reads the latest pulse data from your other thread safely (with a mutex or `std::atomic`) and logs it
- [ ] A clean shutdown on Ctrl-C and on `systemctl stop` (SIGINT and SIGTERM). Stop and join your threads before the logger is destroyed, so the MCAP file gets closed
- [ ] A `.service` file to automatically run your C++ binary on startup in the Pi
- [ ] Your data live in Foxglove at `ws://<pi>:5555`, and the MCAP from `/home/nixos/recordings` replays the same run

When you successfully complete and demo your completed code working in person, we'll ask you a few questions about what you wrote, how you validated it, etc. There are no "set list of questions" or "right answers" for this, we probably will just ask whatever comes to mind first depending on your implementation. If for any reason it is concerningly clear you have no idea what you are talking about or how your code works, we probably will not mark you as completed. 

## LLM Policy
We love using LLMs for research or writing a lot of redundant code, but for this project specifically we kindly ask that you do not use it to one-shot the project or write a majority of the code. Using it for small or redundant bits is fine, but there really is no point to any of this if you have no idea what is going on in this project (which is identical to the codebase you will spend most of your time on).


## Build
```bash
docker compose run --rm dev ./build_script.sh   # -> build/mini-drivebrain
```
The first build compiles the dependencies and takes a while.

## Run
```bash
scp build/mini-drivebrain nixos@<pi>:~/
ssh nixos@<pi>
mkdir -p ~/recordings   # MCAP files go to /home/nixos/recordings
sudo ./mini-drivebrain
```

In Foxglove, open a connection to `ws://192.168.203.67:5555`.

