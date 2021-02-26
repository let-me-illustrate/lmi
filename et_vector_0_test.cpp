// PETE with std::vector: unit test.
//
// Copyright (C) 2008, 2010, 2016, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "et_vector.hpp"

#include "test_tools.hpp"

#include <functional>                   // multiplies(), negate(), plus()

int test_main(int, char*[])
{
    {
    std::vector<double> v0 = {0.0, 1.25, 2.5};
    v0 *= v0;
    std::vector<double> const r0 = {0.0, 1.5625, 6.25};
    BOOST_TEST(r0 == v0);
    }

    // Test peteCast().
    {
    std::vector<double> v0 = {0.0, 1.25, 2.5};
    std::vector<int> v1(v0.size());
    peteCast(int{}, v0); // Legal, but feckless.
    assign(v1, peteCast(int{}, v0));
    std::vector<int> const r1 = {0, 1, 2};
    BOOST_TEST(r1 == v1);
    }

    // Test what used to be called std::unary_function.
    {
    std::vector<double> v0 = {-1.0, 0.0, 3.875};
    assign(v0, apply_unary(std::negate<double>(), v0));
    std::vector<double> const r0 = {1.0, 0.0, -3.875};
    BOOST_TEST(r0 == v0);
    }

    // Test what used to be called std::binary_function.
    {
    std::vector<double> v0 = {0.0, -1.5625, -6.25};
    assign(v0, apply_binary(std::multiplies<double>(), -1.0, v0));
    std::vector<double> const r0 = {0.0, 1.5625, 6.25};
    BOOST_TEST(r0 == v0);

    assign(v0, sqrt(v0));
    std::vector<double> const r1 = {0.0, 1.25, 2.5};
    BOOST_TEST(r1 == v0);

    // Above, scalar -1.0 was LHS; here, it's RHS.
    assign(v0, apply_binary(std::multiplies<double>(), v0, -1.0));
    std::vector<double> const r2 = {0.0, -1.25, -2.5};
    BOOST_TEST(r2 == v0);

    assign(v0, apply_binary(std::multiplies<double>(), v0, v0));
    std::vector<double> const r3 = {0.0, 1.5625, 6.25};
    BOOST_TEST(r3 == v0);

    // Right-add 100, left-add 10000 .
    assign(v0, apply_binary(std::plus<double>(), v0, 100.0));
    assign(v0, apply_binary(std::plus<double>(), 10000.0, v0));
    std::vector<double> const r4 = {10100.0, 10101.5625, 10106.25};
    BOOST_TEST(r4 == v0);
    }

    // Test Min() and Max().
    {
    std::vector<double> v2 = {1.125, 2.25, 3.375, 7.75};
    std::vector<double> v3 = {1.875, 2.875, 3.875, 0.0};
    std::vector<double> v4(v2.size());
    assign(v4, Max(v2, v3));
    std::vector<double> const r0 = {1.875, 2.875, 3.875, 7.75};
    BOOST_TEST(r0 == v4);

    assign(v4, Min(v2, v3));
    std::vector<double> const r1 = {1.125, 2.25, 3.375, 0.0};
    BOOST_TEST(r1 == v4);

    assign(v4, Min(Max(1.25, Min(v2, v3)), 3.125));
    std::vector<double> const r2 = {1.25, 2.25, 3.125, 1.25};
    BOOST_TEST(r2 == v4);
    }

    return 0;
}
