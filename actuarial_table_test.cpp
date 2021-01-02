// Actuarial tables from SOA database--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "actuarial_table.hpp"

#include "assert_lmi.hpp"
#include "cso_table.hpp"
#include "miscellany.hpp"
#include "oecumenic_enumerations.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <cstdio>                       // remove()
#include <fstream>

namespace
{
/// SOA regulatory table database.

std::string const qx_cso("/opt/lmi/data/qx_cso");

/// SOA insurance table database.

std::string const qx_ins("/opt/lmi/data/qx_ins");

/// 'qx_ins' table 750: "1924 US Linton A Lapse"
///
/// Table type: Duration.
///
/// Parameters:
///   min "age" 1; max "age" 15

std::vector<double> table_750()
{
    static int const n = 15;
    static double const q[n] =
        //   0      1      2      3      4      5      6      7      8      9
        {0.100, 0.060, 0.050, 0.044, 0.040, 0.036, 0.032, 0.029, 0.027, 0.025 // 00
        ,0.024, 0.023, 0.022, 0.021, 0.020                                    // 10
        };
    return std::vector<double>(q, q + n);
}

/// 1980 CSO Ultimate ANB Male Unismoke (i.e., 'qx_cso' table 42)

std::vector<double> table_42(int age)
{
    static std::vector<double> const q = cso_table
        (mce_1980cso
        ,oe_orthodox
        ,oe_age_nearest_birthday_ties_younger
        ,mce_male
        ,mce_unismoke
        );
    LMI_ASSERT(0 <= age && age < lmi::ssize(q));
    return std::vector<double>(q.begin() + age, q.end());
}

/// 'qx_ins' table 256: "1934 UK A1924-29, Male+Female, Age nearest"
///
/// Table type: Select.
///
/// Parameters:
///   min age 10; max age 121; select period 3; max select age 80
/// This is a good table for testing because none of its parameters is
/// degenerate: minimum age is not zero, and maximum age differs from
/// maximum select age.

std::vector<double> table_256(int age, int duration)
{
    LMI_ASSERT(0 <= duration && duration <= 3);
    // Select: issue age by duration.
    static int const nsel = 71 * 3;
    static double const qsel[nsel] =
        //     1        2        3
        {0.00106 ,0.00140 ,0.00165 // 10
        ,0.00113 ,0.00148 ,0.00175 // 11
        ,0.00120 ,0.00157 ,0.00184 // 12
        ,0.00127 ,0.00165 ,0.00193 // 13
        ,0.00134 ,0.00174 ,0.00202 // 14
        ,0.00141 ,0.00182 ,0.00210 // 15
        ,0.00147 ,0.00189 ,0.00216 // 16
        ,0.00153 ,0.00195 ,0.00219 // 17
        ,0.00158 ,0.00197 ,0.00220 // 18
        ,0.00160 ,0.00198 ,0.00220 // 19
        ,0.00160 ,0.00198 ,0.00220 // 20
        ,0.00160 ,0.00198 ,0.00220 // 21
        ,0.00160 ,0.00198 ,0.00220 // 22
        ,0.00160 ,0.00198 ,0.00220 // 23
        ,0.00160 ,0.00198 ,0.00220 // 24
        ,0.00160 ,0.00198 ,0.00220 // 25
        ,0.00160 ,0.00198 ,0.00221 // 26
        ,0.00160 ,0.00199 ,0.00223 // 27
        ,0.00160 ,0.00200 ,0.00226 // 28
        ,0.00161 ,0.00202 ,0.00230 // 29
        ,0.00162 ,0.00205 ,0.00236 // 30
        ,0.00164 ,0.00210 ,0.00244 // 31
        ,0.00167 ,0.00216 ,0.00254 // 32
        ,0.00171 ,0.00224 ,0.00266 // 33
        ,0.00177 ,0.00235 ,0.00280 // 34
        ,0.00185 ,0.00248 ,0.00297 // 35
        ,0.00194 ,0.00263 ,0.00316 // 36
        ,0.00205 ,0.00279 ,0.00337 // 37
        ,0.00217 ,0.00297 ,0.00359 // 38
        ,0.00230 ,0.00316 ,0.00382 // 39
        ,0.00244 ,0.00336 ,0.00406 // 40
        ,0.00258 ,0.00356 ,0.00431 // 41
        ,0.00273 ,0.00377 ,0.00458 // 42
        ,0.00288 ,0.00400 ,0.00487 // 43
        ,0.00304 ,0.00425 ,0.00519 // 44
        ,0.00322 ,0.00453 ,0.00556 // 45
        ,0.00342 ,0.00485 ,0.00599 // 46
        ,0.00365 ,0.00522 ,0.00648 // 47
        ,0.00391 ,0.00564 ,0.00703 // 48
        ,0.00421 ,0.00611 ,0.00764 // 49
        ,0.00455 ,0.00663 ,0.00832 // 50
        ,0.00493 ,0.00721 ,0.00908 // 51
        ,0.00535 ,0.00786 ,0.00993 // 52
        ,0.00581 ,0.00859 ,0.01089 // 53
        ,0.00632 ,0.00941 ,0.01199 // 54
        ,0.00690 ,0.01035 ,0.01326 // 55
        ,0.00757 ,0.01143 ,0.01470 // 56
        ,0.00834 ,0.01265 ,0.01629 // 57
        ,0.00920 ,0.01399 ,0.01801 // 58
        ,0.01014 ,0.01544 ,0.01986 // 59
        ,0.01115 ,0.01700 ,0.02184 // 60
        ,0.01223 ,0.01867 ,0.02398 // 61
        ,0.01339 ,0.02048 ,0.02635 // 62
        ,0.01464 ,0.02247 ,0.02902 // 63
        ,0.01601 ,0.02471 ,0.03206 // 64
        ,0.01754 ,0.02726 ,0.03551 // 65
        ,0.01927 ,0.03016 ,0.03938 // 66
        ,0.02123 ,0.03341 ,0.04365 // 67
        ,0.02343 ,0.03699 ,0.04830 // 68
        ,0.02585 ,0.04087 ,0.05330 // 69
        ,0.02847 ,0.04503 ,0.05863 // 70
        ,0.03127 ,0.04947 ,0.06431 // 71
        ,0.03424 ,0.05420 ,0.07036 // 72
        ,0.03738 ,0.05923 ,0.07682 // 73
        ,0.04070 ,0.06459 ,0.08373 // 74
        ,0.04421 ,0.07031 ,0.09112 // 75
        ,0.04793 ,0.07641 ,0.09901 // 76
        ,0.05188 ,0.08291 ,0.10741 // 77
        ,0.05607 ,0.08982 ,0.11632 // 78
        ,0.06051 ,0.09714 ,0.12572 // 79
        ,0.06520 ,0.10486 ,0.13557 // 80
        };
    // Ultimate: attained age.
    static int const nult = 121 - 13 + 1;
    static double const qult[nult] =
        //     0        1        2        3        4        5        6        7        8        9
        {                           0.00186 ,0.00196 ,0.00206 ,0.00216 ,0.00225 ,0.00231 ,0.00234 // 10
        ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00236 ,0.00238 // 20
        ,0.00241 ,0.00246 ,0.00253 ,0.00262 ,0.00273 ,0.00286 ,0.00302 ,0.00320 ,0.00341 ,0.00364 // 30
        ,0.00388 ,0.00413 ,0.00439 ,0.00466 ,0.00495 ,0.00527 ,0.00563 ,0.00604 ,0.00651 ,0.00704 // 40
        ,0.00764 ,0.00831 ,0.00906 ,0.00990 ,0.01084 ,0.01190 ,0.01311 ,0.01450 ,0.01608 ,0.01783 // 50
        ,0.01973 ,0.02176 ,0.02394 ,0.02631 ,0.02893 ,0.03188 ,0.03524 ,0.03908 ,0.04338 ,0.04812 // 60
        ,0.05327 ,0.05881 ,0.06473 ,0.07104 ,0.07777 ,0.08497 ,0.09268 ,0.10093 ,0.10974 ,0.11913 // 70
        ,0.12910 ,0.13962 ,0.15066 ,0.16221 ,0.17425 ,0.18676 ,0.19973 ,0.21315 ,0.22702 ,0.24134 // 80
        ,0.25611 ,0.27133 ,0.28700 ,0.30312 ,0.31970 ,0.33675 ,0.35428 ,0.37231 ,0.39086 ,0.40995 // 90
        ,0.42960 ,0.44983 ,0.47066 ,0.49211 ,0.51420 ,0.53695 ,0.56038 ,0.58451 ,0.60936 ,0.63495 // 100
        ,0.66130 ,0.68843 ,0.71636 ,0.74511 ,0.77470 ,0.80515 ,0.83648 ,0.86871 ,0.90186 ,0.93595 // 110
        ,0.97100 ,1.00000                                                                         // 120
        };
    int isel = 3 * (age - 10);
    std::vector<double> v(qsel + isel + duration, qsel + isel + 3);
    v.insert(v.end(), qult + age - 10, qult + nult);
    return v;
}

/// See 'ChangeLog' for 20080522T1353Z and 20080523T0153Z, as well as
/// 'DefectLog' for 20080523T0407Z.

void assert_table_nondegeneracy(actuarial_table const& t)
{
    LMI_ASSERT(0 < t.min_age());
    LMI_ASSERT(t.max_age() != t.max_select_age() + t.select_period());

    int const min_age = t.min_age();
    int const max_age = t.max_age();
    int const length  = 1 + max_age - min_age;
    std::vector<double> rates = t.values(min_age, length);
    LMI_ASSERT(rates.at(       0) != rates.at(       1));
    LMI_ASSERT(rates.at(length-2) != rates.at(length-1));
}

void test_80cso_erratum
    (int              table_number
    ,oenum_autopisty  autopisty
    ,oenum_alb_or_anb alb_or_anb
    )
{
    std::vector<double> v0 = cso_table
        (mce_1980cso
        ,autopisty
        ,alb_or_anb
        ,mce_male
        ,mce_nonsmoker
        );
    actuarial_table const a(qx_cso, table_number);
    // No smoker-nonsmoker distinction below age 15.
    std::vector<double> v1 = a.values(15, 85);
    BOOST_TEST(v0 == v1);
}
} // Unnamed namespace.

