#include "timer.h"

#include <chrono>
#include <thread>

void LinuxTimer::sleepMs(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}