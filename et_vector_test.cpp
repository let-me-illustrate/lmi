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

#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <functional>                   // multiplies(), negate(), plus()
#include <limits>
#include <vector>

// There can be no volatile standard container.
static std::vector<int>        iv0
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
static std::vector<int> const  iv1
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    ,0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    ,0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    ,0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    ,0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };

// Copied from "miscellany.hpp", and renamed.
template<typename RangeExpression, typename T>
bool each_equal_cplusplus(RangeExpression const& range, T const& t)
{
    for(auto const& i : range)
        {
        if(t != i) return false;
        }
    return true;
}

template<typename T>
bool each_equal_pete(std::vector<T> v, T t)
{
    return AllOf(EqualTo(v, t));
}

void test_each_equal()
{
    // Test with containers.

    LMI_TEST( each_equal_cplusplus(iv0, 0));
    LMI_TEST(!each_equal_cplusplus(iv1, 0));

    LMI_TEST( each_equal_pete(iv0, 0));
    LMI_TEST(!each_equal_pete(iv1, 0));

    // By arbitrary definition, any value compares equal to an empty
    // range.

    std::vector<int> vi_empty;
    LMI_TEST( each_equal_cplusplus(vi_empty, 23456));
    LMI_TEST( each_equal_pete     (vi_empty, 23456));

    // That arbitrary definition extends even to qNaN, which doesn't
    // compare equal to anything--because these tests perform zero
    // comparisons.

    double const qnan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> vd_empty;
    LMI_TEST( each_equal_cplusplus(vd_empty, qnan));
    LMI_TEST( each_equal_pete     (vd_empty, qnan));
}

// A vector of boolean values, represented as double, such as
// might result from querying lmi's database.
static std::vector<double> const bit_valued =
    {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0
    ,1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    ,0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0
    ,1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    ,0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0
    ,1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    ,0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0
    ,1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    ,0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0
    ,1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    };

bool mete_eq0a()
{
    for(auto const& i : bit_valued)
        {
        if(false != i && true != i)
            return false;
        }
    return true;
}

bool mete_eq0b()
{
    for(auto const& i : bit_valued)
        {
        if(0.0 != i && 1.0 != i)
            return false;
        }
    return true;
}

bool mete_eq0c()
{
    for(auto const& i : bit_valued)
        {
        if(i != !!i)
            return false;
        }
    return true;
}

bool mete_eq1a()
{
    return AllOf(EqualTo(true, bit_valued) || EqualTo(false, bit_valued));
}

bool mete_eq1b()
{
    return AllOf(EqualTo(1.0, bit_valued) || EqualTo(0.0, bit_valued));
}

bool mete_eq1c()
{
    return AllOf(EqualTo(bit_valued, !!bit_valued));
}

bool mete_eq1x()
{
    return !AnyOf(UnequalTo(true, bit_valued) && UnequalTo(false, bit_valued));
}

bool mete_eq2s()
{
    return
            each_equal_cplusplus(iv0, 0)
        && !each_equal_cplusplus(iv1, 0)
        ;
}

bool mete_eq2t()
{
    return
            each_equal_pete(iv0, 0)
        && !each_equal_pete(iv1, 0)
        ;
}

void assay_speed()
{
    LMI_TEST_EQUAL(true, mete_eq0a());
    LMI_TEST_EQUAL(true, mete_eq0b());
    LMI_TEST_EQUAL(true, mete_eq0c());
    LMI_TEST_EQUAL(true, mete_eq1a());
    LMI_TEST_EQUAL(true, mete_eq1b());
    LMI_TEST_EQUAL(true, mete_eq1c());
    LMI_TEST_EQUAL(true, mete_eq1x());
    LMI_TEST_EQUAL(true, mete_eq2s());
    LMI_TEST_EQUAL(true, mete_eq2t());
    std::cout << "Speed tests:\n";
    std::cout << "  mete_eq0a " << TimeAnAliquot(mete_eq0a) << '\n';
    std::cout << "  mete_eq0b " << TimeAnAliquot(mete_eq0b) << '\n';
    std::cout << "  mete_eq0c " << TimeAnAliquot(mete_eq0c) << '\n';
    std::cout << "  mete_eq1a " << TimeAnAliquot(mete_eq1a) << '\n';
    std::cout << "  mete_eq1b " << TimeAnAliquot(mete_eq1b) << '\n';
    std::cout << "  mete_eq1c " << TimeAnAliquot(mete_eq1c) << '\n';
    std::cout << "  mete_eq1x " << TimeAnAliquot(mete_eq1x) << '\n';
    std::cout << "  mete_eq2s " << TimeAnAliquot(mete_eq2s) << '\n';
    std::cout << "  mete_eq2t " << TimeAnAliquot(mete_eq2t) << '\n';
}

