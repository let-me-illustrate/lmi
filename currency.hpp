// Represent a currency amount exactly as integral cents.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "bourn_cast.hpp"
#include "round_to.hpp"

#include <cstdint>                      // int64_t
#include <iostream>                     // ostream
#include <vector>

#define USE_CURRENCY_CLASS

#if !defined USE_CURRENCY_CLASS
using currency = double;

#if defined __GNUC__
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#endif // defined __GNUC__
#endif // !defined USE_CURRENCY_CLASS

#if defined USE_CURRENCY_CLASS
class currency
{
//#if defined __GNUC__
//#   pragma GCC diagnostic ignored "-Wuseless-cast"
//#endif // defined __GNUC__
//  using data_type = double;
//  using data_type = long double;
    using data_type = std::int64_t;

    friend std::ostream& operator<<(std::ostream&, currency const&);
    friend class currency_test;

  public:
    currency() = default;
    currency(currency const&) = default;
    ~currency() = default;

    explicit currency(double d)  {m_ = from_double(d);}

    currency& operator=(currency const&) = default;
    currency& operator=(double d) {m_ = from_double(d); return *this;}

    operator double() const {return to_double();}

    // Is this better, with 'const&'?
//  bool operator==(currency const& z) const {return z.m_ == m_;}

    bool operator< (currency z) const {return m_ <  z.m_;}
    bool operator<=(currency z) const {return m_ <= z.m_;}
    bool operator==(currency z) const {return m_ == z.m_;}
    bool operator!=(currency z) const {return m_ != z.m_;}
    bool operator> (currency z) const {return m_ >  z.m_;}
    bool operator>=(currency z) const {return m_ >= z.m_;}

    bool operator< (double d) const {return to_double() <  d;}
    bool operator<=(double d) const {return to_double() <= d;}
    bool operator==(double d) const {return to_double() == d;}
    bool operator!=(double d) const {return to_double() != d;}
    bool operator> (double d) const {return to_double() >  d;}
    bool operator>=(double d) const {return to_double() >= d;}

    // Is this the ideal signature for this operator?
//  currency operator-() const {return currency(-m_);}
//  currency& operator-() {m_ = -m_; return *this;}
// Dangerous--demonstrably makes calculations wrong, but hard to see why
//  currency operator-() const {return currency(bourn_cast<double>(-m_));}

    currency& operator+=(currency z) {m_ += z.m_; return *this;}
    currency& operator-=(currency z) {m_ -= z.m_; return *this;}
    // NOPE!
//  currency& operator*=(currency z) {m_ *= z.m_; return *this;}

    currency& operator+=(double z) {m_ += from_double(z); return *this;}
    currency& operator-=(double z) {m_ -= from_double(z); return *this;}
    // NOPE!
//  currency& operator*=(double z) {m_ *= from_double(z); return *this;}
    // Check result range (and avoid multiplying by 100/100):
//  currency& operator*=(double z) {m_ = bourn_cast<data_type>(100.0 * to_double() * z); return *this;}
    // Far too permissive:
//  currency& operator*=(double z) {m_ = static_cast<data_type>(100.0 * to_double() * z); return *this;}
    double operator*=(double z) {return to_double() * z;}
// Dangerous--can somehow take the place of operator*(double)
//  currency const& operator*=(int z) {m_ *= z; return *this;}

  private:
    // Want something just slightly more permissive:
//  data_type from_double(double d) const {return bourn_cast<data_type>(100.0 * d);}
    // Far too permissive:
//  data_type from_double(double d) const {return static_cast<data_type>(100.0 * d);}
    // ...and a bit insidious:
//  data_type from_double(double d) const {return static_cast<data_type>(100.000000000001 * d);}
    // ...less bad:
    data_type from_double(double d) const {return round(100.0 * d);}
    double to_double() const {return bourn_cast<double>(m_) / 100.0;}

    data_type round(double d) const
        {
        static round_to<double> const r(0, r_to_nearest);
        return static_cast<data_type>(r(d));
        }

    data_type m_ = {0};
};

inline currency operator+(currency lhs, double rhs) {return lhs += currency(rhs);}
inline currency operator-(currency lhs, double rhs) {return lhs -= currency(rhs);}
//inline currency operator*(currency lhs, double rhs) {return lhs *= currency(rhs);}
////inline double operator*(currency lhs, double rhs) {return lhs *= currency(rhs);}
inline double operator*(currency lhs, double rhs) {return lhs.operator double() * rhs;}
//inline currency operator*(currency lhs, int rhs) {return lhs *= rhs;}
//inline currency operator*(int lhs, currency rhs) {return rhs *= lhs;}

inline std::ostream& operator<<(std::ostream& os, currency const& c)
{
//  return os << c.m_ << ' ' << c.to_double();
    return os << c.to_double();
}

#endif // defined USE_CURRENCY_CLASS

// Sloppy.
inline currency requantize(double z) {return currency(z);}

inline std::vector<currency> currencyize(std::vector<double> const& z)
{
    std::vector<currency> r;
    r.reserve(z.size());
    for(auto const& i : z)
        r.push_back(currency(i));
    return r;
}

inline double doubleize(currency const& z)
{
    return currency(z); // This implementation seems surprising.
}

inline std::vector<double> doubleize(std::vector<currency> const& z)
{
    std::vector<double> r;
    r.reserve(z.size());
    for(auto const& i : z)
//      r.push_back(i.operator double()); // no need to convert explicitly
        r.push_back(i);
    return r;
}

#endif // currency_hpp
