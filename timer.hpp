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

// $Id: timer.hpp,v 1.28 2007-01-26 02:09:36 chicares Exp $

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

/// Time an operation over an actively-adjusted number of repetitions.
///
/// Adjust the number of repetitions measured, balancing expenditure
/// of time against accuracy. Motivation: it is often useful to time
/// an operation in unit tests, but unit tests should run quickly even
/// on slow machines or in high-overhead debugging modes.
///
/// Start by executing the operation once. If that takes longer than
/// the limit specified, then record that single observation and exit.
/// Rationale: if it is desired to spend up to one second testing an
/// operation, but the operation takes ten seconds, then the desired
/// limit has already been exceeded tenfold, and the value of one more
/// observation does not justify doubling that harm.
///
/// Otherwise, discard the first observation (which is often anomalous
/// due, e.g., to cache effects); execute the operation repeatedly,
/// until either all allotted time has been used or the repetition
/// count reaches one hundred; and record the mean measured time.
///
/// The rationale for stopping after one hundred repetitions is that
/// continuing is unlikely to increase accuracy. Consider timing an
/// operation that takes one nanosecond, with a (default) limit of
/// one second: one billion repetions would seem excessive, and
/// hardcoding a lower time limit creates exactly the sort of problems
/// that this class's adaptive behavior is intended to avoid.
///
/// Template parameter 'F' is the type of the first ctor parameter,
/// which either is a nullary function or behaves like one. A facility
/// such as boost::bind() is useful for reducing the arity of the
/// argument (see unit test). Naturally, this is subject to the
/// Forwarding Problem, but that's inherent in the language.
///
/// Ctor parameter 'max_seconds' is the desired limit on measurement
/// time, in seconds.
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
        << unit_time_
        << " s: first trial took longer than "
        << max_seconds_
        << " s desired limit"
        ;
    str_ = oss.str();
}

/// Hardcoded widths allow for one hundred iterations and one second
/// per iteration.

template<typename F>
AliquotTimer<F>& AliquotTimer<F>::operator()()
{
    if(max_seconds_ < initial_trial_time_)
        {
        return *this;
        }

    Timer timer;
    double const limit = max_seconds_ * timer.frequency_;
    int j = 0;
    do
        {
        f_();
        ++j;
        }
    while(j < 100 && timer.inspect() < timer.time_when_started_ + limit);
    timer.stop();
    unit_time_ = timer.elapsed_usec() / j;
    std::ostringstream oss;
    oss
        << std::scientific << std::setprecision(3)
        << unit_time_
        << " s = "
        << std::fixed      << std::setprecision(0)
        << std::setw(10) << 1.0e9 * unit_time_
        << " ns, mean of "
        << std::setw( 3) << j
        << " iterations"
        ;
    str_ = oss.str();

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

