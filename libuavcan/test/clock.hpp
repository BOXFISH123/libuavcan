/*
 * Copyright (C) 2014 <pavel.kirienko@gmail.com>
 */

#pragma once

#include <cassert>
#include <uavcan/system_clock.hpp>
#include <time.h>
#include <sys/time.h>

class SystemClockMock : public uavcan::ISystemClock
{
public:
    mutable uint64_t monotonic;
    mutable uint64_t utc;
    int64_t monotonic_auto_advance;

    SystemClockMock(uint64_t initial = 0)
    : monotonic(initial)
    , utc(initial)
    , monotonic_auto_advance(0)
    { }

    void advance(uint64_t usec) const
    {
        monotonic += usec;
        utc += usec;
    }

    uavcan::MonotonicTime getMonotonic() const
    {
        assert(this);
        const uint64_t res = monotonic;
        advance(monotonic_auto_advance);
        return uavcan::MonotonicTime::fromUSec(res);
    }

    uavcan::UtcTime getUtc() const
    {
        assert(this);
        return uavcan::UtcTime::fromUSec(utc);
    }

    void adjustUtc(uavcan::UtcTime, uavcan::UtcDuration)
    {
        assert(0);
    }
};


class SystemClockDriver : public uavcan::ISystemClock
{
public:
    uavcan::MonotonicTime getMonotonic() const
    {
        struct timespec ts;
        const int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (ret != 0)
        {
            assert(0);
            return uavcan::MonotonicTime();
        }
        return uavcan::MonotonicTime::fromUSec(uint64_t(ts.tv_sec) * 1000000UL + ts.tv_nsec / 1000UL);
    }

    uavcan::UtcTime getUtc() const
    {
        struct timeval tv;
        const int ret = gettimeofday(&tv, NULL);
        if (ret != 0)
        {
            assert(0);
            return uavcan::UtcTime();
        }
        return uavcan::UtcTime::fromUSec(uint64_t(tv.tv_sec) * 1000000UL + tv.tv_usec);
    }

    void adjustUtc(uavcan::UtcTime, uavcan::UtcDuration)
    {
        assert(0);
    }
};

inline uavcan::MonotonicTime tsMono(uint64_t usec) { return uavcan::MonotonicTime::fromUSec(usec); }
inline uavcan::UtcTime tsUtc(uint64_t usec) { return uavcan::UtcTime::fromUSec(usec); }

inline uavcan::MonotonicDuration durMono(int64_t usec) { return uavcan::MonotonicDuration::fromUSec(usec); }

template <typename T>
static bool areTimestampsClose(const T& a, const T& b, int64_t precision_usec = 10000)
{
    return (a - b).getAbs().toUSec() < precision_usec;
}