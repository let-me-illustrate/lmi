// Measure elapsed time to high resolution.
//
// Copyright (C) 1998, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "timer.hpp"

#include "bourn_cast.hpp"

#if defined LMI_POSIX
#   include <sys/time.h>                // gettimeofday()
#elif defined LMI_MSW
    // TRICKY !! There being no standard way to ascertain whether
    // <windows.h> has been included, we resort to this hack:
#   if defined STDCALL || defined WM_COMMAND
#       define LMI_MS_HEADER_INCLUDED
#   endif // defined STDCALL || defined WM_COMMAND
    // I'm not willing to bring in a zillion msw headers...
    //#if defined LMI_MSW
    //#   include <windows.h>
    //#endif // defined LMI_MSW
    // ...just to get a couple of prototypes, because that can materially
    // increase compile times for small programs, and because it requires
    // ms extensions and defines many macros.
#   if !defined LMI_MS_HEADER_INCLUDED
#       include <cstdint>
        typedef std::uint64_t msw_elapsed_t;
        // These declarations would be erroneous if the ms headers were
        // included. It's necessary to guard against that explicitly,
        // because those headers might be implicitly included by a pch
        // mechanism.
        extern "C" int __stdcall QueryPerformanceCounter  (msw_elapsed_t*);
        extern "C" int __stdcall QueryPerformanceFrequency(msw_elapsed_t*);
#   endif // !defined LMI_MS_HEADER_INCLUDED
#else // Unknown platform.
#   include <ctime>                     // clock()
#endif // Unknown platform.

/// Suspend execution for a given number of seconds.

#if defined LMI_POSIX
#   include <unistd.h>                  // sleep()
void lmi_sleep(int seconds) {sleep(bourn_cast<unsigned int>(seconds));}
#elif defined LMI_MSW
#   if !defined LMI_MS_HEADER_INCLUDED
extern "C" void __stdcall Sleep(unsigned int);
#   endif // !defined LMI_MS_HEADER_INCLUDED
void lmi_sleep(int seconds) {Sleep(bourn_cast<unsigned int>(1000 * seconds));}
#else // Unknown platform.
#   error Unknown platform.
#endif // Unknown platform.

/// Create, calibrate, and start a timer.
///
/// Postcondition: 0 < frequency_. Throws if a positive frequency_
/// cannot be determined.

Timer::Timer()
    :elapsed_time_      {0}
    ,is_running_        {false}
    ,time_when_started_ {0}
    ,time_when_stopped_ {0}
{
    frequency_ = calibrate();
    if(frequency_ <= 0)
        {
        throw std::runtime_error("High resolution timer not available.");
        }
    start();
}

/// Set elapsed time to zero, and restart timer.

Timer& Timer::restart()
{
    elapsed_time_ = 0;
    start();
    return *this;
}

/// Stop timer and mark elapsed time. Throws if timer was not running.

Timer& Timer::stop()
{
    if(!is_running_)
        {
        throw std::logic_error
            ("Timer::stop() called, but timer was not running."
            );
        }

    is_running_ = false;
    time_when_stopped_ = inspect();
    elapsed_time_ += time_when_stopped_ - time_when_started_;
    return *this;
}

/// Format argument as a string representing integral milliseconds.

std::string Timer::elapsed_msec_str(double seconds)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << 1000.0 * seconds;
    oss << " milliseconds";
    return oss.str();
}

/// Elapsed time as a string representing integral milliseconds.

std::string Timer::elapsed_msec_str() const
{
    return elapsed_msec_str(elapsed_seconds());
}

/// Elapsed time in seconds.
///
/// Preconditions:
///  - Timer must have been stopped; throws if it is still running.
///  - frequency_ must be nonzero (guaranteed by ctor), so that
///    dividing by it is safe.

double Timer::elapsed_seconds() const
{
    if(is_running_)
        {
        throw std::logic_error
            ("Timer::elapsed_seconds() called, but timer is still running."
            );
        }

    return elapsed_time_ / frequency_;
}

/// Ascertain timer frequency in ticks per second.

double Timer::calibrate()
{
#if defined LMI_POSIX
    return 1000000.0;
#elif defined LMI_MSW
#   if defined LMI_MS_HEADER_INCLUDED
    LARGE_INTEGER z;
    QueryPerformanceFrequency(&z);
    return static_cast<double>(z.QuadPart);
#   else  // !defined LMI_MS_HEADER_INCLUDED
    msw_elapsed_t z;
    QueryPerformanceFrequency(&z);
    return static_cast<double>(z);
#   endif // !defined LMI_MS_HEADER_INCLUDED
#else // Unknown platform.
    return static_cast<double>(CLOCKS_PER_SEC);
#endif // Unknown platform.
}

/// Start timer. Throws if it was already running.

void Timer::start()
{
    if(is_running_)
        {
        throw std::logic_error
            ("Timer::start() called, but timer was already running."
            );
        }

    is_running_ = true;
    time_when_started_ = inspect();
}

/// Ticks elapsed since timer started.

double Timer::inspect() const
{
#if defined LMI_POSIX
    timeval x;
    gettimeofday(&x, nullptr);
    return
                      bourn_cast<double>(x.tv_usec)
        + 1000000.0 * bourn_cast<double>(x.tv_sec)
        ;
#elif defined LMI_MSW
#   if defined LMI_MS_HEADER_INCLUDED
    LARGE_INTEGER z;
    QueryPerformanceCounter(&z);
    return static_cast<double>(z.QuadPart);
#   else  // !defined LMI_MS_HEADER_INCLUDED
    msw_elapsed_t z;
    QueryPerformanceCounter(&z);
    return static_cast<double>(z);
#   endif // !defined LMI_MS_HEADER_INCLUDED
#else // Unknown platform.
    return static_cast<double>(std::clock());
#endif // Unknown platform.
}

#undef LMI_MS_HEADER_INCLUDED
