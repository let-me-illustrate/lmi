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

#ifndef monnaie_hpp
#define monnaie_hpp

#include "config.hpp"

#include "bourn_cast.hpp"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

/// Represent a currency amount exactly as integral cents.
///
/// This class is tailored to US currency, as lmi is tailored to US
/// life insurance.
///
/// By storing the amount as an integer number of cents internally,
/// this class avoids roundoff error for addition and subtraction.
/// For multiplicative operations, conversions to and from double
/// point type are provided; it is the caller's responsibility to
/// round the final result of such calculations to a currency amount.
///
/// This class provides value-like semantics and has a small size,
/// making it appropriate to pass instances by value instead of the
/// more usual const reference.

class monnaie
{
    friend class monnaie_test;

  public:
    /// Using int32_t for the value would limit the range to about
    /// twenty million dollars, which is insufficient; but int64_t
    /// accommodates values up to about ninety quadrillion dollars,
    /// which is enough for any life insurance contract in 2016.

    using amount_type = std::int_fast64_t;

    static constexpr int cents_digits = 2;
    static constexpr int cents_per_dollar = 100; // pow(10, cents_digits)

    static constexpr amount_type max_dollars()
        {
        return std::numeric_limits<amount_type>::max() / cents_per_dollar;
        }

    /// No ctor-initializer-list is needed because the lone data
    /// member has a member-initializer at its point of declaration.

    monnaie() = default;

    /// Constructor from a positive number of dollars and cents.
    ///
    /// The cents argument must be normalized: i.e., positive and
    /// strictly less than cents_per_dollar.

    monnaie(amount_type dollars, int cents)
        {
        if(!(0 <= dollars && dollars < max_dollars()))
            {
            throw std::overflow_error("Currency amount out of range.");
            }

        if(!(0 <= cents && cents < cents_per_dollar))
            {
            throw std::runtime_error("Invalid number of cents.");
            }

        cents_ = cents_per_dollar * dollars + cents;
        }

    /// Convert from floating-point dollars.
    ///
    /// The argument may be positive or negative. Its value is rounded
    /// to the nearest cent.

    static monnaie from_value(double d)
        {
        if(static_cast<double>(max_dollars()) <= std::trunc(d))
            {
            throw std::overflow_error("Currency amount out of range.");
            }

        // The check above ensures that the value fits in amount_type.
        return monnaie
            (static_cast<amount_type>(std::round(cents_per_dollar * d))
            );
        }

    monnaie(monnaie const&) = default;
    monnaie& operator=(monnaie const&) = default;
    ~monnaie() = default;

    // Accessors.

    /// Number of whole dollars. May be negative.

    amount_type dollars() const
        {
        return cents_ / cents_per_dollar;
        }

    /// Number of whole cents. May be negative.
    ///
    /// The number of cents must be negative if the number of dollars
    /// is negative. The number of cents may be negative if the number
    /// of dollars is zero. Otherwise the number of cents must be
    /// nonnegative.

    int cents() const
        {
        return bourn_cast<int>(cents_ % cents_per_dollar);
        }

    /// Total number of cents, e.g., 123 for 1 dollar and 23 cents.

    amount_type total_cents() const
        {
        return cents_;
        }

    /// Value as floating-point dollars, for mixed-mode arithmetic.

    double value() const
        {
        double result = bourn_cast<double>(cents_);
        result /= cents_per_dollar;
        return result;
        }

    // Comparisons.
    bool operator< (monnaie other) const { return cents_ <  other.cents_; }
    bool operator<=(monnaie other) const { return cents_ <= other.cents_; }
    bool operator==(monnaie other) const { return cents_ == other.cents_; }
    bool operator!=(monnaie other) const { return cents_ != other.cents_; }
    bool operator> (monnaie other) const { return cents_ >  other.cents_; }
    bool operator>=(monnaie other) const { return cents_ >= other.cents_; }

    // Arithmetic operations.
    monnaie operator-() const
        {
        return monnaie(-cents_);
        }

    monnaie& operator+=(monnaie other)
        {
        cents_ += other.cents_;
        return *this;
        }

    monnaie& operator-=(monnaie other)
        {
        cents_ -= other.cents_;
        return *this;
        }

    monnaie& operator*=(int factor)
        {
        cents_ *= factor;
        return *this;
        }

  private:
    /// This ctor is only used internally: it is too error-prone to
    /// expose it publicly.

    explicit monnaie(amount_type cents)
        :cents_ {cents}
        {
        }

    amount_type cents_ = 0;
};

inline monnaie operator+(monnaie lhs, monnaie rhs)
{
    return lhs += rhs;
}

inline monnaie operator-(monnaie lhs, monnaie rhs)
{
    return lhs -= rhs;
}

inline monnaie operator*(monnaie lhs, int rhs)
{
    return lhs *= rhs;
}

inline monnaie operator*(int lhs, monnaie rhs)
{
    return rhs *= lhs;
}

/// Insert the dollars-and-cents amount into a stream.
///
/// Dollars and cents, being exact integers, are formatted separately,
/// but the negative sign cannot be supplied by either of those two
/// separate formatting operations: $-12.34 must not be inserted as
/// "-12.-34"; and $-0.56 must be inserted as "-0.56" even though the
/// whole-dollar amount is not negative.
///
/// The decimal mark is hard-coded as '.' because that is universal
/// US practice.

inline std::ostream& operator<<(std::ostream& os, monnaie c)
{
    if(c.total_cents() < 0)
        {
        os << '-';
        }

    return os
        << std::abs(c.dollars())
        << '.'
        << std::setfill('0')
        << std::setw(monnaie::cents_digits)
        << std::abs(c.cents());
}

/// Extract a dollars-and-cents amount from a stream.
///
/// The negative sign requires special attention so that $-0.56 is not
/// extracted as -0 dollars plus 56 cents.
///
/// The decimal mark is hard-coded as '.' because that is universal
/// US practice.

inline std::istream& operator>>(std::istream& is, monnaie& c)
{
    bool const negative = is.peek() == '-';
    if(negative)
        {
        is.get();
        }

    monnaie::amount_type dollars = 0;
    is >> dollars;
    if(!is)
        {
        return is;
        }

    if(is.get() != '.')
        {
        is.setstate(std::ios_base::failbit);
        return is;
        }

    int cents = 0;
    is >> cents;
    if(!is)
        {
        return is;
        }

    if(!(0 <= cents && cents < monnaie::cents_per_dollar))
        {
        is.setstate(std::ios_base::failbit);
        return is;
        }

    c = monnaie(dollars, cents);
    if(negative)
        {
        c = -c;
        }

    return is;
}

#endif // monnaie_hpp