int test_main(int, char*[])
{
    {
    std::vector<double> v0 = {0.0, 1.25, 2.5};
    v0 *= v0;
    std::vector<double> const r0 = {0.0, 1.5625, 6.25};
    LMI_TEST(r0 == v0);
    }

    // Test length determination.
    {
    std::vector<double> v0 = {0.5, 1.5, 2.5};
    std::vector<double> v1 = {2.0, 3.0, 4.0};
    LMI_TEST_EQUAL(3, forEach(v0, LengthLeaf(), MaxCombine()));
    LMI_TEST_EQUAL(3, forEach(v0 / v1 + v0 * v1, LengthLeaf(), MaxCombine()));
    // Rho(std::vector<T> const&) could be supported, of course, but
    // it seems better to restrict Rho() to 'Expression' instances.
//  LMI_TEST_EQUAL(3, Rho(v0));
    LMI_TEST_EQUAL(3, Rho(v0 / v1 + v0 * v1));
    }

    // Test non-conformable assignment.
    {
    std::vector<double> v0 = {0.0, 1.25, 2.5};
    std::vector<double> v1 = {0.0, 1.25, 2.5, 3.75};
    char const* s {"Nonconformable lengths: 3 lhs vs. 4 rhs."};
    LMI_TEST_THROW(v0 *= v1, std::runtime_error, s);
    }

    // Test assignment to vector.
    {
    std::vector<double> v0 = {1.0, 1.25, 1.5};
    std::vector<double> v1 = {0.0, 0.25, 0.5};
    std::vector<double> const x = Eval(v0 + v1);
    auto                const y = Eval(v0 + v1 + x);
    std::vector<double> const r0 = {1.0, 1.5, 2.0};
    LMI_TEST(r0 == x);
    std::vector<double> const r1 = {2.0, 3.0, 4.0};
    LMI_TEST(r1 == y);
    }

    // Test "assignment" operator<<=().
    {
    std::vector<double> v0 = {1.0, 1.25, 1.5};
    std::vector<double> v1 = {0.0, 0.25, 0.5};

    std::vector<double> w(5); // Error: not of conformable length.
    char const* s {"Nonconformable lengths: 5 lhs vs. 3 rhs."};
    LMI_TEST_THROW(assign(w, v0 + v1), std::runtime_error, s);

    std::vector<double> x(3); // Must be of conformable length.
    assign(x, v0 + v1);
    std::vector<double> const r0 = {1.0, 1.5, 2.0};
    LMI_TEST(r0 == x);

    std::vector<double> y(7); // Needn't be of conformable length.
    y <<= v0 + v1 + x;
    std::vector<double> const r1 = {2.0, 3.0, 4.0};
    LMI_TEST(r1 == y);
    LMI_TEST_EQUAL(3, lmi::ssize(y));
    }

    // Test peteCast().
    {
    std::vector<double> v0 = {0.0, 1.25, 2.5};
    std::vector<int> v1(v0.size());
    peteCast(int{}, v0); // Legal, but feckless.
    assign(v1, peteCast(int{}, v0));
    std::vector<int> const r1 = {0, 1, 2};
    LMI_TEST(r1 == v1);
    }

    // Test what used to be called std::unary_function.
    {
    std::vector<double> v0 = {-1.0, 0.0, 3.875};
    assign(v0, apply_unary(std::negate<double>(), v0));
    std::vector<double> const r0 = {1.0, 0.0, -3.875};
    LMI_TEST(r0 == v0);
    }

    // Test what used to be called std::binary_function.
    {
    std::vector<double> v0 = {0.0, -1.5625, -6.25};
    assign(v0, apply_binary(std::multiplies<double>(), -1.0, v0));
    std::vector<double> const r0 = {0.0, 1.5625, 6.25};
    LMI_TEST(r0 == v0);

    assign(v0, sqrt(v0));
    std::vector<double> const r1 = {0.0, 1.25, 2.5};
    LMI_TEST(r1 == v0);

    // Above, scalar -1.0 was LHS; here, it's RHS.
    assign(v0, apply_binary(std::multiplies<double>(), v0, -1.0));
    std::vector<double> const r2 = {0.0, -1.25, -2.5};
    LMI_TEST(r2 == v0);

    assign(v0, apply_binary(std::multiplies<double>(), v0, v0));
    std::vector<double> const r3 = {0.0, 1.5625, 6.25};
    LMI_TEST(r3 == v0);

    // Right-add 100, left-add 10000 .
    assign(v0, apply_binary(std::plus<double>(), v0, 100.0));
    assign(v0, apply_binary(std::plus<double>(), 10000.0, v0));
    std::vector<double> const r4 = {10100.0, 10101.5625, 10106.25};
    LMI_TEST(r4 == v0);
    }

    // Test Min() and Max().
    {
    std::vector<double> v2 = {1.125, 2.25, 3.375, 7.75};
    std::vector<double> v3 = {1.875, 2.875, 3.875, 0.0};
    std::vector<double> v4(v2.size());
    assign(v4, Max(v2, v3));
    std::vector<double> const r0 = {1.875, 2.875, 3.875, 7.75};
    LMI_TEST(r0 == v4);

    assign(v4, Min(v2, v3));
    std::vector<double> const r1 = {1.125, 2.25, 3.375, 0.0};
    LMI_TEST(r1 == v4);

    assign(v4, Min(Max(1.25, Min(v2, v3)), 3.125));
    std::vector<double> const r2 = {1.25, 2.25, 3.125, 1.25};
    LMI_TEST(r2 == v4);
    }

    // Test reductions.
    //
    // Unary '+' is not defined for std::vector, but it is for PETE
    // expressions, so '+v' converts a std::vector 'v' into a PETE
    // expression.
    {
    std::vector<double> v0 = { 0.0, 0.0, 0.0};
    std::vector<double> v1 = { 0.0, 1.0, 0.0};
    std::vector<double> v2 = { 1.0, 1.0, 1.0};
    std::vector<double> v3 = {-1.0, 0.0, 6.5};
    std::vector<double> v4 = {-1.0, 4.0, 6.5};

    LMI_TEST(false == AllOf(+v0));
    LMI_TEST(false == AllOf(+v1));
    LMI_TEST(true  == AllOf(+v2));
    LMI_TEST(false == AllOf(+v3));
    LMI_TEST(true  == AllOf(+v4));

    LMI_TEST(false == AnyOf(+v0));
    LMI_TEST(true  == AnyOf(+v1));
    LMI_TEST(true  == AnyOf(+v2));
    LMI_TEST(true  == AnyOf(+v3));
    LMI_TEST(true  == AnyOf(+v4));

    LMI_TEST_EQUAL( 0.0, SumOf(+v0));
    LMI_TEST_EQUAL( 1.0, SumOf(+v1));
    LMI_TEST_EQUAL( 3.0, SumOf(+v2));
    LMI_TEST_EQUAL( 5.5, SumOf(+v3));
    LMI_TEST_EQUAL( 9.5, SumOf(+v4));

    LMI_TEST_EQUAL(  0.0, ProductOf(+v0));
    LMI_TEST_EQUAL(  0.0, ProductOf(+v1));
    LMI_TEST_EQUAL(  1.0, ProductOf(+v2));
    LMI_TEST_EQUAL(  0.0, ProductOf(+v3));
    LMI_TEST_EQUAL(-26.0, ProductOf(+v4));

    LMI_TEST_EQUAL( 0.0, MaxOf(+v0));
    LMI_TEST_EQUAL( 1.0, MaxOf(+v1));
    LMI_TEST_EQUAL( 1.0, MaxOf(+v2));
    LMI_TEST_EQUAL( 6.5, MaxOf(+v3));
    LMI_TEST_EQUAL( 6.5, MaxOf(+v4));

    LMI_TEST_EQUAL( 0.0, MinOf(+v0));
    LMI_TEST_EQUAL( 0.0, MinOf(+v1));
    LMI_TEST_EQUAL( 1.0, MinOf(+v2));
    LMI_TEST_EQUAL(-1.0, MinOf(+v3));
    LMI_TEST_EQUAL(-1.0, MinOf(+v4));

    // Combination example: test whether vector is all boolean-valued.
    // In APL, one might write:
    //   AND slash [ravel] V ElementOf 0 1
    //   (using 'ravel' to flatten rank, which is irrelevant here)
    // Lacking ElementOf, this example uses:
    //   AND slash (v=0) OR v=1
    // In APL, one might instead write:
    //   AND slash V = NOT NOT V
    // using NOT NOT to turn values into booleans, but, using PETE
    // with std::vector, "!!V" returns a vector<bool>; ET !! perhaps
    // boolean operators should therefore be amended to return 'int'.
    LMI_TEST_EQUAL(true , AllOf(EqualTo(true, v0) || EqualTo(false, v0)));
    LMI_TEST_EQUAL(true , AllOf(EqualTo(true, v1) || EqualTo(false, v1)));
    LMI_TEST_EQUAL(true , AllOf(EqualTo(true, v2) || EqualTo(false, v2)));
    LMI_TEST_EQUAL(false, AllOf(EqualTo(true, v3) || EqualTo(false, v3)));
    LMI_TEST_EQUAL(false, AllOf(EqualTo(true, v4) || EqualTo(false, v4)));
    }

    test_each_equal();

    assay_speed();

    return 0;
}
