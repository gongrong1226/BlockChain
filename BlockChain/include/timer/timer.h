#pragma once
#include <vector>
#include <functional>
#include <thread>
#include <iostream>
#include <sys/time.h>

namespace timer {
	class Timer;

class TimerManager
{
public:
	//得到毫秒时间
    static unsigned long long getCurrentMillisecs();

	void start();
	void stop(void);

	//检查定时器
    void detectTimers();

	TimerManager();
	virtual ~TimerManager();

private:
    friend class Timer;
    void addTimer(Timer* timer);
    void removeTimer(Timer* timer);
 
    void upHeap(size_t index);
    void downHeap(size_t index);
    void swapHeap(size_t, size_t index2);
 
private:
    struct HeapEntry
    {
        unsigned long long time;
        Timer* timer;
    };
	const int SLEEP_MS = 1000;
    std::vector<HeapEntry> heap_;
	std::thread thread_;
};
class TimerManager;
 
class Timer
{
public:
    enum TimerType { ONCE, CIRCLE };
	Timer(void);
    Timer(TimerManager& manager);
    ~Timer();
 
    void init(std::function<void(void)> && fun, const unsigned interval, TimerType timeType = CIRCLE);
    void stop();
 
private:
    void onTimer(unsigned long long now);
 
private:
    friend class TimerManager;
    TimerManager& manager_;
    TimerType timerType_;					//单次执行或循环
    std::function<void(void)> timerFun_;	//回调函数
    unsigned interval_;						//等待时间
    unsigned long long expires_;			//触发时间
 
    size_t heapIndex_;
};

inline void Timer::init(std::function<void(void)> && fun, const unsigned interval_ms, TimerType timeType)
{
	stop();
	interval_ = interval_ms;
	timerFun_ = fun;
	timerType_ = timeType;
	expires_ = interval_ + TimerManager::getCurrentMillisecs();
	manager_.addTimer(this);
} 
}