void mete()
{
    std::vector<double> rates;

    rates = actuarial_table(qx_cso,  42).values( 0, 100);
    rates = actuarial_table(qx_cso,  42).values(35,  65);
    rates = actuarial_table(qx_ins, 256).values(90,  32);
    rates = actuarial_table(qx_ins, 256).values(10, 112);
}

void assay_speed()
{
    std::cout << "  Speed test: " << TimeAnAliquot(mete) << '\n';
}

/// Test general preconditions.
///
/// Table numbers must be positive.
///
/// Both '.ndx' and '.dat' files must exist.
///
/// The function for nondefault lookup methods cannot be used with the
/// default lookup method: even attempting to do so is an error.

void test_precondition_failures()
{
    BOOST_TEST_THROW
        (actuarial_table("nonexistent", 0)
        ,std::runtime_error
        ,"There is no table number 0 in file 'nonexistent'."
        );

    BOOST_TEST_THROW
        (actuarial_table("nonexistent", 1)
        ,std::runtime_error
        ,"File 'nonexistent.ndx' is required but could not be found."
         " Try reinstalling."
        );

    std::ifstream ifs((qx_cso + ".ndx").c_str(), ios_in_binary());
    std::ofstream ofs("eraseme.ndx", ios_out_trunc_binary());
    ofs << ifs.rdbuf();
    ofs.close();
    BOOST_TEST_THROW
        (actuarial_table("eraseme", 1)
        ,std::runtime_error
        ,"File 'eraseme.dat' is required but could not be found."
         " Try reinstalling."
        );
    BOOST_TEST(0 == std::remove("eraseme.ndx"));

    actuarial_table z(qx_ins, 256);
    BOOST_TEST_THROW
        (z.values_elaborated(80, 42, e_reenter_never, 0, 0)
        ,std::runtime_error
        ,"Cannot use 'e_reenter_never' here."
        );
}

