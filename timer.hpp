// Measure elapsed time to high resolution.
//
// Copyright (C) 1998, 2000, 2001, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: timer.hpp,v 1.13 2007-01-09 15:53:25 chicares Exp $

#ifndef timer_hpp
#define timer_hpp

#include "config.hpp"

#include "so_attributes.hpp"

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

class LMI_SO Timer
    :private boost::noncopyable
{
    template<typename F> friend class AliquotTimer;

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

/// Design of class template AliquotTimer.
///
/// Time an operation, dynamically adjusting the number of iterations
/// measured to balance accuracy with a desired limit on total time
/// for the measurement.
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
/// Template parameter 'F' is the type of the first ctor parameter,
/// which either is a nullary function or behaves like one. A facility
/// such as boost::bind() is useful for reducing the arity of the
/// argument (see unit test). Naturally, this is subject to the
/// Forwarding Problem, but that's inherent in the language.
///
/// Ctor parameter 'max_seconds' is the desired limit on measurement
/// time, in seconds. If that limit is exceeded by the initial
/// calibration trial, then the operation is not run again. Otherwise,
/// the operation is repeated for (0.1 * 'max_seconds', 'max_seconds']
/// (more or less, to the extent that the initial calibration trial's
/// speed was atypical).
///
/// This class template is a friend of class Timer so that it can
/// access Timer::frequency_, which should not have a public accessor
/// because its type is platform dependent.
///
/// Implementation of class template AliquotTimer.
///
/// Class Timer guarantees that its frequency_ member is nonzero, so
/// it is safe to divide by that member.
///
/// An intermediate value is volatile-qualified in order to work
/// around a defect observed with MinGW gcc: the defective ms C
/// runtime library MinGW uses doesn't reliably return integer
/// results for std::pow() with exact-integer arguments.

template<typename F>
class AliquotTimer
{
    friend class TimerTest;

  public:
    AliquotTimer(F f, double max_seconds);
    std::string operator()();

  private:
    static unsigned long int GreatestNonnegativePowerOfTen(double);

    F      f_;
    double max_seconds_;
    Timer  timer_;
    double initial_trial_time_;
};

template<typename F>
AliquotTimer<F>::AliquotTimer(F f, double max_seconds)
    :f_          (f)
    ,max_seconds_(max_seconds)
{
    f_();
    timer_.stop();
    initial_trial_time_ = timer_.elapsed_usec();
}

template<typename F>
std::string AliquotTimer<F>::operator()()
{
    double const v =
        (0.0 != initial_trial_time_)
        ? max_seconds_ / initial_trial_time_
        : max_seconds_ * timer_.frequency_
        ;
    unsigned long int const z = GreatestNonnegativePowerOfTen(v);
    if(1 < z)
        {
        timer_.restart();
        for(unsigned long int j = 0; j < z; j++)
            {
            f_();
            }
        timer_.stop();
        }
    std::ostringstream oss;
    oss
        << std::scientific << std::setprecision(3)
        << "[" << timer_.elapsed_usec() / z << "] "
        << z
        << " iteration" << ((1 == z) ? "" : "s") << " took "
        << timer_.elapsed_msec_str()
        ;
    return oss.str();
}

/// Greatest nonnegative integer power of ten that is less than or
/// equal to the argument, if such a power exists; else zero.
///
/// Motivation: to determine the number of times to repeat an
/// operation in a timing loop.

template<typename F>
unsigned long int AliquotTimer<F>::GreatestNonnegativePowerOfTen(double d)
{
    double const w = std::min(std::log10(d), static_cast<double>(ULONG_MAX));
    unsigned long int const x = static_cast<unsigned long int>(w);
    double const volatile y = std::pow(10.0, static_cast<double>(x));
    return static_cast<unsigned long int>(y);
}

/// Time an operation, using class template AliquotTimer.
///
/// Because it can deduce the function-parameter type, this function
/// template is more convenient to use than the class template in
/// terms of which it is implemented. This is particularly valuable
/// when that type cannot readily be named--or is indeed unspecified,
/// as when boost::bind() is used.
///
/// The maximum time defaults to one second, which is generally long
/// enough to get a stable measurement.

template<typename F>
std::string TimeAnAliquot(F f, double max_seconds = 1.0)
{
    return AliquotTimer<F>(f, max_seconds)();
}

#endif // timer_hpp

