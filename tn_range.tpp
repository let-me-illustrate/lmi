// Trammeled Numeric range type: class template implementation.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: tn_range.tpp,v 1.7 2006-07-08 00:52:18 chicares Exp $

#include "tn_range.hpp"

#include "alert.hpp"
#include "numeric_io_cast.hpp"

#include <cmath> // std::pow()
#include <exception>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace
{
    /// Signum, defined here to return 0 for NaNs.
    ///
    /// To handle unsigned types without warnings, the value zero is
    /// stored in a volatile variable, and the value negative one is
    /// cast to the argument type.

    template<typename T>
    T signum(T t)
    {
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
        T volatile zero = 0;
        if(t < zero)
            {
            return static_cast<T>(-1);
            }
        else if(0 < t)
            {
            return  1;
            }
        else
            {
            return  0;
            }
    }

    /// Exact-integer function templates.
    ///
    /// Motivation: Ascertaining whether a floating-point value lies
    /// within a range like [-1.07, +1.07] requires careful handling
    /// of the endpoints, but exact range limits don't.
    ///
    /// Certainly (double)(1.07) is inexact on a binary machine. It
    /// is evaluated [C++98 2.13.3/1] as one of the two exactly-
    /// representable doubles closest to the real number 1.07--either
    /// the next higher (call it 1.07+) or the next lower (1.07-).
    /// The choice between these values is not guaranteed to be more
    /// predictable than completely random. Now, if an input value of
    /// 1.07+ is compared to a range with upper limit 1.07-, the user
    /// is confronted with a puzzling diagnostic claiming that 1.07
    /// is too high because the upper limit is 1.07 . Exactly such a
    /// problem has been observed in practice when the present
    /// precautions are not taken.
    ///
    /// To prevent that serious usability problem, every inexact range
    /// limit is mapped to a value that is certain to lie outside the
    /// real-number range by less than two times epsilon.
    ///
    /// This is difficult without builtin language support. A floating
    /// literal that is inexact may be translated to a floating value
    /// that is. Perhaps requiring that the value be integral is wise,
    /// or perhaps not: for an IEEE754 64-bit double, integral values
    /// greater than 2^53 don't differ from their upper neighbors by
    /// unity, whereas 0.5 is exact and C++98 4.8/1 requires it to be
    /// treated as such.
    ///
    /// If C99's remainder() can be used, it's guaranteed to give an
    /// exact result; but in 2004-12, that's not part of C++.
    ///
    /// Implementation.
    ///
    /// Determine whether a floating-point value represents an integer
    /// exactly. This assumes is_iec559, without asserting it, because
    /// some compilers would fail such an assertion even though the
    /// underlying hardware conforms to that standard.
    ///
    /// This function template actually returns true iff
    ///  - t is in the range that the floating-point type could
    ///    represent exactly; and
    ///  - t is in the range of long int; and
    ///  - t equals static_cast<long int>(t)
    /// Instead, type long long int might have been used, but it is
    /// not yet part of standard C++ and not all compilers support it.
    ///
    /// See this discussion:
    ///   http://groups.google.com/groups?th=1b868327b241fb74
    ///   http://groups.google.com/groups?selm=3DF66B8D.F1C3D2C0%40sun.com

    template<typename T>
    bool floating_point_value_is_exact_integer(T t)
    {
        BOOST_STATIC_ASSERT(boost::is_float<T>::value);
        static T z0 = std::pow
            (static_cast<T>(std::numeric_limits<T>::radix)
            ,static_cast<T>(std::numeric_limits<T>::digits)
            );
        long int z1 = std::numeric_limits<long int>::max();
        return
                -z0 < t
            &&        t < z0
            &&  -z1 < t
            &&        t < z1
            && t == static_cast<long int>(t);
    }

    template<typename T>
    bool is_exact_integer(T)
    {
        return std::numeric_limits<T>::is_exact;
    }

    template<> bool is_exact_integer(float t)
    {
        return floating_point_value_is_exact_integer(t);
    }

    template<> bool is_exact_integer(double t)
    {
        return floating_point_value_is_exact_integer(t);
    }

    template<> bool is_exact_integer(long double t)
    {
        return floating_point_value_is_exact_integer(t);
    }

    /// Like C99 nextafter(), but prevents range error, and returns
    /// exact integral values unchanged.
    ///
    /// Using the straightforward
    ///    if(boost::is_float<T>::value)
    /// would cause compiler warnings that later statements are
    /// unreachable when the condition is true. Instead, the condition
    /// is stored in a volatile object. That object really ought to be
    /// const volatile, but, as discussed here:
    ///   http://www.google.com/groups?selm=4192bc30%241%40newsgroups.borland.com
    /// that would elicit a spurious warning from the borland compiler.

    template<typename T>
    T adjust_bound(T t, T direction)
    {
        bool volatile is_float = boost::is_float<T>::value;
        if(!is_float)
            {
            return t;
            }
        if(is_exact_integer(t))
            {
            return t;
            }
        // Here, '0 -' avoids a compiler warning about negating an
        // unsigned value.
        if
            (       std::numeric_limits<T>::max()      == t
            ||  0 - std::numeric_limits<T>::max()      == t
            ||      std::numeric_limits<T>::infinity() == t
            ||  0 - std::numeric_limits<T>::infinity() == t
            )
            {
            return t;
            }

        // The return values computed here are cast to T because, for
        // instance, if T is signed char, then an integral promotion
        // would be performed--even though these calculations are
        // actually reachable only for floating-point types. This
        // workaround is more compact than specializing the template.
        else if(t < direction)
            {
            return static_cast<T>
                (t * (1 + signum(t) * std::numeric_limits<T>::epsilon())
                );
            }
        else if(direction < t)
            {
            return static_cast<T>
                (t * (1 - signum(t) * std::numeric_limits<T>::epsilon())
                );
            }
        else
            {
            return t;
            }
    }

    /// The second argument of adjust_bound() must be cast to T if it
    /// is negative. Otherwise, an integral promotion [5.3.1/7] might
    /// be performed, and that would prevent template resolution. And
    /// '0 -' avoids a compiler warning about negating an unsigned
    /// value.

    template<typename T>
    T adjust_minimum(T t)
    {
        static T const extremum = std::numeric_limits<T>::is_signed
            ? static_cast<T>(0 - std::numeric_limits<T>::max())
            : static_cast<T>(0)
            ;
        return adjust_bound(t, extremum);
    }

    template<typename T>
    T adjust_maximum(T t)
    {
        static T const extremum = std::numeric_limits<T>::max();
        return adjust_bound(t, extremum);
    }
} // Unnamed namespace.