/// Test preconditions for actuarial_table::specific_values().
///
/// It is sufficient to test only one table type, because the same
/// preconditions apply to all types.

void test_lookup_errors()
{
    // Aggregate table:
    //   0 minimum age
    //  99 maximum age

    BOOST_TEST_THROW
        (actuarial_table(qx_cso, 42).values(  0,  -1)
        ,std::runtime_error
        ,"Assertion '0 <= length && length <= 1 + max_age_ - issue_age' failed."
        );

    BOOST_TEST_THROW
        (actuarial_table(qx_cso, 42).values(  0, 101)
        ,std::runtime_error
        ,"Assertion '0 <= length && length <= 1 + max_age_ - issue_age' failed."
        );

    BOOST_TEST_THROW
        (actuarial_table(qx_cso, 42).values(  1, 100)
        ,std::runtime_error
        ,"Assertion '0 <= length && length <= 1 + max_age_ - issue_age' failed."
        );

    BOOST_TEST_THROW
        (actuarial_table(qx_cso, 42).values( -1,  10)
        ,std::runtime_error
        ,"Assertion 'min_age_ <= issue_age && issue_age <= max_age_' failed."
        );

    BOOST_TEST_THROW
        (actuarial_table(qx_cso, 42).values(100,   1)
        ,std::runtime_error
        ,"Assertion 'min_age_ <= issue_age && issue_age <= max_age_' failed."
        );
}

