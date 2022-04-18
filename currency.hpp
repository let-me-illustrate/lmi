// Represent a currency amount exactly as integral cents.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
#include <limits>
#include <ostream>
#include <stdexcept>                    // runtime_error
#include <vector>

class raw_cents {}; // Tag class.

class currency
{
    friend class currency_test;
    friend currency from_cents(double);       // private ctor
    template<typename> friend class round_to; // private ctor
    friend class round_to_test;               // currency::cents_digits
    friend consteval currency operator""_cents(unsigned long long int);

    static constexpr int    cents_digits     = 2;
    static constexpr double cents_per_dollar = 100.0;

  public:
    using data_type = double;

    currency() = default;
    currency(currency const&) = default;
    currency& operator=(currency const&) = default;
    ~currency() = default;

    currency& operator+=(currency z) {m_ += z.m_; return *this;}
    currency& operator-=(currency z) {m_ -= z.m_; return *this;}

    currency& operator*=(int      z) {m_ *= z   ; return *this;}

    constexpr currency operator-() const {return currency(-m_, raw_cents {});}

    constexpr data_type cents() const {return m_;}
    // CURRENCY !! add a unit test for possible underflow
    // CURRENCY !! is multiplication by reciprocal faster or more accurate?
    double d() const {return m_ / cents_per_dollar;}

  private:
    constexpr explicit currency(data_type z, raw_cents) : m_ {z} {}

    data_type m_ = {};
};

consteval currency operator""_cents(unsigned long long int cents)
{
    constexpr auto mant_dig = std::numeric_limits<currency::data_type>::digits;
    constexpr unsigned long long int limit = 1ULL << mant_dig;
    return
          cents <= limit
        ? currency(static_cast<currency::data_type>(cents), raw_cents{})
        : throw std::domain_error("outside currency domain")
        ;
}

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
inline double operator/(currency lhs, currency rhs)
    {return lhs.cents() / rhs.cents();}

inline std::ostream& operator<<(std::ostream& os, currency z)
    {return os << z.d();}

/// Convert from an integer-valued double to currency.
///
/// This function is intended to be called very seldom (and then
/// almost always with a manifest-constant argument), so the cost
/// of the runtime value-preservation test doesn't matter.

inline currency from_cents(double cents)
    {
    if(cents != std::rint(cents))
        {
        throw std::runtime_error("Nonintegral cents.");
        }
    return currency(cents, raw_cents{});
    }

inline double centize(currency z) {return z.cents();}

inline std::vector<double> centize(std::vector<currency> const& z)
{
    std::vector<double> r;
    r.reserve(z.size());
    for(auto const& i : z)
        {
        r.emplace_back(i.cents());
        }
    return r;
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

/// Zero cents as a terse compile-time constant.
///
/// This particular value occurs so often that it merits a name
/// no wider than '0.0' (unlike the equivalent '0_cents').

inline constexpr currency C0 = {};

#endif // currency_hpp
