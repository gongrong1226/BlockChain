#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class Queue
{
public:
	Queue() = default;
	Queue(Queue&&) = default;

	void push(T&& item) {
		std::unique_lock<std::mutex> lock(lock_); //上锁,src:_Pmtx->lock();
		queue_.push_back(item);
		//自动调用unlock()释放锁、并且阻塞等待另外的notify_*，此时唤醒并Lock()
		cond_.notify_one();
	}

	void push(const T& item) {
		std::unique_lock<std::mutex> lock(lock_);
		queue_.push_back(item);
		cond_.notify_one();
	}

	bool empty() {
		std::unique_lock<std::mutex> lock(lock_);
		return queue_.empty();
	}

	size_t count() {
		std::unique_lock<std::mutex> lock(lock_);
		return queue_.size();
	}

	void clear() {
		std::unique_lock<std::mutex> lock(lock_);
		std::deque<T> tmp;
		queue_.swap(tmp);
	}

	bool pop(T& front) {
		std::unique_lock<std::mutex> lock(lock_);
		if (!queue_.empty()) {
			front = queue_.front();
			queue_.pop_front();
			return true;
		}
		return false;
	}

	bool pop_wait(T& front) {
		std::unique_lock<std::mutex> lock(lock_);
		if (queue_.empty()) {
			cond_.wait(lock);
		}
		if (!queue_.empty()) {
			front = queue_.front();
			queue_.pop_front();
			return true;
		}
		return false;
	}

protected:
	mutable std::mutex lock_;
	std::condition_variable cond_;
	std::deque<T> queue_;
};