void test_e_reenter_never()
{
    std::vector<double> rates;
    std::vector<double> gauge;

    rates = actuarial_table(qx_ins, 750).values(1, 15);
    BOOST_TEST(rates == table_750());

    rates = actuarial_table(qx_cso, 42).values( 0, 100);
    BOOST_TEST(rates == table_42(0));

    rates = actuarial_table(qx_cso, 42).values(35,  65);
    BOOST_TEST(rates == table_42(35));

    rates = actuarial_table(qx_ins, 256).values(10, 112);
    gauge = table_256(10, 0);
    BOOST_TEST(rates == gauge);

    rates = actuarial_table(qx_ins, 256).values(80,  42);
    gauge = table_256(80, 0);
    BOOST_TEST(rates == gauge);

    // Issue age exceeds maximum select age. Arguably, this ought to
    // signal an error; however, this behavior is reasonable, and the
    // 'e_reenter_at_inforce_duration' implementation relies on it.
    rates = actuarial_table(qx_ins, 256).values(81,  41);
    BOOST_TEST_EQUAL(rates[0], 0.10486); // [81]+0 --> [80]+1
    BOOST_TEST_EQUAL(rates[1], 0.13557); // [81]+1 --> [80]+2
    BOOST_TEST_EQUAL(rates[2], 0.16221); // [81]+2 --> 83 ultimate

    gauge = table_256(80, 0);
    gauge.erase(gauge.begin(), 1 + gauge.begin());
    BOOST_TEST(rates == gauge);

    gauge = table_256(80, 1);
    BOOST_TEST(rates == gauge);

    rates = actuarial_table(qx_ins, 256).values(82,  40);
    BOOST_TEST_EQUAL(rates[0], 0.13557); // [82]+0 --> [80]+2
    BOOST_TEST_EQUAL(rates[1], 0.16221); // [82]+1 --> 83 ultimate
    gauge = table_256(80, 2);
    BOOST_TEST(rates == gauge);

    rates = actuarial_table(qx_ins, 256).values(83,  39);
    BOOST_TEST_EQUAL(rates[0], 0.16221); // [83]+0 --> 83 ultimate
    BOOST_TEST_EQUAL(rates[1], 0.17425); // [83]+1 --> 84 ultimate
    gauge = table_256(80, 3);
    BOOST_TEST(rates == gauge);

    rates = actuarial_table(qx_ins, 256).values(84,  38);
    BOOST_TEST_EQUAL(rates[0], 0.17425); // [84]+0 --> 84 ultimate
    gauge = table_256(80, 0);
    gauge.erase(gauge.begin(), 4 + gauge.begin());
    BOOST_TEST(rates == gauge);
}

