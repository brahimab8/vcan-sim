#include "base_ecu.h"

BaseEcu::BaseEcu(ICanDriver& driver, ITimer& timer)
    : driver_(driver)
    , timer_(timer)
{
}

void BaseEcu::start()
{
    running_ = true;
}

void BaseEcu::stop()
{
    running_ = false;
}