template<typename T>
void trammel_base<T>::assert_sanity() const
{
    if(!(nominal_minimum() <= nominal_maximum()))
        {
        fatal_error()
            << "Lower bound "
            << nominal_minimum()
            << " exceeds upper bound "
            << nominal_maximum()
            << " ."
            << LMI_FLUSH
            ;
        }
    if(!(nominal_minimum() <= default_value()))
        {
        fatal_error()
            << "Lower bound "
            << nominal_minimum()
            << " exceeds default value "
            << default_value()
            << " ."
            << LMI_FLUSH
            ;
        }
    if(!(default_value() <= nominal_maximum()))
        {
        fatal_error()
            << "Default value "
            << default_value()
            << " exceeds upper bound "
            << nominal_maximum()
            << " ."
            << LMI_FLUSH
            ;
        }
}

template<typename T>
T trammel_base<T>::default_initializer() const
{
    return default_value();
}

template<typename T>
T trammel_base<T>::minimum_minimorum() const
{
    return adjust_minimum(nominal_minimum());
}

template<typename T>
T trammel_base<T>::maximum_maximorum() const
{
    return adjust_maximum(nominal_maximum());
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range()
    :minimum_ (trammel_.minimum_minimorum())
    ,maximum_ (trammel_.maximum_maximorum())
    ,value_   (trammel_.default_initializer())
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(Number n)
    :minimum_ (trammel_.minimum_minimorum())
    ,maximum_ (trammel_.maximum_maximorum())
    ,value_   (trammel(n))
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(std::string const& s)
    :minimum_ (trammel_.minimum_minimorum())
    ,maximum_ (trammel_.maximum_maximorum())
    ,value_   (trammel(numeric_io_cast<Number>(s)))
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::~tn_range()
{
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>& tn_range<Number,Trammel>::operator=(Number n)
{
    value_ = trammel(n);
    return *this;
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>& tn_range<Number,Trammel>::operator=(std::string const& s)
{
    value_ = trammel(numeric_io_cast<Number>(s));
    return *this;
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator==(tn_range<Number,Trammel> const& z) const
{
    return z.value_ == value_;
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator==(Number n) const
{
    return n == value_;
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator==(std::string const& s) const
{
    return numeric_io_cast<Number>(s) == value_;
}

template<typename Number, typename Trammel>
void tn_range<Number,Trammel>::enforce_limits()
{
    value_ = trammel(value_);
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::equal_to(std::string const& s) const
{
    return operator==(s);
}

/// Change minimum. Postcondition:
///   minimum_minimorum() <= minimum() <= maximum()
/// but value() is not necessarily limited by the new minimum.

template<typename Number, typename Trammel>
void tn_range<Number,Trammel>::minimum(Number n)
{
    if(minimum() == n)
        {
        return;
        }

    Number candidate(adjust_minimum(n));
    if(!(trammel_.minimum_minimorum() <= candidate))
        {
        fatal_error()
            << "Cannot change lower bound to "
            << candidate
            << ", which is less than infimum "
            << trammel_.minimum_minimorum()
            << " ."
            << LMI_FLUSH
            ;
        }
    if(!(candidate <= maximum()))
        {
        fatal_error()
            << "Cannot change lower bound to "
            << candidate
            << ", which is greater than upper bound "
            << maximum()
            << " ."
            << LMI_FLUSH
            ;
        }

    minimum_ = candidate;
}

/// Change maximum. Postcondition:
///   minimum() <= maximum() <= maximum_maximorum()
/// but value() is not necessarily limited by the new maximum.

template<typename Number, typename Trammel>
void tn_range<Number,Trammel>::maximum(Number n)
{
    if(maximum() == n)
        {
        return;
        }

    Number candidate(adjust_maximum(n));
    if(!(minimum() <= candidate))
        {
        fatal_error()
            << "Cannot change upper bound to "
            << candidate
            << ", which is less than lower bound "
            << minimum()
            << " ."
            << LMI_FLUSH
            ;
        }
    if(!(candidate <= trammel_.maximum_maximorum()))
        {
        fatal_error()
            << "Cannot change upper bound to "
            << candidate
            << ", which is greater than supremum "
            << trammel_.maximum_maximorum()
            << " ."
            << LMI_FLUSH
            ;
        }

    maximum_ = candidate;
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::minimum() const
{
    return minimum_;
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::maximum() const
{
    return maximum_;
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::value() const
{
    return value_;
}

/// Show limits in a way suitable only for a diagnostic.
///
/// The trivial implementation that displays both upper and lower
/// limits is not always suitable. Sometimes there's effectively no
/// limit other than that imposed by the floating-point number system.
/// In that case, it is confusing to tell end users to enter a number
/// between, say, 0.0 and 1.79769e+308; what they really need to know
/// is that the value must not be less than zero.
///
/// This implementation assumes that an error has been detected, so
/// it signals an error if the value is within bounds. It is therefore
/// not suitable for displaying limits when the value is acceptable.

template<typename Number, typename Trammel>
std::string tn_range<Number,Trammel>::format_limits_for_error_message() const
{
    static Number const extremum = std::numeric_limits<Number>::max();
    std::ostringstream oss;
    bool bounded_above = maximum_ < extremum;
    bool bounded_below = -extremum < minimum_;
    if(bounded_above && bounded_below)
        {
        oss
            << "value must be between "
            << numeric_io_cast<std::string>(minimum_)
            << " and "
            << numeric_io_cast<std::string>(maximum_)
            << " inclusive."
            ;
        }
    else if(!bounded_above && bounded_below)
        {
        oss
            << numeric_io_cast<std::string>(minimum_)
            << " is the lower limit."
            ;
        }
    else if(bounded_above && !bounded_below)
        {
        oss
            << numeric_io_cast<std::string>(maximum_)
            << " is the upper limit."
            ;
        }
    else
        {
        fatal_error() << "Unanticipated case." << LMI_FLUSH;
        }
    return oss.str();
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::is_valid(Number n) const
{
    return minimum_ <= n && n <= maximum_;
}

// TODO ?? EGREGIOUS_DEFECT
template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::is_valid(std::string const& s) const
{
    try
        {
        Number n = numeric_io_cast<Number>(s);
        return minimum_ <= n && n <= maximum_;
        }
    catch(std::exception const& e)
        {
        // TODO ?? Also provide a string indicating the problem.
        return false;
        }
}

template<typename Number, typename Trammel>
std::string tn_range<Number,Trammel>::str() const
{
    return numeric_io_cast<std::string>(value_);
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::trammel(Number n) const
{
    if(n <= minimum_)
        {
        return minimum_;
        }
    else if(maximum_ <= n)
        {
        return maximum_;
        }
    else
        {
        return n;
        }
}

template<typename Number, typename Trammel>
std::istream& tn_range<Number,Trammel>::read(std::istream& is)
{
    std::string s;
    is >> s;
    operator=(s);
    return is;
}

template<typename Number, typename Trammel>
std::ostream& tn_range<Number,Trammel>::write(std::ostream& os) const
{
    return os << str();
}

template<typename Number, typename Trammel>
std::string tn_range<Number,Trammel>::diagnose_invalidity
    (std::string const& s
    ) const
{
    static Number const extremum = std::numeric_limits<Number>::max();
    Number n;
    try
        {
        n = numeric_io_cast<Number>(s);
        }
    catch(std::exception const&)
        {
        std::ostringstream oss;
        oss << "'" << s << "' is ill formed.";
        return oss.str();
        }

    if(is_valid(n))
        {
        return "";
        }
    else if(n < -extremum || extremum < n)
        {
        // C99 7.20.1/1 .
        std::ostringstream oss;
        oss
            << s
            << " is not representable."
            ;
        return oss.str();
        }
    else if(n < minimum_)
        {
        std::ostringstream oss;
        oss
            << s
            << " is too low: "
            << format_limits_for_error_message()
            ;
        return oss.str();
        }
    else if(maximum_ < n)
        {
        std::ostringstream oss;
        oss
            << s
            << " is too high: "
            << format_limits_for_error_message()
            ;
        return oss.str();
        }
    else
        {
        fatal_error() << "Unanticipated case." << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

