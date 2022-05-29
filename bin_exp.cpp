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

#include "pchfile.hpp"

#include "bin_exp.hpp"

#include <cstdio>                       // printf()

/// Binary method for exponentiation.
///
/// See Knuth, TAOCP volume 2, section 4.6.3 (p. 442 in 2nd ed.).

double Algorithm_A(double x, int n)
{
    if(n <= 0) throw "n must be positive";
    int mult_count {0};
//A1:
    int    N {n};
    double Y {1.0};
    double Z {x};
    std::printf("               %3s  %7s  %7s\n"  , "N", "Y", "Z");
    std::printf("After step A1  %3i  %7.f  %7.f\n",  N,   Y,   Z );
  A2: // [Halve N.] (At this point, x^n = Y * Z^N .)
    bool was_even = 0 == N % 2;
std::printf("%40s  %3i %s\n", "A2:", N, was_even ? "even" : "odd");
    N /= 2; // integer division truncates
    if(was_even) goto A5;
//A3: // [Multiply Y by Z.]
std::printf("%40s #%i %7.f × %7.f → %7.f\n", "A3:", mult_count, Y, Z, Y*Z);
    Y *= Z;
    ++mult_count;
//A4: // [N == 0?]
    std::printf("After step A4  %3i  %7.f  %7.f\n",  N,   Y,   Z );
    if(0 == N)
        {
        std::printf("Algorithm A: %i multiplications\n\n", mult_count);
        return Y;
        }
  A5: // [Square Z.]
std::printf("%40s #%i %7.f ^ %7.f → %7.f\n", "A5:", mult_count, Z, 2.0, Z*Z);
    Z *= Z;
    ++mult_count;
    goto A2;
}
