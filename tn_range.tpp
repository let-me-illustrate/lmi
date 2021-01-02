// Trammeled Numeric range type: class template implementation.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "tn_range.hpp"

#include "alert.hpp"
#include "value_cast.hpp"

#include <cmath>                        // pow(), signbit()
#include <exception>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <type_traits>

namespace
{
    /// Clearer diagnostics can be written if it is ascertainable
    /// whether a candidate value lies strictly between its type's
    /// extrema. If a value is constrained to be positive, e.g., and a
    /// negative candidate is to be tested, then it is unhelpful to
    /// tell users that the proper range is something like:
    ///    [0.0, 1.7976931348623157E+308] // IEC 60559 double.
    ///    [0, 32767] // Minimum INT_MAX that C99 E.1 allows.
    /// because they are not likely to recognize those maximum values
    /// as such. Any value of a type for which std::numeric_traits is
    /// not specialized is treated as lying strictly between extrema
    /// for this purpose, because that generally yields an appropriate
    /// outcome, though of course a different behavior can be obtained
    /// by adding a specialization.
    ///
    /// It is significant that floating-point variables can hold
    /// values outside the normalized extrema.

    template
        <typename T
        ,bool=std::numeric_limits<T>::is_specialized
        ,bool=std::is_floating_point_v<T>
        >
    struct strictly_between_extrema_tester
    {};

    // Type is not fundamental (and therefore not floating).

    template<typename T>
    struct strictly_between_extrema_tester<T,false,false>
    {
        bool operator()(T) {return true;}
    };

    // Type is fundamental but not floating, and therefore integral
    // (or void, which would naturally be improper).

    template<typename T>
    struct strictly_between_extrema_tester<T,true,false>
    {
        bool operator()(T t)
            {
            static T const lower_limit = std::numeric_limits<T>::min();
            static T const upper_limit = std::numeric_limits<T>::max();
            return lower_limit < t && t < upper_limit;
            }
    };

    // Type is floating.

    template<typename T>
    struct strictly_between_extrema_tester<T,true,true>
    {
        bool operator()(T t)
            {
            static T const lower_limit = -std::numeric_limits<T>::max();
            static T const upper_limit =  std::numeric_limits<T>::max();
            return lower_limit < t && t < upper_limit;
            }
    };

    template<typename T>
    bool is_strictly_between_extrema(T t)
    {
        return strictly_between_extrema_tester<T>()(t);
    }

    /// Algebraic sign of argument.
    ///
    /// Return value is of same type as argument, as for many members
    /// of std::numeric_limits. Thus, (t * signum(t)) is of type T,
    /// which would not always be the case if an int were returned.

    template<typename T>
    T signum(T t)
    {
        static_assert(std::is_arithmetic_v<T>);
        return (0 == t) ? 0 : std::signbit(t) ? -1 : 1;
    }

    /// Exact-integer determination for floating types.
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
    /// A value of floating type is considered exact iff
    ///  - it is in the range that the floating-point type could
    ///    represent exactly; and
    ///  - it is in the range of long int; and
    ///  - converting it to type long int preserves its value.
    /// Type long long int might have been used instead, but it is not
    /// yet part of standard C++ and not all compilers support it.
    ///
    /// No nonfundamental type is considered exact.
    ///
    /// See this discussion:
    ///   http://groups.google.com/groups?th=1b868327b241fb74
    ///   http://groups.google.com/groups?selm=3DF66B8D.F1C3D2C0%40sun.com

    template<typename T, bool=std::is_floating_point_v<T>>
    struct is_exact_integer_tester
    {};

    template<typename T>
    struct is_exact_integer_tester<T,false>
    {
        bool operator()(T) {return std::numeric_limits<T>::is_exact;}
    };

