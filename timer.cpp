// Measure elapsed time to high resolution.
//
// Copyright (C) 1998, 2000, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: timer.cpp,v 1.3 2005-04-21 16:11:47 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "timer.hpp"

#include <sstream>
#include <stdexcept>

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
    // increase compile times for small programs.
#   ifndef LMI_MS_HEADER_INCLUDED
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
    if(0 == frequency_)
        {
        throw std::runtime_error("High resolution timer not available.");
        }
    Start();
}

//============================================================================
Timer::~Timer()
{
}

//============================================================================
Timer& Timer::Start()
{
    if(is_running_)
        {
        throw std::logic_error
            ("Timer::Start() called, but timer was already running."
            );
        }

    is_running_ = true;
    time_when_started_ = inspect();
    return *this;
}

//============================================================================
Timer& Timer::Stop()
{
    if(!is_running_)
        {
        throw std::logic_error
            ("Timer::Stop() called, but timer was not running."
            );
        }

    is_running_ = false;
    time_when_stopped_ = inspect();
    elapsed_time_ += time_when_stopped_ - time_when_started_;
    return *this;
}

//============================================================================
Timer& Timer::Restart()
{
    elapsed_time_ = 0;
    return Start();
}

//============================================================================
double Timer::Result()
{
    if(is_running_)
        {
        throw std::logic_error
            ("Timer::Result() called, but timer is still running."
            );
        }

    // The static_cast is necessary in case elapsed_t is integral.
    // It is impossible for frequency_ to be zero.
    return static_cast<double>(elapsed_time_) / frequency_;
}

//============================================================================
std::string Timer::Report()
{
    std::ostringstream oss;
    oss << static_cast<int>(1000.0 * Result());
    oss << " milliseconds ";
    return oss.str().c_str();
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
elapsed_t Timer::inspect()
{
#if defined LMI_POSIX
    timeval x;
    gettimeofday(&x, 0);
    return 1000000 * x.tv_sec + x.tv_usec;
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

