// Testing sandbox.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "test_tools.hpp"

#include <cstdio>

// Binary method for exponentiation.
//
// See Knuth, TAOCP volume 2, section 4.6.3 (p. 442 in 2nd ed.);
// and SGI's power(), present elsewhere in the lmi sources.
//
// The printf statements that aren't commented out print a table
// like Knuth's example. Enable the ones that are commented out
// to see the details of each multiplication. Or :g/printf/d
// to remove all that clutter.
//
// Knuth's algorithm takes one more multiplication for 'Y *= Z'
// when Y has its initial value of unity. SGI refactors it to
// avoid a goto, but the result is harder to understand.

#pragma GCC diagnostic ignored "-Wunused-label"

// TAOCP, volume 2, section 4.6.3, page 442
double Algorithm_A(double x, int n)
{
    if(n <= 0) throw "n must be positive";
    int mult_count {0};
  A1:
    int    N {n};
    double Y {1.0};
    double Z {x};
    std::printf("               %3s  %7s  %7s\n"  , "N", "Y", "Z");
    std::printf("After step A1  %3i  %7.f  %7.f\n",  N,   Y,   Z );
  A2: // [Halve N.] (At this point, x^n = Y * Z^N .)
    bool was_even = 0 == N % 2;
    N /= 2; // integer division truncates
    if(was_even) goto A5;
  A3: // [Multiply Y by Z.]
// std::printf("multiply #%i %7.f by %7.f yielding %7.f\n", mult_count, Y, Z, Y*Z);
    Y *= Z;
    ++mult_count;
  A4: // [N == 0?]
    std::printf("After step A4  %3i  %7.f  %7.f\n",  N,   Y,   Z );
    if(0 == N)
        {
        std::printf("Algorithm A: %i multiplications\n", mult_count);
        return Y;
        }
  A5: // [Square Z.]
//std::printf("multiply #%i %7.f by %7.f yielding %7.f\n", mult_count, Z, Z, Z*Z);
    Z *= Z;
    ++mult_count;
    goto A2;
}

// SGI extension to STL, somewhat refactored for clarity
double power(double x, int n)
{
    if(n < 0)
        {
        throw std::logic_error("power() called with negative exponent.");
        }

    int mult_count {0};
    while(0 == n % 2) // while ((n & 1) == 0)
        {
        n /= 2; // n >>= 1;
// std::printf("multiply #%i %7.f by %7.f yielding %7.f\n", mult_count, x, x, x*x);
        x *= x;
        ++mult_count;
std::printf("After step B1 %3i  %7.f\n", n, x);
        }
    double result = x;
    n /= 2; // n >>= 1;
    while (n != 0)
        {
// std::printf("multiply #%i %7.f by %7.f yielding %7.f\n", mult_count, x, x, x*x);
        x *= x;
        ++mult_count;
std::printf("After step B2 %3i  %7.f\n", n, x);
        if(0 != n % 2) // if((n & 1) != 0)
            {
// std::printf("multiply #%i %7.f by %7.f yielding %7.f\n", mult_count, result, x, result*x);
            result *= x;
            ++mult_count;
            }
        n /= 2; // n >>= 1;
        }
std::printf("power(): %i multiplications\n", mult_count);
    return result;
}

int test_main(int, char*[])
{
    LMI_TEST(8388608 == Algorithm_A(2.0, 23));
    LMI_TEST(8388608 == power      (2.0, 23));

    LMI_TEST(2 * 8388608 == Algorithm_A(2.0, 24));
    LMI_TEST(2 * 8388608 == power      (2.0, 24));

    return 0;
}
