// Represent a currency amount exactly as integral cents.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef currency_hpp
#define currency_hpp

#include "config.hpp"

#include <cmath>                        // rint()
#include <ostream>
#include <stdexcept>                    // runtime_error
#include <vector>

// Macros USE_CURRENCY_CLASS and CURRENCY_UNIT_IS_CENTS are used
// elsewhere. Eventually they'll both be eliminated, along with
// all code along paths where they aren't both defined.

#define USE_CURRENCY_CLASS

#if !defined USE_CURRENCY_CLASS
#   undef CURRENCY_UNIT_IS_CENTS // Requires currency class.

using currency = double;

inline currency from_cents(double z) {return z / 100.0;}

inline double dblize(currency z) {return z;}

inline std::vector<double> dblize(std::vector<currency> const& z)
{
    return z;
}

#else // defined USE_CURRENCY_CLASS

#   define CURRENCY_UNIT_IS_CENTS

class raw_cents {}; // Tag class.

class currency
{
    friend class currency_test;
    friend currency from_cents(double);       // explicit ctor
    template<typename> friend class round_to; // explicit ctor
    friend class round_to_test;               // currency::cents_digits

#   if defined CURRENCY_UNIT_IS_CENTS
    static constexpr int    cents_digits     = 2;
    static constexpr double cents_per_dollar = 100.0;
#   else  // !defined CURRENCY_UNIT_IS_CENTS
    static constexpr int    cents_digits     = 0;
    static constexpr double cents_per_dollar = 1.0;
#   endif // !defined CURRENCY_UNIT_IS_CENTS

  public:
    using data_type = double;

    currency() = default;
    currency(currency const&) = default;
    currency& operator=(currency const&) = default;
    ~currency() = default;

    currency& operator+=(currency z) {m_ += z.m_; return *this;}
    currency& operator-=(currency z) {m_ -= z.m_; return *this;}

    currency& operator*=(int      z) {m_ *= z   ; return *this;}

    currency operator-() const {return currency(-cents(), raw_cents {});}

    data_type cents() const {return m_;}
    // CURRENCY !! add a unit test for possible underflow
    // CURRENCY !! is multiplication by reciprocal faster or more accurate?
    double d() const {return m_ / cents_per_dollar;}

  private:
    explicit currency(data_type z, raw_cents) : m_ {z} {}

    data_type m_ = {};
};

inline bool operator==(currency lhs, currency rhs)
    {return lhs.cents() == rhs.cents();}
inline bool operator< (currency lhs, currency rhs)
    {return lhs.cents() <  rhs.cents();}
inline bool operator!=(currency lhs, currency rhs)
    {return !operator==(lhs, rhs);}
inline bool operator> (currency lhs, currency rhs)
    {return  operator< (rhs, lhs);}
inline bool operator<=(currency lhs, currency rhs)
    {return !operator> (lhs, rhs);}
inline bool operator>=(currency lhs, currency rhs)
    {return !operator< (lhs, rhs);}

inline currency operator+(currency lhs, currency rhs)
    {return currency {lhs} += rhs;}
inline currency operator-(currency lhs, currency rhs)
    {return currency {lhs} -= rhs;}

inline currency operator*(currency lhs, int rhs)
    {return currency {lhs} *= rhs;}
inline currency operator*(int lhs, currency rhs)
    {return currency {rhs} *= lhs;}

inline double operator*(currency lhs, double rhs)
    {return lhs.d() * rhs;}
inline double operator*(double lhs, currency rhs)
    {return lhs * rhs.d();}
inline double operator/(currency lhs, double rhs)
    {return lhs.d() / rhs;}

inline std::ostream& operator<<(std::ostream& os, currency z)
    {return os << z.d();}

inline currency from_cents(double z)
    {
#   if defined CURRENCY_UNIT_IS_CENTS
    if(z != std::rint(z)) throw std::runtime_error("Nonintegral cents.");
    return currency(z, raw_cents{});
#   else  // !defined CURRENCY_UNIT_IS_CENTS
    // If currency unit is dollars rather than cents, then:
    //  - dividing by 100 is the only reasonable thing to do here, even
    //    though 'cents_per_dollar' is unity; and
    //  - a value such as $.01 cannot be integral, so the desired
    //    invariant that the result is integral must be sacrificed.
    return currency(z / 100.0, raw_cents{});
#   endif // !defined CURRENCY_UNIT_IS_CENTS
    }

inline double dblize(currency z) {return z.d();}

inline std::vector<double> dblize(std::vector<currency> const& z)
{
    std::vector<double> r;
    r.reserve(z.size());
    for(auto const& i : z)
        {
        r.emplace_back(i.d());
        }
    return r;
}

#   endif // defined USE_CURRENCY_CLASS

/// Zero cents--akin to a user-defined literal.
///
/// UDLs seem less convenient because the obvious "0_c" is likely to
/// collide with some other UDL, and "currency::0_c" is too verbose.
/// "0_cents" may avoid both those problems, but "C0" is terser.
/// "C0" is chosen instead of "c0" only for the pixilated reason that
/// the capital letter looks kind of like a "0".

inline constexpr currency C0 = {};

#endif // currency_hpp
