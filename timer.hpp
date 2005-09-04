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

// $Id: timer.hpp,v 1.8 2005-09-04 17:05:30 chicares Exp $

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

#include <boost/utility.hpp>

#include <algorithm> // std::min()
#include <climits>   // ULONG_MAX
#include <cmath>     // std::log10(), std::pow()
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

template<typename F> std::string aliquot_timer(F, double = 1.0);

/// Why another timer class?
///
/// Boost provides a timer class, but they deliberately chose to use
/// only a low-resolution timer. Their rationale is apparently that
/// high-resolution timers may be more precise than they are accurate,
/// and that latency is a significant concern. This class uses a high-
/// resolution timer if available; it's a sharp tool that lets you
/// make your own decision about that rationale.
///
/// Class timer design.
///
/// ctor postconditions: frequency_ != 0. Throws if a nonzero
/// frequency_ cannot be determined.
///
/// elapsed_msec_str(), elapsed_usec(), stop(), restart(): nomen est omen.

class LMI_EXPIMP Timer
    :private boost::noncopyable
{
    template<typename F> friend std::string aliquot_timer(F, double);

  public:
    Timer();
    ~Timer();

    Timer&      restart();
    Timer&      stop();

    std::string elapsed_msec_str() const;
    double      elapsed_usec() const;

  private:
    elapsed_t   calibrate();
    void        start();

    elapsed_t   inspect() const;

    elapsed_t   elapsed_time_;
    elapsed_t   frequency_;
    bool        is_running_;
    elapsed_t   time_when_started_;
    elapsed_t   time_when_stopped_;
};

/// Design of function template aliquot_timer().
///
/// aliquot_timer() reports how long an operation takes, dynamically
/// adjusting the number of iterations measured to balance accuracy
/// with a desired limit on total time for the measurement.
///
/// Execute the operation once and observe how long it took. Repeat
/// the operation as many times as that observation indicates it can
/// be repeated in the time interval specified, but rounding the
/// number of iterations down to the next-lower power of ten so that
/// the reported timing and iteration count can be divided at sight.
///
/// If the operation took longer than the specified interval in the
/// initial calibration trial, then just report how long that took.
/// Rationale: if it is desired to spend one second testing an
/// operation, but the operation takes ten seconds, then it's not
/// appropriate to spend another ten seconds for a single iteration.
///
/// If the operation took no measurable amount of time, set the number
/// of iterations to the number of timer quanta in the specified
/// interval. Rationale: the initial calibration trial could have
/// taken just less than one quantum, and the specified interval
/// should not be exceeded.
///
/// Template parameter 'F' either is a nullary function or behaves
/// like one; boost::bind() is useful for reducing the arity of the
/// template argument (see unit test). Naturally, this is subject to
/// the Forwarding Problem, but that's inherent in the language.
///
/// Parameter 'seconds' is the desired limit on measurement time,
/// in seconds. It is approximately respected iff the operation
/// takes no longer than that limit. The default is one second, which
/// is generally long enough to get a stable measurement.
///
/// This function template is a friend of class Timer so that it can
/// access Timer::frequency_, which should not have a public accessor
/// because its type is platform dependent.
///
/// Implementation of function template aliquot_timer().
///
/// Class Timer guarantees that its frequency_ member is nonzero, so
/// it is safe to divide by that member.
///
/// An intermediate value is volatile-qualified in order to work
/// around a defect observed with MinGW gcc: the defective ms C
/// runtime library MinGW uses doesn't reliably return integer
/// results for std::pow() with exact-integer arguments.
///
/// It might be nicer to make this a non-template nullary function
/// and move its definition out of the header. The problem there is
/// that the type of a boost::bind() expression is unspecified.
///
/// Function names should be verb phrases, but English seems to lack
/// a one-word transitive verb for chronometry. Maybe someday this
/// function will become a functor anyway.

template<typename F>
std::string aliquot_timer(F f, double seconds)
{
    Timer timer;
    f();
    timer.stop();
    double elapsed = timer.elapsed_usec();
    double const v =
        (0.0 != elapsed)
        ? seconds / elapsed
        : seconds * timer.frequency_
        ;

    double const w = std::min(std::log10(v), static_cast<double>(ULONG_MAX));
    unsigned long int const x = static_cast<unsigned long int>(w);
    double const volatile y = std::pow(10.0, static_cast<double>(x));
    unsigned long int const z = static_cast<unsigned long int>(y);
    if(1 < z)
        {
        timer.restart();
        for(unsigned long int j = 0; j < z; j++)
            {
            f();
            }
        timer.stop();
        }
    std::ostringstream oss;
    oss
        << std::scientific << std::setprecision(3)
        << "[" << timer.elapsed_usec() / z << "] "
        << z
        << " iteration" << ((1 == z) ? "" : "s") << " took "
        << timer.elapsed_msec_str()
        ;
    return oss.str();
}

#endif // timer_hpp