void test_e_reenter_at_inforce_duration()
{
    std::vector<double> rates;
    std::vector<double> gauge;

    e_actuarial_table_method const m = e_reenter_at_inforce_duration;

    actuarial_table const table(qx_ins, 256);
    assert_table_nondegeneracy(table);

    int const min_age     = table.min_age();
    int const max_age     = table.max_age();
    int const max_sel_age = table.max_select_age();

    // Ignored for 'e_reenter_at_inforce_duration'.
    int const reset_dur = 0;

    int pol_dur = 0;
    int iss_age = 0;
    int length  = 0;

    for(int i = max_sel_age - min_age; 0 <= i; --i)
        {
        pol_dur = i;
        iss_age = max_sel_age - pol_dur;
        length  = 1 + max_age - iss_age;
        rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
        gauge = table_256(iss_age + pol_dur, 0);
        gauge.insert(gauge.begin(), pol_dur, 0.0);
        BOOST_TEST(rates == gauge);
        }

    // Once age has been set ahead to maximum, can't push it farther.
    pol_dur = 1;
    iss_age = max_sel_age;
    length  = 1 + max_age - iss_age;
    rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
    gauge = table_256(iss_age, 0);
    gauge[0] = 0.0;
    BOOST_TEST(rates == gauge);

    BOOST_TEST_THROW
        (table.values_elaborated(min_age - 1, 1, m, 0, 0)
        ,std::runtime_error
        ,"Assertion 'min_age_ <= issue_age && issue_age <= max_age_' failed."
        );

    BOOST_TEST_THROW
        (table.values_elaborated(min_age, 1, m, -1, 0)
        ,std::runtime_error
        ,"Assertion '0 <= inforce_duration' failed."
        );

    BOOST_TEST_THROW
        (table.values_elaborated(min_age, 1, m, 999, 0)
        ,std::runtime_error
        ,"Assertion 'inforce_duration < 1 + max_age_ - issue_age' failed."
        );

    BOOST_TEST_THROW
        (table.values_elaborated(min_age - 999, 1000, m, 999, 0)
        ,std::runtime_error
        ,"Assertion 'min_age_ <= issue_age && issue_age <= max_age_' failed."
        );
}

void test_e_reenter_upon_rate_reset()
{
    std::vector<double> rates;
    std::vector<double> gauge0;
    std::vector<double> gauge1;

    e_actuarial_table_method const m = e_reenter_upon_rate_reset;

    actuarial_table const table(qx_ins, 256);
    assert_table_nondegeneracy(table);

    int const select_period = table.select_period();
    int const min_age = table.min_age();
    int const max_age = table.max_age();
    int const iss_age = 2 + min_age;
    int const length  = 1 + max_age - iss_age;

    // Ignored for 'e_reenter_upon_rate_reset' except as it limits
    // reset duration.
    int pol_dur = max_age - iss_age;

    int reset_dur = 0;

    // Negative reset duration (reset date precedes contract date):
    // age is in effect set back.

    for(int i = 0; i < select_period; ++i)
        {
        reset_dur = -i;
        int effective_age = iss_age + reset_dur;
        rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
        gauge0 = table_256(effective_age, 0);
        gauge0.erase(gauge0.begin(), gauge0.begin() - reset_dur);
        BOOST_TEST(rates == gauge0);
        gauge1 = table.values(effective_age, 1 + max_age - effective_age);
        gauge1.erase(gauge1.begin(), gauge1.begin() - reset_dur);
        BOOST_TEST(rates == gauge1);
        }

    // Once age has been set back to minimum, can't push it farther.
    BOOST_TEST(iss_age - select_period < min_age);
    reset_dur = -select_period;
    rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
    BOOST_TEST(rates == gauge0);
    BOOST_TEST(rates == gauge1);

    // A group's reset date can precede a new entrant's birthdate, so
    // the age-setback argument as such isn't limited. The algorithm
    // limits it internally, constraining the effective age to the
    // table's minimum age.
    reset_dur = -999;
    rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
    BOOST_TEST(rates == gauge0);
    BOOST_TEST(rates == gauge1);

    // Positive reset duration (reset date follows contract date):
    // age is in effect set forward.

    for(int i = 0; i <= 1 + select_period; ++i)
        {
        reset_dur = i;
        int effective_age = iss_age + reset_dur;
        rates = table.values_elaborated(iss_age, length, m, pol_dur, reset_dur);
        gauge0 = table_256(effective_age, 0);
        gauge0.insert(gauge0.begin(), reset_dur, 0.0);
        BOOST_TEST(rates == gauge0);
        gauge1 = table.values(effective_age, 1 + max_age - effective_age);
        gauge1.insert(gauge1.begin(), reset_dur, 0.0);
        BOOST_TEST(rates == gauge1);
        }

    // 'e_reenter_upon_rate_reset' and 'e_reenter_at_inforce_duration'
    // become roughly equivalent when both set age forward by the same
    // distance. They aren't quite the same in that rates for
    //   issue age + t, t < current (zero-based) policy duration
    // are indeterminate and need not compare equal.
    pol_dur   = 1;
    reset_dur = pol_dur;
    std::vector<double> rates0 = table.values_elaborated
        (iss_age
        ,length
        ,e_reenter_upon_rate_reset
        ,pol_dur
        ,reset_dur
        );
    rates0.erase(rates0.begin(), rates0.begin() + reset_dur);
    std::vector<double> rates1 = table.values_elaborated
        (iss_age
        ,length
        ,e_reenter_at_inforce_duration
        ,pol_dur
        ,reset_dur
        );
    rates1.erase(rates1.begin(), rates1.begin() + pol_dur);
    BOOST_TEST(rates0 == rates1);

    BOOST_TEST_THROW
        (table.values_elaborated(min_age, 1, m, 0, 999)
        ,std::runtime_error
        ,"Assertion 'reset_duration <= inforce_duration' failed."
        );

    BOOST_TEST_THROW
        (table.values_elaborated(min_age - 1, 1, m, 0, 0)
        ,std::runtime_error
        ,"Assertion 'min_age_ <= issue_age && issue_age <= max_age_' failed."
        );

    // A group's reset date can precede a new entrant's birthdate,
    // so an apparently-excessive negative reset-duration argument
    // must be accepted.
    rates = table.values_elaborated(min_age, 1 + max_age - min_age, m, 0, -999);
    gauge0 = table_256(min_age, 0);
    BOOST_TEST(rates == gauge0);
}

