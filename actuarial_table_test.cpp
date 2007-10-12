// Actuarial tables from SOA database--unit test.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: actuarial_table_test.cpp,v 1.19 2007-10-12 16:07:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "actuarial_table.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

namespace
{
// SOA database 'qx_cso' table 42
// "1980 US CSO Male Age nearest"
std::vector<double> table_42(int age)
    {
    static int const n = 100;
    static double const q[n] =
        //     0        1        2        3        4        5        6        7        8        9
        {0.00418, 0.00107, 0.00099, 0.00098, 0.00095, 0.00090, 0.00086, 0.00080, 0.00076, 0.00074 // 00
        ,0.00073, 0.00077, 0.00085, 0.00099, 0.00115, 0.00133, 0.00151, 0.00167, 0.00178, 0.00186 // 10
        ,0.00190, 0.00191, 0.00189, 0.00186, 0.00182, 0.00177, 0.00173, 0.00171, 0.00170, 0.00171 // 20
        ,0.00173, 0.00178, 0.00183, 0.00191, 0.00200, 0.00211, 0.00224, 0.00240, 0.00258, 0.00279 // 30
        ,0.00302, 0.00329, 0.00356, 0.00387, 0.00419, 0.00455, 0.00492, 0.00532, 0.00574, 0.00621 // 40
        ,0.00671, 0.00730, 0.00796, 0.00871, 0.00956, 0.01047, 0.01146, 0.01249, 0.01359, 0.01477 // 50
        ,0.01608, 0.01754, 0.01919, 0.02106, 0.02314, 0.02542, 0.02785, 0.03044, 0.03319, 0.03617 // 60
        ,0.03951, 0.04330, 0.04765, 0.05264, 0.05819, 0.06419, 0.07053, 0.07712, 0.08390, 0.09105 // 70
        ,0.09884, 0.10748, 0.11725, 0.12826, 0.14025, 0.15295, 0.16609, 0.17955, 0.19327, 0.20729 // 80
        ,0.22177, 0.23698, 0.25345, 0.27211, 0.29590, 0.32996, 0.38455, 0.48020, 0.65798, 1.00000 // 90
        };
    return std::vector<double>(q + age, q + n);
    }

// SOA database 'qx_cso' table 47
// "1980 US CSO Selection Factors, Female"
// Parameters: min age 0; max age 105; select period 10; max select age 89.
std::vector<double> table_47_age_89()
    {
    static int const n = 17;
    static double const q[n] =
        //  0     1     2     3     4     5     6     7     8     9
        {                                                      0.60 //  80
        ,0.60, 0.64, 0.68, 0.68, 0.72, 0.75, 0.75, 0.80, 0.80, 1.00 //  90
        ,0.00, 0.00, 0.00, 0.00, 0.00, 0.00                         // 100
        };
    return std::vector<double>(q, q + n);
    }

// SOA database 'qx_ins' table 308
// "1956 Texas Chamberlain, Male & Female, Age next"
// Parameters: min age 1; max age 105; select period 5; max select age 100.
// Return q as though max age were 10: that's enough for testing.
std::vector<double> table_308(int age)
    {
    // Select: issue age by duration.
    static int const nsel = 25;
    static double const qsel[nsel] =
        //      0         1         2         3         4
        {0.003341, 0.002463, 0.002225, 0.001852, 0.001483 // 1
        ,0.000688, 0.001970, 0.001842, 0.001574, 0.001274 // 2
        ,0.000596, 0.001722, 0.001636, 0.001395, 0.001124 // 3
        ,0.000513, 0.001520, 0.001455, 0.001239, 0.000999 // 4
        ,0.000438, 0.001339, 0.001294, 0.001116, 0.000917 // 5
        };
    // Ultimate: attained age.
    static int const nult = 5;
    static double const qult[nult] =
        //      6         7         8         9        10
        {0.001099, 0.000958, 0.000843, 0.000750, 0.000712
        };
    int isel = 5 * (age - 1);
    std::vector<double> v(qsel + isel, qsel + isel + 5);
    v.insert(v.end(), qult + age - 1, qult + nult);
    return v;
    }
} // Unnamed namespace.

void mete()
{
    std::string const qx_cso("/opt/lmi/data/qx_cso");
    std::vector<double> rates;

    rates = actuarial_table(qx_cso, 42).values( 0, 100);
    rates = actuarial_table(qx_cso, 42).values(35,  65);
    rates = actuarial_table(qx_cso, 47).values(89,  17);
    rates = actuarial_table(qx_cso, 47).values(80,  26);
    rates = actuarial_table(qx_cso, 47).values(80,  20);
    rates = actuarial_table(qx_cso, 47).values(20,  20);
}

void assay_speed()
{
    std::cout << "  Speed test: " << TimeAnAliquot(mete) << '\n';
}

void test_precondition_failures()
{
    BOOST_TEST_THROW
        (actuarial_table("nonexistent", 0)
        ,std::runtime_error
        ,"There is no table number 0 in file 'nonexistent'."
        );

    std::string const qx_cso("/opt/lmi/data/qx_cso");
    actuarial_table z(qx_cso, 47);
    BOOST_TEST_THROW
        (z.values_elaborated(3, 8, e_reenter_never, 0, 0)
        ,std::runtime_error
        ,"Table-lookup method 0 is not valid in this context."
        );
}

