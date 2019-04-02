#include <timer/timer.h>

namespace timer {

	Timer::Timer(TimerManager& manager)
		: manager_(manager)
		, heapIndex_(-1)
	{
	}

	Timer::~Timer()
	{
		stop();
	}

	void Timer::stop()
	{
		if (heapIndex_ != -1)
		{
			manager_.removeTimer(this);
			heapIndex_ = -1;
		}
	}

	void Timer::onTimer(unsigned long long now)
	{
		if (timerType_ == Timer::CIRCLE)
		{
			expires_ = interval_ + now;
			manager_.addTimer(this);
		}
		else
		{
			heapIndex_ = -1;
		}
		timerFun_();
	}

	//////////////////////////////////////////////////////////////////////////
	// TimerManager

	TimerManager::TimerManager() {
		//addTimer(timer);
	}
	TimerManager:: ~TimerManager() {
	}

	void TimerManager::addTimer(Timer* timer)
	{
		timer->heapIndex_ = heap_.size();
		HeapEntry entry = { timer->expires_, timer };
		heap_.push_back(entry);
		upHeap(heap_.size() - 1);
	}

	void TimerManager::removeTimer(Timer* timer)
	{
		size_t index = timer->heapIndex_;
		if (!heap_.empty() && index < heap_.size())
		{
			if (index == heap_.size() - 1)
			{
				heap_.pop_back();
			}
			else
			{
				swapHeap(index, heap_.size() - 1);
				heap_.pop_back();
				size_t parent = (index - 1) / 2;
				if (index > 0 && heap_[index].time < heap_[parent].time)
					upHeap(index);
				else
					downHeap(index);
			}
		}
	}


	void TimerManager::start() {
		thread_ = std::thread(std::bind(&TimerManager::detectTimers, this));
	}

	void TimerManager::stop(void) {
		//pthread_cancel(thread_.get_id());
		if (thread_.joinable()) {
			//pthread_join(thread_.get_id, NULL);
			thread_.join();
		}
		else {
			std::cout << "thread is not joinable" << std::endl;
		}
	}

	void TimerManager::detectTimers()
	{
		//pthread_detach(pthread_self()); //线程结束自动释放资源
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //设置本线程对Cancel信号的反应  CANCEL_ENABLE
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置本线程取消动作的执行时机 立即取消
		while (true) {
			unsigned long long now = getCurrentMillisecs();
			while (!heap_.empty() && heap_[0].time <= now)
			{
				Timer* timer = heap_[0].timer;
				removeTimer(timer);
				timer->onTimer(now);
			}
			if (!heap_.empty()) {
				unsigned long long now = getCurrentMillisecs();
				std::this_thread::sleep_for(std::chrono::milliseconds(abs(now - heap_[0].time)));
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
			}
		}
	}

	void TimerManager::upHeap(size_t index)
	{
		size_t parent = (index - 1) / 2;
		while (index > 0 && heap_[index].time < heap_[parent].time)
		{
			swapHeap(index, parent);
			index = parent;
			parent = (index - 1) / 2;
		}
	}

	void TimerManager::downHeap(size_t index)
	{
		size_t child = index * 2 + 1;
		while (child < heap_.size())
		{
			size_t minChild = (child + 1 == heap_.size() || heap_[child].time < heap_[child + 1].time)
				? child : child + 1;
			if (heap_[index].time < heap_[minChild].time)
				break;
			swapHeap(index, minChild);
			index = minChild;
			child = index * 2 + 1;
		}
	}

	void TimerManager::swapHeap(size_t index1, size_t index2)
	{
		HeapEntry tmp = heap_[index1];
		heap_[index1] = heap_[index2];
		heap_[index2] = tmp;
		heap_[index1].timer->heapIndex_ = index1;
		heap_[index2].timer->heapIndex_ = index2;
	}


	unsigned long long TimerManager::getCurrentMillisecs()
	{
#ifdef _MSC_VER
		_timeb timebuffer;
		_ftime(&timebuffer);
		unsigned long long ret = timebuffer.time;
		ret = ret * 1000 + timebuffer.millitm;
		return ret;
#else
		timeval tv;
		::gettimeofday(&tv, 0);
		unsigned long long ret = tv.tv_sec;
		return ret * 1000 + tv.tv_usec / 1000;
#endif
	}

}