#include "Platform.h"
#include "PLAMIO.h"

#include "pico/stdlib.h"
#include "pico/time.h"

using namespace PLAMIO;

uint32_t Platform::getMsec()
{
    return to_ms_since_boot(get_absolute_time());
}

uint64_t Platform::getUsec()
{
    return to_us_since_boot(get_absolute_time());
}

void Platform::sleepMsec(uint32_t msec)
{
    sleep_ms(msec);
}

void Platform::sleepUsec(uint32_t usec)
{
    sleep_us(usec);
}

bool Platform::elapsed(uint32_t now, uint32_t startMsec, uint32_t durationMsec)
{
    return static_cast<uint32_t>(now - startMsec) >= durationMsec;
}