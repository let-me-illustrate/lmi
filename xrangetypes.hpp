// Range-checked types.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: xrangetypes.hpp,v 1.4 2007-01-27 00:00:52 wboutin Exp $

#ifndef xrangetypes_hpp
#define xrangetypes_hpp

#include "config.hpp"

#include "xrange.hpp"

#include "calendar_date.hpp"

#include <limits>
#include <utility>

// "Abstract" range types.

// TODO ?? Limit to endowment duration.
struct range_duration
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
typedef xrange<range_duration, int> r_duration;

// TODO ?? Limit to issue age, maturity duration.
struct range_attained_age
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
typedef xrange<range_attained_age, int> r_attained_age;

// "Concrete" range types.

// In general, we use enumerative type e_yes_or_no in preference to
// this, because it lets us use keywords 'Yes' and 'No' in the GUI,
// and that's more readily understood by users than 1 and 0.
struct range_bool
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 1);
    }
};
typedef xrange<range_bool, int> r_bool;

struct range_iss_age
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 45;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 99);
    }
};
typedef xrange<range_iss_age, int> r_iss_age;

struct range_ret_age
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 65;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 100);
    }
};
typedef xrange<range_ret_age, int> r_ret_age;

struct range_inforce_month
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 11);
    }
};
typedef xrange<range_inforce_month, int> r_inforce_month;

typedef xrange<range_duration, int> r_inforce_year;

// TODO ?? For now, we don't much worry about valid ranges for
// contract month and year. We should refine this someday: for
// instance, contract duration mustn't exceed policy duration.
struct range_contract_month
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 11);
    }
};
typedef xrange<range_contract_month, int> r_contract_month;

struct range_contract_year
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>(0, 6);
    }
};
typedef xrange<range_duration, int> r_contract_year;

struct range_solve_beg_year
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
typedef xrange<range_solve_beg_year, int> r_solve_beg_year;

struct range_solve_end_year
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
typedef xrange<range_solve_end_year, int> r_solve_end_year;

struct range_solve_tgt_year
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
typedef xrange<range_solve_tgt_year, int> r_solve_tgt_year;

struct range_solve_beg_time
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
typedef xrange<range_solve_beg_time, int> r_solve_beg_time;

struct range_solve_end_time
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
typedef xrange<range_solve_end_time, int> r_solve_end_time;

struct range_solve_tgt_time
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
typedef xrange<range_solve_tgt_time, int> r_solve_tgt_time;

struct range_solve_tgt_csv
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_solve_tgt_csv, double> r_solve_tgt_csv;

struct range_loan
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_loan, double> r_loan;

struct range_wd
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_wd, double> r_wd;

struct range_spec_amt
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
// TODO ?? Undo this workaround and find a good way to allow the sum
// of term and base specamt to be reduced in the GUI.
//            (0
            (-std::numeric_limits<EssentialType>::max()
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_spec_amt, double> r_spec_amt;

struct range_pmt
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_pmt, double> r_pmt;

struct range_dumpin
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_dumpin, double> r_dumpin;

struct range_dumpin1035
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_dumpin1035, double> r_dumpin1035;

struct range_basis1035
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_basis1035, double> r_basis1035;

struct range_curr_int_rate
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (-1.0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_curr_int_rate, double> r_curr_int_rate;

struct range_loan_int_rate
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_loan_int_rate, double> r_loan_int_rate;

struct range_fund
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
typedef xrange<range_fund, int> r_fund;

// TODO ?? Use a particular type everywhere these catchall types appear;
// or typedef particular types in terms of these and don't use them in
// any other way:

struct range_unchecked_double
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        // For floating-point types, min() isn't the most negative value.
        return std::pair<EssentialType,EssentialType>
            (-std::numeric_limits<EssentialType>::max()
            , std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_unchecked_double, double> r_unchecked_double;

struct range_nonnegative_double
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_nonnegative_double, double> r_nonnegative_double;

struct range_nonnegative_int
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_nonnegative_int, int> r_nonnegative_int;

struct range_positive_int
{
    typedef int EssentialType;
    static EssentialType default_value()
    {
        return 1;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (1
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_positive_int, int> r_positive_int;

struct range_double_ge_one
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 1;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (1
            ,std::numeric_limits<EssentialType>::max()
            );
    }
};
typedef xrange<range_double_ge_one, double> r_double_ge_one;

struct range_proportion
{
    typedef double EssentialType;
    static EssentialType default_value()
    {
        return 0;
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (0
            ,1
            );
    }
};
typedef xrange<range_proportion, double> r_proportion;

struct range_date
{
    typedef calendar_date EssentialType;
    static EssentialType default_value()
    {
        return calendar_date();
    }
    static std::pair<EssentialType,EssentialType> range_limits()
    {
        return std::pair<EssentialType,EssentialType>
            (calendar_date(gregorian_epoch())
            ,calendar_date(9999, 12, 31)
            );
    }
};
typedef xrange<range_date, calendar_date> r_date;

#endif // xrangetypes_hpp