    template<typename T>
    struct is_exact_integer_tester<T,true>
    {
        static_assert(std::is_floating_point_v<T>);
        bool operator()(T t)
            {
            // SOMEDAY !! nonstd::power() [SGI extension] may be
            // preferable, because std::pow() might not be exact.
            static T z0 = std::pow
                (static_cast<T>(std::numeric_limits<T>::radix)
                ,static_cast<T>(std::numeric_limits<T>::digits)
                );
            long int z1 = std::numeric_limits<long int>::max();
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif // defined __GNUC__
            return
                   -z0 < t
                &&       t < z0
                && -z1 < t
                &&       t < z1
                && t == static_cast<long int>(t)
                ;
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
            }
    };

    template<typename T>
    bool is_exact_integer(T t)
    {
        return is_exact_integer_tester<T>()(t);
    }

    /// Like C99 nextafter(), but prevents range error, and returns
    /// exact integral values unchanged.

    template<typename T>
    T adjust_bound(T t, T direction)
    {
        static_assert(std::is_floating_point_v<T>);
        if(is_exact_integer(t))
            {
            return t;
            }
        // Here, '0 -' avoids a compiler warning about negating an
        // unsigned value.
        if
            (      std::numeric_limits<T>::max()      == t
            || 0 - std::numeric_limits<T>::max()      == t
            ||     std::numeric_limits<T>::infinity() == t
            || 0 - std::numeric_limits<T>::infinity() == t
            )
            {
            return t;
            }
        else if(t < direction)
            {
            return t * (T(1) + signum(t) * std::numeric_limits<T>::epsilon());
            }
        else if(direction < t)
            {
            return t * (T(1) - signum(t) * std::numeric_limits<T>::epsilon());
            }
        else
            {
            return t;
            }
    }

    template<typename T, int>
    struct bound_adjuster
    {};

    template<typename T>
    struct bound_adjuster<T,0>
    {
        T operator()(T t) {return t;}
    };

    template<typename T>
    struct bound_adjuster<T,-1>
    {
        static_assert(std::is_floating_point_v<T>);
        T operator()(T t)
            {
            static T const extremum = -std::numeric_limits<T>::max();
            return adjust_bound(t, extremum);
            }
    };

    template<typename T>
    struct bound_adjuster<T,1>
    {
        static_assert(std::is_floating_point_v<T>);
        T operator()(T t)
            {
            static T const extremum = std::numeric_limits<T>::max();
            return adjust_bound(t, extremum);
            }
    };

    template<typename T>
    T adjust_minimum(T t)
    {
        return bound_adjuster<T,std::is_floating_point_v<T> ? -1 : 0>()(t);
    }

    template<typename T>
    T adjust_maximum(T t)
    {
        return bound_adjuster<T,std::is_floating_point_v<T> ? 1 : 0>()(t);
    }
} // Unnamed namespace.

