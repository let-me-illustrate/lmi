// TAOCP 4.6.3 Right-to-left binary method for exponentiation.
//
// Copyright (C) 2022 Gregory W. Chicares.
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

#ifndef bin_exp_hpp
#define bin_exp_hpp

#include "config.hpp"

#include "math_functions.hpp"           // u_abs()

#include <type_traits>                  // is_floating_point_v

/// Binary method for exponentiation.
///
/// Allow only floating-point types for the base argument, because
/// integer types can overflow. Allow only 'int' for the exponent
/// argument, because it can't be narrower than 32 bits in any
/// environment where lmi builds, and exponents over 2^32 aren't
/// needed in practice.
///
/// See Knuth, TAOCP volume 2, section 4.6.3, which notes (p. 443
/// in 2nd ed.):
///   "The number of multiplications required by Algorithm A
///   is ⌊lg n⌋ + ν(n), where ν(n) is the number of ones in the
///   binary representation of n. This is one more multiplication
///   than the left-to-right binary method ... would require, due
///   to the fact that the first execution of step A3 is simply a
///   multiplication by unity."
/// This seems to be an inefficiency that ought to be removed.
/// However, initializing the result to unity takes care of the case
/// where the exponent is zero. Attempting to remove the needless
/// multiplication by unity, while preserving correctness when the
/// exponent is zero, is surely possible, but several attempts just
/// produced more complex code that ran no faster.
///
/// Others often write bitwise operators instead of multiplicative.
/// That's incorrect for signed integers:
///   (-1 % 2) = -1, whereas
///   (-1 & 1) =  1; and
///   (-1 / 2) =  0, whereas
///   (-1 >>1) = -1;
/// and twenty-first-century optimizers generate the same code for
/// unsigned values anyway.

template<typename T>
constexpr T bin_exp(T x, int exponent)
{
    static_assert(std::is_floating_point_v<T>);
    bool negative_exponent {exponent < 0};
    unsigned int n = u_abs(exponent);
    T y = 1;
    for(;;)
        {
        if(0 != n % 2)
            y *= x;
        n /= 2;
        if(0 == n)
            break;
        x *= x;
        }
    return negative_exponent ? 1 / y : y;
}

double Algorithm_A(double x, int n);

#endif // bin_exp_hpp
