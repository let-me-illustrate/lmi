// Trammeled Numeric range type: template class implementation.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: tn_range.tpp,v 1.2 2005-02-19 03:27:45 chicares Exp $

#include "config.hpp"

#include "tn_range.hpp"

#include "numeric_io_cast.hpp"

#include <cmath> // std::pow()
#include <istream>
#include <ostream>
#include <sstream>

namespace
{
    // Signum, defined here to return 0 for NaNs.
    //
    // To handle unsigned types without warnings, the value zero is
    // stored in a volatile variable, and the value negative one is
    // cast to the argument type.
    //
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

    // TODO ?? These exact-integer template functions are experimental.
    //
    // Motivation: ascertaining whether a floating-point value lies
    // within a range like [-1.07, +1.07] requires careful handling of
    // the endpoints, but exact range limits don't.
    //
    // Without builtin language support, this is difficult. A floating
    // literal that is inexact may be translated to a floating value
    // that is. Perhaps requiring that the value be integral is wise,
    // or perhaps not: for an IEEE754 64-bit double, integral values
    // greater than 2^53 don't differ from their upper neighbors by
    // unity, whereas 0.5 is exact and C++98 4.8/1 requires it to be
    // treated as such.
    //
    // If C99's remainder() can be used, it's guaranteed to give an
    // exact result; but in 2004-12, that's not part of C++.
    //
    // Maybe this refinement isn't worth the trouble.

    // Determine whether a floating-point value represents an integer
    // exactly. This assumes is_iec559, without asserting it, because
    // some compilers would fail such an assertion even though the
    // underlying hardware conforms to that standard.
    //
    // This template function actually returns true iff
    //  - t is in the range that the floating-point type could
    //    represent exactly; and
    //  - t is in the range of long int; and
    //  - t equals static_cast<long int>(t)
    // Instead, type long long int might have been used, but it is
    // not yet part of standard C++ and not all compilers support it.
    //
    // See this discussion:
    //   http://groups.google.com/groups?th=1b868327b241fb74
    //   http://groups.google.com/groups?selm=3DF66B8D.F1C3D2C0%40sun.com
    //
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

    // Like C99 nextafter(), but prevents range error, and returns
    // exact integral values unchanged.
    //
    // Using the straightforward
    //    if(boost::is_float<T>::value)
    // would cause compiler warnings that later statements are
    // unreachable when the condition is true. Instead, the condition
    // is stored in a volatile object. That object really ought to be
    // const volatile, but, as discussed here:
    //   http://www.google.com/groups?selm=4192bc30%241%40newsgroups.borland.com
    // that would elicit a spurious warning from the borland compiler.
    //
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
            (       std::numeric_limits<T>::max() == t
            ||  0 - std::numeric_limits<T>::max() == t
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
} // Unnamed namespace.

template<typename T>
void trammel_base<T>::check_sanity()
{
    if(!(nominal_minimum() <= nominal_maximum()))
        {
        std::ostringstream oss;
        oss
            << "Lower bound '"
            << nominal_minimum()
            << "' exceeds upper bound '"
            << nominal_maximum()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    else if(!(nominal_minimum() <= default_value()))
        {
        std::ostringstream oss;
        oss
            << "Lower bound '"
            << nominal_minimum()
            << "' exceeds default value '"
            << default_value()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    else if(!(default_value() <= nominal_maximum()))
        {
        std::ostringstream oss;
        oss
            << "Default value '"
            << default_value()
            << "' exceeds upper bound '"
            << nominal_maximum()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    else
        {
        return;
        }
}

template<typename T>
T trammel_base<T>::maximum()
{
    return adjust_bound
        (nominal_maximum()
        ,std::numeric_limits<T>::max()
        );
}

// The second argument of adjust_bound() must be cast to T if it
// is negative. Otherwise, an integral promotion [5.3.1/7] might
// be performed, and that would prevent template resolution. And
// '0 -' avoids a compiler warning about negating an unsigned value.
//
template<typename T>
T trammel_base<T>::minimum()
{
    return adjust_bound
        (nominal_minimum()
        ,std::numeric_limits<T>::is_signed
            ? static_cast<T>(0 - std::numeric_limits<T>::max())
            : static_cast<T>(0)
        );
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range()
{
    Trammel x;
    x.check_sanity();
    max_   = x.maximum();
    min_   = x.minimum();
    value_ = x.default_value();
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(Number n)
{
    Trammel x;
    x.check_sanity();
    max_   = x.maximum();
    min_   = x.minimum();
    value_ = trammel(n);
}

template<typename Number, typename Trammel>
tn_range<Number,Trammel>::tn_range(std::string const& s)
{
    Trammel x;
    x.check_sanity();
    max_   = x.maximum();
    min_   = x.minimum();
    value_ = trammel(numeric_io_cast<Number>(s));
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
bool tn_range<Number,Trammel>::is_valid(Number n) const
{
    return min_ <= n && n <= max_;
}

template<typename Number, typename Trammel>
bool tn_range<Number,Trammel>::is_valid(std::string const& s) const
{
    try
        {
        Number n = numeric_io_cast<Number>(s);
        return min_ <= n && n <= max_;
        }
    catch(std::exception const& e)
        {
        // TODO ?? Also provide a string indicating the problem.
        return false;
        }
}

template<typename Number, typename Trammel>
std::pair<Number,Number> tn_range<Number,Trammel>::limits() const
{
    return std::make_pair(min_, max_);
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::trammel(Number n) const
{
    if(max_ <= n)
        {
        return max_;
        }
    else if(n <= min_)
        {
        return min_;
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
std::string tn_range<Number,Trammel>::str() const
{
    return numeric_io_cast<std::string>(value_);
}

template<typename Number, typename Trammel>
Number tn_range<Number,Trammel>::value() const
{
    return value_;
}

