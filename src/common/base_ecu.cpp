#include "base_ecu.h"

BaseEcu::BaseEcu(ICanDriver& driver)
    : driver_(driver)
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