void test_e_reenter_never()
{
    std::string const qx_cso("/opt/lmi/data/qx_cso");
    std::vector<double> rates;

// TODO ?? Also test a 'duration' table--has SOA published any?

    rates = actuarial_table(qx_cso, 42).values( 0, 100);
    BOOST_TEST(rates == table_42(0));

    rates = actuarial_table(qx_cso, 42).values(35,  65);
    BOOST_TEST(rates == table_42(35));

    rates = actuarial_table(qx_cso, 47).values(89,  17);
    BOOST_TEST(rates == table_47_age_89());

    // Arguably, this ought to signal an error, because 90 exceeds the
    // maximum select age. However, this behavior is reasonable, and
    // the e_reenter_at_inforce_duration implementation relies on it.
    rates = actuarial_table(qx_cso, 47).values(90,  16);
    std::vector<double> gauge = table_47_age_89();
    gauge.erase(gauge.begin());
    BOOST_TEST(rates == gauge);
}

void test_e_reenter_at_inforce_duration()
{
    std::string const qx_cso("/opt/lmi/data/qx_cso");
    std::vector<double> rates;
    std::vector<double> gauge;
    int inforce_duration = 0;

    // Map status [89]+0+j to rate for [89+0]+j .
    inforce_duration = 0;
    rates = actuarial_table(qx_cso, 47).values_elaborated
        (89 - inforce_duration // issue_age
        ,17 + inforce_duration // length
        ,e_reenter_at_inforce_duration
        ,     inforce_duration // full_years_since_issue
        ,0                     // full_years_since_last_rate_reset
        );
    gauge = table_47_age_89();
    gauge.insert(gauge.begin(), inforce_duration, 0.0);
    BOOST_TEST(rates == gauge);

    // Map status [78]+11+j to rate for [78+11]+j .
    inforce_duration = 11;
    rates = actuarial_table(qx_cso, 47).values_elaborated
        (89 - inforce_duration // issue_age
        ,17 + inforce_duration // length
        ,e_reenter_at_inforce_duration
        ,     inforce_duration // full_years_since_issue
        ,0                     // full_years_since_last_rate_reset
        );
    gauge = table_47_age_89();
    gauge.insert(gauge.begin(), inforce_duration, 0.0);
    BOOST_TEST(rates == gauge);

    // Map status [89]+1+j to rate for [89]+1+j . There is no rate for
    // [89+1]+j because the maximum select age is 89.
    rates = actuarial_table(qx_cso, 47).values_elaborated
        (89 // issue_age
        ,16 // length
        ,e_reenter_at_inforce_duration
        ,1  // full_years_since_issue
        ,0  // full_years_since_last_rate_reset
        );
    gauge = table_47_age_89();
    gauge[0] = 0.0;
    gauge.resize(16);
    BOOST_TEST(rates == gauge);
}

/// Test with SOA table 308 in 'qx_ins', chosen because it's a select
/// and ultimate table with a minimum age distinct from zero.

void test_e_reenter_upon_rate_reset()
{
    std::string const qx_ins("/opt/lmi/data/qx_ins");
    std::vector<double> rates;
    std::vector<double> gauge0;
    std::vector<double> gauge1;

    rates = actuarial_table(qx_ins, 308).values_elaborated
        (3      // issue_age
        ,8      // length
        ,e_reenter_upon_rate_reset
        ,0      // full_years_since_issue
        ,0      // full_years_since_last_rate_reset
        );
    gauge0 = table_308(3);
    gauge0.erase(gauge0.begin(), 0 + gauge0.begin());
    BOOST_TEST(rates == gauge0);
    gauge1 = actuarial_table(qx_ins, 308).values(3, 8);
    gauge1.erase(gauge1.begin(), 0 + gauge1.begin());
    BOOST_TEST(rates == gauge1);

    rates = actuarial_table(qx_ins, 308).values_elaborated
        (3      // issue_age
        ,8      // length
        ,e_reenter_upon_rate_reset
        ,0      // full_years_since_issue
        ,1      // full_years_since_last_rate_reset
        );
    gauge0 = table_308(2);
    gauge0.erase(gauge0.begin(), 1 + gauge0.begin());
    BOOST_TEST(rates == gauge0);
    gauge1 = actuarial_table(qx_ins, 308).values(2, 9);
    gauge1.erase(gauge1.begin(), 1 + gauge1.begin());
    BOOST_TEST(rates == gauge1);

    rates = actuarial_table(qx_ins, 308).values_elaborated
        (3      // issue_age
        ,8      // length
        ,e_reenter_upon_rate_reset
        ,0      // full_years_since_issue
        ,2      // full_years_since_last_rate_reset
        );
    gauge0 = table_308(1);
    gauge0.erase(gauge0.begin(), 2 + gauge0.begin());
    BOOST_TEST(rates == gauge0);
    gauge1 = actuarial_table(qx_ins, 308).values(1, 10);
    gauge1.erase(gauge1.begin(), 2 + gauge1.begin());
    BOOST_TEST(rates == gauge1);

    rates = actuarial_table(qx_ins, 308).values_elaborated
        (3      // issue_age
        ,8      // length
        ,e_reenter_upon_rate_reset
        ,0      // full_years_since_issue
        ,3      // full_years_since_last_rate_reset
        );
    BOOST_TEST(rates == gauge0);
    BOOST_TEST(rates == gauge1);

    rates = actuarial_table(qx_ins, 308).values_elaborated
        (3      // issue_age
        ,8      // length
        ,e_reenter_upon_rate_reset
        ,0      // full_years_since_issue
        ,999    // full_years_since_last_rate_reset
        );
    BOOST_TEST(rates == gauge0);
    BOOST_TEST(rates == gauge1);
}

int test_main(int, char*[])
{
    test_precondition_failures();
    test_e_reenter_never();
    test_e_reenter_at_inforce_duration();
    test_e_reenter_upon_rate_reset();

    assay_speed();

    return EXIT_SUCCESS;
}

