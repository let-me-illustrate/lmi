// Financial functions.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef financial_hpp
#define financial_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "mc_enum_type_enums.hpp"       // mcenum_mode
#include "miscellany.hpp"               // ios_out_app_binary()
#include "zero.hpp"

#include <fstream>
#include <iterator>                     // iterator_traits

class calendar_date;

// TODO ?? Things to reconsider later:
//
// v*v*v...*v != v^n because of floating-point roundoff.
// Find i to n decimals, not v to n decimals.
// Separate implementations: in advance vs. in arrears.
//
// The a priori irr bounds ought to be parameterized.
//
// When no root is bracketed, -100% is always conservative enough;
// but if a root is known to exceed the a priori upper bound, then
// perhaps that upper bound could be returned instead.
//
// Is it really advantageous to use long double? Why not use a
// template type argument instead?

template<typename InputIterator>
long double fv
    (InputIterator first
    ,InputIterator last
    ,long double i
    )
{
    if(first == last)
        {
        return 0.0L;
        }

    // Symbol v, meaning 1/(1+i), is standard. A corresponding
    // symbol u, meaning (1+i), is not standard, but should be.
    long double const u = 1.0L + i;
    long double z = 0.0L;
    for(InputIterator j = first; j != last; ++j)
        {
        z += *j;
        z *= u;
        }
    return z;
}

template<typename InputIterator>
class irr_helper
{
  public:
    irr_helper
        (InputIterator first
        ,InputIterator last
        ,long double   x
        ,int           decimals
        )
        :first_    {first}
        ,last_     {last}
        ,x_        {x}
        ,decimals_ {decimals}
        {}

    long double operator()(long double i)
        {
        return fv(first_, last_, i) - x_;
        }

    long double operator()()
        {
// Uncomment to trace iterations:
//      std::ofstream ofs_trace;
//      ofs_trace.open("trace_irr.txt", ios_out_app_binary());
//      std::ostream os_trace(ofs_trace.rdbuf());
        root_type const z = decimal_root
            (*this
            ,-1.0       // A priori lower bound.
            ,1000.0     // Assumed upper bound.
            ,bias_lower // Return the final bound with the lower FV.
            ,decimals_
            ,64
//          ,os_trace
            );
        switch(z.validity)
            {
            case root_is_valid:
                {return z.root;}
            // Return -100% if NPVs of a priori bounds have same sign.
            case root_not_bracketed:
                {return -1.0L;}
            case improper_bounds:
                {throw "IRR: improper bounds.";}
            }
        throw "Unreachable--silences a compiler diagnostic.";
        }

  private:
    InputIterator first_;
    InputIterator last_;
    long double   x_;
    int           decimals_;
};

template<typename InputIterator>
long double irr
    (InputIterator first
    ,InputIterator last
    ,long double x
    ,int decimals
    )
{
    return irr_helper<InputIterator>(first, last, x, decimals)();
}

template
    <typename InputIterator0
    ,typename InputIterator1
    ,typename OutputIterator
    >
OutputIterator const irr
    (InputIterator0 first0
    ,InputIterator0 last0
    ,InputIterator1 first1
    ,OutputIterator result
    ,int            decimals
    )
{
    // Copy arguments whose values we need to modify so that const
    // arguments are valid.
    //
    // The point where 'pmts' is incremented requires explanation
    // because 'pmts' serves two purposes. It controls the number
    // of iterations. In the loop body, it marks one past the end
    // of the interval we want to pass to the subfunction. We could
    // use an auxiliary variable in the loop body instead:
    //   pmts_end = pmts;
    //   ++pmts_end;
    // and then increment 'pmts' in a place that seems more normal,
    // but timing tests show that to be significantly slower with
    // some of the compilers we use, and we have demonstrated that
    // IRR calculations take enough run time to be inconvenient to
    // users already.

    InputIterator0 pmts = first0;
    InputIterator1 bfts = first1;
    for(;pmts != last0; ++bfts, ++result)
        {
        auto z = irr_helper<InputIterator0>(first0, ++pmts, *bfts, decimals)();
        typedef typename std::iterator_traits<OutputIterator>::value_type T;
        *result = bourn_cast<T>(z);
        }
    return result;
}

// Specialized IRR for life insurance, reflecting lapse year: less
// general, but handier and safer. Calculate the IRR of premiums
// versus benefits through the lapse duration only. The input
// containers may extend past the lapse duration or may be of
// different sizes, so the total duration through which IRRs are
// wanted is supplied in an argument. IRR is defined to be -100% in
// the interval [lapse duration, total duration). Before performing
// any IRR calculation, assert that the premium and benefit containers
// have size() of at least the lapse duration, and that lapse duration
// is at least total duration.
//
// Alternatively, lapse duration might be determined from context.
// It is not sufficient to ignore it: even if the benefit is zero, a
// payment stream that contains at least one positive and at least one
// negative may result in an IRR other than -100%. Neither is it
// adequate to infer that lapse occurs immediately after the last
// nonzero element in the container of benefits, since benefits need
// not be death benefit, but may be cash values for example. For an
// IRR of account values versus payments, account value may go to zero
// while cash surrender values remain positive due to a secondary cash
// value guarantee. For an IRR of cash values versus payments, cash
// value may go to zero while death benefits remain positive due to a
// no-lapse guarantee. We could require death benefits to be passed as
// an extra parameter, and infer lapse duration from them, hoping to
// have ignored no further such subtlety, but that is less simple than
// passing the lapse duration itself--the datum we really need.

template
    <typename InputContainer0
    ,typename InputContainer1
    ,typename OutputContainer
    >
void irr
    (InputContainer0 const& pmts
    ,InputContainer1 const& bfts
    ,OutputContainer& results
    ,typename OutputContainer::size_type lapse_duration
    ,typename OutputContainer::size_type total_duration
    ,int decimals
    )
{
    LMI_ASSERT(lapse_duration <= pmts.size());
    LMI_ASSERT(lapse_duration <= bfts.size());
    LMI_ASSERT(lapse_duration <= total_duration);
    results.clear();
    results.resize(total_duration, -1.0);
    irr
        (pmts.begin()
        ,pmts.begin() + lapse_duration
        ,bfts.begin()
        ,results.begin()
        ,decimals
        );
}

double list_bill_premium
    (double               prem_ante
    ,double               prem_post
    ,mcenum_mode          mode
    ,calendar_date const& cert_date
    ,calendar_date const& bill_date
    ,double               v12
    );

#endif // financial_hpp
