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

// $Id: timer.hpp,v 1.23 2007-01-21 16:24:14 chicares Exp $

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

#include <climits> // LONG_MAX
#include <cmath>
#include <iomanip>
#include <ios>
#include <ostream>
#include <sstream>
#include <stdexcept>
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
    friend class TimerTest;
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
/// Implicitly-declared special member functions do the right thing.

template<typename F>
class AliquotTimer
{
    friend class TimerTest;

  public:
    AliquotTimer(F f, double max_seconds);
    AliquotTimer& operator()();

    std::string const& str() const;
    double unit_time() const;

  private:
    static long int GreatestNonnegativePowerOfTen(double);

    F           f_;
    double      max_seconds_;
    double      initial_trial_time_;
    double      unit_time_;
    std::string str_;
};

template<typename F>
AliquotTimer<F>::AliquotTimer(F f, double max_seconds)
    :f_          (f)
    ,max_seconds_(max_seconds)
{
    Timer timer;
    if(max_seconds_ * timer.frequency_ < 1.0)
        {
        std::ostringstream oss;
        oss
            << "Timer interval "
            << max_seconds_
            << " is too short: it is less than the reciprocal of "
            << timer.frequency_
            << ", the timer frequency."
            ;
        throw std::invalid_argument(oss.str());
        }

    f_();
    timer.stop();
    initial_trial_time_ = timer.elapsed_usec();
    unit_time_ = initial_trial_time_;
    std::ostringstream oss;
    oss
        << std::scientific << std::setprecision(3)
        << "[" << unit_time_ << "]"
        << " initial calibration took "
        << timer.elapsed_msec_str()
        ;
    str_ = oss.str();
}

template<typename F>
AliquotTimer<F>& AliquotTimer<F>::operator()()
{
    Timer timer;
    double const v =
        (0.0 != initial_trial_time_)
        ? max_seconds_ / initial_trial_time_
        : max_seconds_ * timer.frequency_
        ;
    long int const z = GreatestNonnegativePowerOfTen(v);
    if(1 < z)
        {
        for(long int j = 0; j < z; ++j)
            {
            f_();
            }
        timer.stop();
        unit_time_ = timer.elapsed_usec() / z;
        std::ostringstream oss;
        oss
            << std::scientific << std::setprecision(3)
            << "[" << unit_time_ << "]"
            << " " << z
            << " iterations took "
            << timer.elapsed_msec_str()
            ;
        str_ = oss.str();
        }
    return *this;
}

template<typename F>
std::string const& AliquotTimer<F>::str() const
{
    return str_;
}

template<typename F>
double AliquotTimer<F>::unit_time() const
{
    return unit_time_;
}

/// Greatest nonnegative-integer power of ten that is less than or
/// equal to the argument, if such a power exists--but never greater
/// than LONG_MAX; else zero.
///
/// Motivation: to determine the number of times to repeat an
/// operation in a timing loop.
///
/// An intermediate value is volatile-qualified in order to work
/// around a defect observed with MinGW gcc: the defective ms C
/// runtime library MinGW uses doesn't reliably return integer
/// results for std::pow() with exact-integer arguments. To see the
/// problem, remove the 'volatile' qualifier and run the unit test.
///
/// The last line is not equivalent to
///    return std::min(LONG_MAX, static_cast<long int>(z));
/// , which would compare integral rather than double values; for
/// proof, run the unit test with that line substituted (and the
/// appropriate header included).

template<typename F>
long int AliquotTimer<F>::GreatestNonnegativePowerOfTen(double d)
{
    if(d <= 0.0)
        {
        return 0L;
        }
    double const volatile z = std::pow(10.0, std::floor(std::log10(d)));
    return LONG_MAX < z ? LONG_MAX : static_cast<long int>(z);
}

template<typename F>
std::ostream& operator<<(std::ostream& os, AliquotTimer<F> const& z)
{
    return os << z.str();
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
AliquotTimer<F> TimeAnAliquot(F f, double max_seconds = 1.0)
{
    return AliquotTimer<F>(f, max_seconds)();
}

#endif // timer_hpp

