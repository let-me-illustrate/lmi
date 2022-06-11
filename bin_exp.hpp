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

#include <limits>
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
///
/// The last line conditionally forms the reciprocal of 'y', which
/// C++20 says is UB if ±0.0 == y. However, it's well defined by
/// IEEE 754 (and lmi is built with compiler flags to prescribe
/// IEEE 754 behavior to the fullest extent possible), so it would
/// be unreasonable for a compiler to perform any optimization that
/// assumes otherwise. See:
///   https://bugs.llvm.org/show_bug.cgi?id=19535#c1
///
/// IEEE754-2008's "Division by zero" section [7.3] specifies:
/// | The default result of divideByZero shall be an ∞ correctly
/// | signed according to the operation
/// The C and C++ standards should specifically permit IEEE 754
/// semantics by making floating-point division by zero implementation
/// -defined rather than undefined behavior.
///
/// C++20 [7.6.5/4] says:
/// | If the second operand of / or % is zero the behavior is undefined.
/// Although C99 [6.5.5/5] says:
/// | if the value of the second operand is zero, the behavior is
/// | undefined.
/// its normative Annex F allows an implementation to define
/// __STDC_IEC_559__, in which case [F.1]
/// | the IEC 60559-specified behavior is adopted by reference,
/// | unless stated otherwise.
/// It isn't completely clear whether that exception means "unless
/// stated otherwise in Annex F"; but if it includes [6.5.5/5] as
/// well, then the four examples of division by zero in [F.7.4/2]
/// are incorrect: whether they raise an exception or not cannot
/// be specified as indicated in the comments, because that would
/// be undefined.

template<typename T>
#if defined LMI_GCC || defined LMI_CLANG
__attribute__((no_sanitize("float-divide-by-zero")))
#endif // defined LMI_GCC || defined LMI_CLANG
constexpr T bin_exp(T x, int exponent)
{
    static_assert(std::is_floating_point_v<T>);
    static_assert(std::numeric_limits<T>::is_iec559);
    bool is_exponent_negative {exponent < 0};
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
    return is_exponent_negative ? 1 / y : y;
}

double Algorithm_A(double x, int n);

#endif // bin_exp_hpp
