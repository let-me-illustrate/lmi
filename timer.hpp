// Measure elapsed time to high resolution.
//
// Copyright (C) 1998, 2000, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: timer.hpp,v 1.3 2005-04-21 15:22:16 chicares Exp $

// Boost provides a timer class, but they deliberately chose to use
// only a low-resolution timer. Their rationale is apparently that
// high-resolution timers may be more precise than they are accurate,
// and that latency is a significant concern. This class uses a high-
// resolution timer if available; it's a sharp tool that lets you
// make your own decision about that rationale.

// TODO ?? Consider whether boost's timer class has a better api.

#ifndef timer_hpp
#define timer_hpp

#include "config.hpp"

#include "expimp.hpp"

#if defined LMI_POSIX
#   include <sys/time.h> // gettimeofday()
    typedef double elapsed_t;
#elif defined LMI_MSW
    // Compilers for this platform use various types for its high-
    // resolution timer, but they use the same platform API, so
    // it's sufficient to use the same 64-bit integer type for all.
    //
    // Type double can't be used, even though it be the right size:
    // arithmetic performed on this type requires that it be integral.
    //
#   ifndef __BORLANDC__
    typedef unsigned long long int elapsed_t;
#   else // __BORLANDC__
    typedef unsigned __int64 elapsed_t;
#   endif // __BORLANDC__
#else // Unknown platform.
#   include <ctime>
    typedef std::clock_t elapsed_t;
#endif // Unknown platform.

#include <string>

class LMI_EXPIMP Timer
{
  public:
    Timer(bool autostart = true);

    Timer&      Start();
    Timer&      Stop();
    Timer&      Reset();
    double      Result();
    std::string Report();

  private:
    elapsed_t   calibrate();
    elapsed_t   inspect();

    bool        is_running;
    elapsed_t   freq;
    elapsed_t   start;
    elapsed_t   end;
    elapsed_t   elapsed;
    long int    iterations;
};

#endif // timer_hpp