template<typename T>
void trammel_base<T>::assert_sanity() const
{
    if(!(nominal_minimum() <= nominal_maximum()))
        {
        alarum()
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
        alarum()
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
        alarum()
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
T trammel_base<T>::minimum_minimorum() const
{
    return adjust_minimum(nominal_minimum());
}

template<typename T>
T trammel_base<T>::default_initializer() const
{
    return default_value();
}

template<typename T>
T trammel_base<T>::maximum_maximorum() const
{
    return adjust_maximum(nominal_maximum());
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range()
    :minimum_ {trammel_.minimum_minimorum()}
    ,maximum_ {trammel_.maximum_maximorum()}
    ,value_   {trammel_.default_initializer()}
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(Number n)
    :minimum_ {trammel_.minimum_minimorum()}
    ,maximum_ {trammel_.maximum_maximorum()}
    ,value_   {curb(n)}
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(std::string const& s)
    :minimum_ {trammel_.minimum_minimorum()}
    ,maximum_ {trammel_.maximum_maximorum()}
    ,value_   {curb(value_cast<Number>(s))}
{
    trammel_.assert_sanity();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>& tn_range<Number,Trammel>::operator=(Number n)
{
    value_ = curb(n);
    return *this;
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>& tn_range<Number,Trammel>::operator=(std::string const& s)
{
    value_ = curb(value_cast<Number>(s));
    return *this;
}

/// Change minimum.
///
/// Precondition:
///   minimum_minimorum() <= n <= maximum()
///
/// Postcondition:
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
        alarum()
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
        alarum()
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

/// Change maximum.
///
/// Precondition:
///   minimum() <= n <= maximum_maximorum()
///
/// Postcondition:
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
        alarum()
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
        alarum()
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

/// Set both minimum and maximum, atomically.
///
/// First set both limits to their extrema, then set both to the
/// desired values. Otherwise, when changing between disjoint limit-
/// pairs, limits momentarily cross, and that's detected as an error.
///
/// Example: Suppose it is desired to change the limits from [3, 5]
/// to [0, 1] or [7, 9]. Because the minimum() and maximum() mutators
/// both maintain the invariant
///   minimum() <= maximum()
/// calling them in either order consistently must fail for one of
/// those ranges.

template<typename Number, typename Trammel>
void tn_range<Number,Trammel>::minimum_and_maximum
    (Number n0
    ,Number n1
    )
{
    minimum_ = trammel_.minimum_minimorum();
    maximum_ = trammel_.maximum_maximorum();
    minimum(n0);
    maximum(n1);
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
    return value_cast<Number>(s) == value_;
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator!=(tn_range<Number,Trammel> const& z) const
{
    return !operator==(z);
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator!=(Number n) const
{
    return !operator==(n);
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator!=(std::string const& s) const
{
    return !operator==(s);
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::operator<(tn_range<Number,Trammel> const& z) const
{
    return value_ < z.value_;
}

template<typename Number, typename Trammel>
Trammel const& tn_range<Number,Trammel>::trammel() const
{
    return trammel_;
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::value() const
{
    return value_;
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::curb(Number n) const
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
    std::ostringstream oss;
    bool bounded_below = is_strictly_between_extrema(minimum_);
    bool bounded_above = is_strictly_between_extrema(maximum_);
    if(bounded_below && bounded_above)
        {
        oss
            << "value must be between "
            << value_cast<std::string>(minimum_)
            << " and "
            << value_cast<std::string>(maximum_)
            << " inclusive."
            ;
        }
    else if(bounded_below && !bounded_above)
        {
        oss
            << value_cast<std::string>(minimum_)
            << " is the lower limit."
            ;
        }
    else if(!bounded_below && bounded_above)
        {
        oss
            << value_cast<std::string>(maximum_)
            << " is the upper limit."
            ;
        }
    else
        {
        alarum() << "Unanticipated case." << LMI_FLUSH;
        }
    return oss.str();
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::is_valid(Number n) const
{
    return minimum_ <= n && n <= maximum_;
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
    Number n;
    try
        {
        n = value_cast<Number>(s);
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
    else if(!is_strictly_between_extrema(n))
        {
        std::ostringstream oss;
        oss
            << s
            << " is not normalized."
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
        alarum() << "Unanticipated case." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

template<typename Number, typename Trammel>
void tn_range<Number,Trammel>::enforce_circumscription()
{
    value_ = curb(value_);
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::equal_to(std::string const& s) const
{
    return operator==(s);
}

template<typename Number, typename Trammel>
std::string tn_range<Number,Trammel>::str() const
{
    return value_cast<std::string>(value_);
}

template<typename Number, typename Trammel>
double tn_range<Number,Trammel>::universal_minimum() const
{
    return value_cast<double>(minimum_);
}

template<typename Number, typename Trammel>
double tn_range<Number,Trammel>::universal_maximum() const
{
    return value_cast<double>(maximum_);
}

template<typename Number, typename Trammel>
std::type_info const& tn_range<Number,Trammel>::value_type() const
{
    return typeid(Number);
}
