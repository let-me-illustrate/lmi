// Represent an amount in currency units and subunits.
//
// Copyright (C) 2016 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifndef currency_hpp
#define currency_hpp

#include "config.hpp"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

/// Represent a monetary amount as an exact number of base units and subunits.
///
/// This class currently assumes that there are 100 subunits in the base unit,
/// which is not the case for all currencies, but definitely is for USD, which
/// is the only currency used in lmi.
///
/// By storing the amount as an integer number of units and subunits internally
/// this class avoids rounding errors for the operations involving additions
/// and subtractions. For the multiplicative operations, conversions to and
/// from floating point type are provided and it is the caller responsibility
/// to correctly round the final result of a calculation involving such
/// operations to a currency amount.
///
/// This class provides value-like semantics and has a small size, making it
/// appropriate to pass it by value instead of more usual const reference.

class currency
{
  public:
    // Using int32_t for the value would limit this class to ~200 million units
    // which is insufficient, so use 64 bit type which allows to represent
    // values up to almost 1 quintillion which should be sufficient.
    using amount_type = std::int_fast64_t;

    static constexpr int subunits_digits = 2;
    static constexpr int subunits_per_unit = 100; // std::pow(10, subunits_digits)

    static constexpr amount_type max_units()
        {
        return std::numeric_limits<amount_type>::max() / subunits_per_unit;
        }

    // Default-initialized currency objects is 0.
    currency()
        {
        }

    // Constructor from a positive number of units and subunits. The subunits
    // argument must be normalized i.e. positive and strictly less than
    // subunits_per_unit.
    currency(amount_type units, int subunits)
        {
        if(units < 0 || units >= max_units())
            {
            throw std::overflow_error("Currency amount out of range.");
            }

        if(subunits < 0 || subunits >= subunits_per_unit)
            {
            throw std::runtime_error("Invalid amount of currency subunits.");
            }

        subunits_ = subunits_per_unit*units + subunits;
        }

    // Static constructor from the fractional amount of units rounding them to
    // the nearest subunit. The argument may be positive or negative.
    static currency from_value(double d)
        {
        if(std::trunc(d) >= static_cast<double>(max_units()))
            {
            throw std::overflow_error("Currency amount out of range.");
            }

        // The check above ensures that the product fits into amount_type.
        return currency
            (static_cast<amount_type>(std::round(subunits_per_unit*d))
            );
        }

    currency(currency const&) = default;
    currency& operator=(currency const&) = default;
    ~currency() = default;

    // Accessors.

    // The number of units may be negative.
    amount_type units() const
        {
        return subunits_ / subunits_per_unit;
        }
    // The number of subunits may also be negative and will always be if the
    // number of units is, i.e. -12.34 is (-12) units + (-34) subunits.
    int subunits() const
        {
        return subunits_ % subunits_per_unit;
        }

    // Total number of subunits, i.e. 123 for 1 unit and 23 subunits.
    amount_type total_subunits() const
        {
        return subunits_;
        }

    // Value in terms of units, to be used for arithmetic operations not
    // provided by this class itself.
    double value() const
        {
        double result = subunits_;
        result /= subunits_per_unit;
        return result;
        }

    // Comparisons.
    bool operator< (currency other) const { return subunits_ <  other.subunits_; }
    bool operator<=(currency other) const { return subunits_ <= other.subunits_; }
    bool operator==(currency other) const { return subunits_ == other.subunits_; }
    bool operator!=(currency other) const { return subunits_ != other.subunits_; }
    bool operator> (currency other) const { return subunits_ >  other.subunits_; }
    bool operator>=(currency other) const { return subunits_ >= other.subunits_; }

    // Arithmetic operations.
    currency operator-() const
        {
        return currency(-subunits_);
        }

    currency& operator+=(currency other)
        {
        subunits_ += other.subunits_;
        return *this;
        }

    currency& operator-=(currency other)
        {
        subunits_ -= other.subunits_;
        return *this;
        }

    currency& operator*=(int factor)
        {
        subunits_ *= factor;
        return *this;
        }

  private:
    // This ctor is only used internally, it is too error-prone to expose it
    // publicly.
    explicit currency(amount_type subunits)
        :subunits_(subunits)
        {
        }

    amount_type subunits_ = 0;
};

inline currency operator+(currency lhs, currency rhs)
{
    return currency(lhs) += rhs;
}

inline currency operator-(currency lhs, currency rhs)
{
    return currency(lhs) -= rhs;
}

inline currency operator*(currency lhs, int rhs)
{
    return currency(lhs) *= rhs;
}

inline currency operator*(int lhs, currency rhs)
{
    return currency(rhs) *= lhs;
}

inline std::ostream& operator<<(std::ostream& os, currency c)
{
    // When formatting a negative currency amount, there should be no sign
    // before the number of subunits: "-12.34" and not "-12.-34". On the other
    // hand side, we need to output the sign manually for negative amount
    // because we can't rely on it appearing as part of c.units(): this doesn't
    // work when units amount is 0.
    //
    // Decimal point is currently hard-coded as it is always the appropriate
    // separator to use for lmi.
    if(c.total_subunits() < 0)
        {
        os << '-';
        }

    return os
        << std::abs(c.units())
        << '.'
        << std::setfill('0')
        << std::setw(currency::subunits_digits)
        << std::abs(c.subunits());
}

inline std::istream& operator>>(std::istream& is, currency& c)
{
    // We can't rely on comparing units with 0 as this doesn't work for
    // "-0.xx", so test for the sign ourselves and skip it if it's there.
    bool const negative = is.peek() == '-';
    if(negative)
        {
        is.get();
        }

    currency::amount_type units = 0;
    is >> units;
    if(!is)
        return is;

    if(is.get() != '.')
        {
        is.setstate(std::ios_base::failbit);
        return is;
        }

    int subunits = 0;
    is >> subunits;
    if(!is)
        return is;

    if(subunits < 0 || subunits >= currency::subunits_per_unit)
        {
        is.setstate(std::ios_base::failbit);
        return is;
        }

    c = currency(units, subunits);
    if(negative)
        {
        c = -c;
        }

    return is;
}

#endif // currency_hpp