/// The e_actuarial_table_method variants are designed for use with
/// select-and-ultimate tables. However, they must work with attained-
/// age tables as well (for which they should be irrelevant).

void test_exotic_lookup_methods_with_attained_age_table()
{
    actuarial_table const table42(qx_cso, 42);
    std::vector<double> rates;

    rates = table42.values(0, 100);
    BOOST_TEST(rates == table_42(0));

    rates = table42.values_elaborated
        (0
        ,100
        ,e_reenter_at_inforce_duration
        ,99   // inforce_duration
        ,0    // reset_duration
        );
    BOOST_TEST(rates == table_42(0));

    rates = table42.values_elaborated
        (0
        ,100
        ,e_reenter_upon_rate_reset
        ,0    // inforce_duration
        ,-999 // reset_duration
        );
    BOOST_TEST(rates == table_42(0));

    rates = table42.values_elaborated
        (0
        ,100
        ,e_reenter_upon_rate_reset
        ,99   // inforce_duration
        ,99   // reset_duration
        );
    BOOST_TEST(rates == table_42(0));

    BOOST_TEST_THROW
        (table42.values_elaborated
            (0
            ,100
            ,e_reenter_upon_rate_reset
            ,0    // inforce_duration
            ,99   // reset_duration
            )
        ,std::runtime_error
        ,"Assertion 'reset_duration <= inforce_duration' failed."
        );
}

void test_1980cso_errata()
{
    test_80cso_erratum(43, oe_heterodox, oe_age_last_birthday);
    test_80cso_erratum(57, oe_orthodox , oe_age_last_birthday);
    test_80cso_erratum(44, oe_heterodox, oe_age_nearest_birthday_ties_younger);
    test_80cso_erratum(58, oe_orthodox , oe_age_nearest_birthday_ties_younger);
    test_80cso_erratum(44, oe_heterodox, oe_age_nearest_birthday_ties_older);
    test_80cso_erratum(58, oe_orthodox , oe_age_nearest_birthday_ties_older);
}

int test_main(int, char*[])
{
    test_precondition_failures();
    test_lookup_errors();
    test_e_reenter_never();
    test_e_reenter_at_inforce_duration();
    test_e_reenter_upon_rate_reset();
    test_exotic_lookup_methods_with_attained_age_table();
    test_1980cso_errata();

    assay_speed();

    return EXIT_SUCCESS;
}
