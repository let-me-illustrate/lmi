// IEEE 754 esoterica--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "ieee754.hpp"

#include "test_tools.hpp"

#include <limits>

int test_main(int, char*[])
{
    LMI_TEST(is_infinite(infinity<float      >()));
    LMI_TEST(is_infinite(infinity<double     >()));
    LMI_TEST(is_infinite(infinity<long double>()));

    // Narrowing conversions commented out here:
    LMI_TEST(is_infinite<float      >(-infinity<float      >()));
//  LMI_TEST(is_infinite<float      >(-infinity<double     >()));
//  LMI_TEST(is_infinite<float      >(-infinity<long double>()));
    LMI_TEST(is_infinite<double     >(-infinity<float      >()));
    LMI_TEST(is_infinite<double     >(-infinity<double     >()));
//  LMI_TEST(is_infinite<double     >(-infinity<long double>()));
    LMI_TEST(is_infinite<long double>(-infinity<float      >()));
    LMI_TEST(is_infinite<long double>(-infinity<double     >()));
    LMI_TEST(is_infinite<long double>(-infinity<long double>()));

    // Narrowing conversions tested here:
#if defined LMI_GCC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif // defined LMI_GCC
#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#endif // defined LMI_CLANG
    LMI_TEST(is_infinite<float      >(-infinity<double     >()));
    LMI_TEST(is_infinite<float      >(-infinity<long double>()));
    LMI_TEST(is_infinite<double     >(-infinity<long double>()));
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG
#if defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined LMI_GCC

    LMI_TEST(!is_infinite(0.0));
    LMI_TEST(!is_infinite( std::numeric_limits<double>::max()));
    LMI_TEST(!is_infinite(-std::numeric_limits<double>::max()));

    if(std::numeric_limits<double>::has_quiet_NaN)
        {
        std::cout << "has quiet NaN" << std::endl;
        }
    else
        {
        std::cout << "lacks quiet NaN" << std::endl;
        }

    float       x = implausible_value<float>      ();
    double      y = implausible_value<double>     ();
    long double z = implausible_value<long double>();

    bool xx = x == x;
    LMI_TEST(!xx);
    bool yy = y == y;
    LMI_TEST(!yy);
    bool zz = z == z;
    LMI_TEST(!zz);

    return 0;
}
