#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <utility>

template <typename T>
class vlock {
	std::mutex m;
	std::condition_variable cv;
	bool ready = false;
	T val;
public:
	void set_callback(T val) {
		{
			std::lock_guard lock(m);
			this->val = std::move(val);
			ready = true;
		}
		cv.notify_one();
	}

	T wait(int timeout_ms = 15000) {
		std::unique_lock lk(m);
		if (timeout_ms > 0) {
			cv.wait_for(lk, std::chrono::milliseconds(timeout_ms), [this]() { return ready; });
		} else {
			cv.wait(lk, [this]() { return ready; });
		}

		return val;
	}
};
