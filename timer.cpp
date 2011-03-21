// Measure elapsed time to high resolution.
//
// Copyright (C) 1998, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "timer.hpp"

#ifdef LMI_MSW
    // TRICKY !! There being no standard way to ascertain whether
    // <windows.h> has been included, we resort to this hack:
#   if defined STDCALL || defined WM_COMMAND
#       define LMI_MS_HEADER_INCLUDED
#   endif // defined STDCALL || defined WM_COMMAND
    // I'm not willing to bring in a zillion msw headers...
    //#ifdef LMI_MSW
    //#   include <windows.h>
    //#endif // LMI_MSW
    // ...just to get a couple of prototypes, because that can materially
    // increase compile times for small programs, and because it requires
    // ms extensions and defines many macros.
#   ifndef LMI_MS_HEADER_INCLUDED
    // These declarations would be erroneous if the ms headers were
    // included. It's necessary to guard against that explicitly,
    // because those headers might be implicitly included by a pch
    // mechanism.
        extern "C" int __stdcall QueryPerformanceCounter(elapsed_t*);
        extern "C" int __stdcall QueryPerformanceFrequency(elapsed_t*);
#   endif // LMI_MS_HEADER_INCLUDED
#endif // LMI_MSW

//============================================================================
Timer::Timer()
    :elapsed_time_      (0)
    ,is_running_        (false)
    ,time_when_started_ (0)
    ,time_when_stopped_ (0)
{
    frequency_ = calibrate();
    if(frequency_ <= 0)
        {
        throw std::runtime_error("High resolution timer not available.");
        }
    start();
}

//============================================================================
Timer::~Timer()
{
}

//============================================================================
Timer& Timer::restart()
{
    elapsed_time_ = 0;
    start();
    return *this;
}

//============================================================================
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

//============================================================================
std::string Timer::elapsed_msec_str(double z)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << 1000.0 * z;
    oss << " milliseconds";
    return oss.str();
}

//============================================================================
std::string Timer::elapsed_msec_str() const
{
    return elapsed_msec_str(elapsed_usec());
}

//============================================================================
double Timer::elapsed_usec() const
{
    if(is_running_)
        {
        throw std::logic_error
            ("Timer::elapsed_usec() called, but timer is still running."
            );
        }

    // The static_casts are necessary in case elapsed_t is integral.
    // It is impossible for frequency_ to be zero.
    return static_cast<double>(elapsed_time_) / static_cast<double>(frequency_);
}

//============================================================================
elapsed_t Timer::calibrate()
{
#if defined LMI_POSIX
    return 1000000;
#elif defined LMI_MSW
#   ifdef LMI_MS_HEADER_INCLUDED
    LARGE_INTEGER z;
    QueryPerformanceFrequency(&z);
    return z.QuadPart;
#   else
    elapsed_t z;
    QueryPerformanceFrequency(&z);
    return z;
#   endif // LMI_MS_HEADER_INCLUDED
#else // Unknown platform.
    return CLOCKS_PER_SEC;
#endif // Unknown platform.
}

//============================================================================
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

//============================================================================
elapsed_t Timer::inspect() const
{
#if defined LMI_POSIX
    timeval x;
    gettimeofday(&x, 0);
    return elapsed_t(1000000) * x.tv_sec + x.tv_usec;
#elif defined LMI_MSW
#   ifdef LMI_MS_HEADER_INCLUDED
    LARGE_INTEGER z;
    QueryPerformanceCounter(&z);
    return z.QuadPart;
#   else
    elapsed_t z;
    QueryPerformanceCounter(&z);
    return z;
#   endif // LMI_MS_HEADER_INCLUDED
#else // Unknown platform.
    return std::clock();
#endif // Unknown platform.
}

#undef LMI_MS_HEADER_INCLUDED

