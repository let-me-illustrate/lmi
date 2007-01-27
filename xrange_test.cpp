// Range-checked type--unit test.
//
// Copyright (C) 2001, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: xrange_test.cpp,v 1.6 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xrange.hpp"
#include "xrangetypes.hpp"

#include "test_tools.hpp"

#include <ios>
#include <iostream>
#include <sstream>

// Sample range types for unit test.

struct range_dbl_0_1000
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 123.0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0.0, 1000.0);
    }
};
typedef xrange<range_dbl_0_1000, double> r_dbl_0_1000;

/*
// TODO ?? Alternatively we could just specialize the functions
// we need instead of forwarding, e.g.
template<> std::pair<double,double>
xrange<range_dbl_0_1000,double>::range_limits() const
{
//    return std::pair<double,double>(0.0, 1000.0);
    return range_dbl_0_1000::range_limits();
}
*/

struct range_int_0_1200
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 456;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 1200);
    }
};
typedef xrange<range_int_0_1200, int> r_int_0_1200;

struct range_int_100pct
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 100);
    }
};
typedef xrange<range_int_100pct, int> r_int_100pct;

struct range_dbl_abs_val_le_1
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0.0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(-1.0, 1.0);
    }
};
typedef xrange<range_dbl_abs_val_le_1, double> r_dbl_abs_val_le_1;

class Inputs
{
public:
    Inputs()
        :pct_default (100)
        ,pct_0       (  0)
        ,pct_1       (  0)
        ,pct_2       (  0)
        {}
    Inputs
        (int a_default
        ,int a_0
        ,int a_1
        ,int a_2
        )
        :pct_default (a_default)
        ,pct_0       (a_0)
        ,pct_1       (a_1)
        ,pct_2       (a_2)
        {}

    int pct_allocated()
        {return pct_0.value() + pct_1.value() + pct_2.value();}
    // TODO ?? This function can be moved into struct range_int_100pct when
    // we ultimately give it a pointer to the struct Inputs that contains it.
    int max_pct(r_int_100pct self)
        {return self.value() + 100 - pct_allocated();}

    r_int_100pct pct_default;
    r_int_100pct pct_0;
    r_int_100pct pct_1;
    r_int_100pct pct_2;
};

int test_main(int, char*[])
{
    r_dbl_0_1000 f;
    BOOST_TEST( 123.0 == f.value());
    r_dbl_0_1000::limits_type lf = f.range_limits();
    BOOST_TEST(   0.0 == lf.first);
    BOOST_TEST(1000.0 == lf.second);

    BOOST_TEST(!f.is_valid(   -.7));
    BOOST_TEST( f.is_valid(   0.0));
    BOOST_TEST( f.is_valid( 500.0));
    BOOST_TEST( f.is_valid(1000.0));
    BOOST_TEST(!f.is_valid(1000.7));

    BOOST_TEST( f.is_valid("500.0"));

    r_int_0_1200 m;
    BOOST_TEST( 456 == m.value());
    r_int_0_1200::limits_type lm = m.range_limits();
    BOOST_TEST(   0 == lm.first);
    BOOST_TEST(1200 == lm.second);

    m = 11;

#ifdef RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED
    int return_value = -1;
    try
        {
        // This should throw:
        m = 1e6;
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        return_value = 0;
        }

    try
        {
        // This should throw:
        m = "-1";
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        return_value = 0;
        }
#else // not RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED
    int return_value = 0;
#endif // not RESUMPTION_SEMANTICS_SUPPRESSED_AND_TERMINATION_SEMANTICS_RESTORED

    r_int_0_1200 m_result;
    std::stringstream ss;
    ss << m;
    ss >> m_result;
    BOOST_TEST(11 == m_result.value());

    Inputs i(10, 20, 30, 40);
    BOOST_TEST(30 == i.max_pct(i.pct_0));

    // Make sure operator<() works.
    BOOST_TEST(r_dbl_abs_val_le_1(-0.2) < r_dbl_abs_val_le_1(-0.1));
    BOOST_TEST(r_dbl_abs_val_le_1(-0.1) < r_dbl_abs_val_le_1( 0.0));
    BOOST_TEST(r_dbl_abs_val_le_1( 0.0) < r_dbl_abs_val_le_1( 0.1));
    BOOST_TEST(r_dbl_abs_val_le_1( 0.1) < r_dbl_abs_val_le_1( 0.2));

    BOOST_TEST(!(r_dbl_abs_val_le_1(-0.1) < r_dbl_abs_val_le_1(-0.1)));
    BOOST_TEST(!(r_dbl_abs_val_le_1(-0.0) < r_dbl_abs_val_le_1(-0.0)));
    BOOST_TEST(!(r_dbl_abs_val_le_1(-0.0) < r_dbl_abs_val_le_1( 0.0)));
    BOOST_TEST(!(r_dbl_abs_val_le_1( 0.0) < r_dbl_abs_val_le_1( 0.0)));
    BOOST_TEST(!(r_dbl_abs_val_le_1( 0.1) < r_dbl_abs_val_le_1( 0.1)));

    // Test calendar-date ranges: this is the only non-POD type we
    // use with the 'xrange' template, at least as of 2003-11.
    r_date const date0(calendar_date(2001, 1, 1));
    r_date const date1(calendar_date(2002, 2, 2));
    BOOST_TEST(!(date0 == date1));
    BOOST_TEST(  date0 != date1 );
    BOOST_TEST(  date0 <  date1 );
    BOOST_TEST(  date0 <= date1 );
    BOOST_TEST(  date1 >  date0 );
    BOOST_TEST(  date1 >= date0 );

    return return_value;
}

