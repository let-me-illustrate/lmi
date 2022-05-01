// Determine specamt from GLP or GSP.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "gpt_specamt.hpp"

#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "safely_dereference_as.hpp"
#include "zero.hpp"                     // decimal_root()

currency gpt_specamt::CalculateGLPSpecAmt
    (BasicValues const& a_Values
    ,int                a_Duration
    ,double             a_Premium
    ,mcenum_dbopt_7702  a_DBOpt
    )
{
    Irc7702 const& z(safely_dereference_as<Irc7702>(a_Values.Irc7702_.get()));
    return CalculateSpecAmt
        (a_Values
        ,Irc7702::Get4PctBasis(a_DBOpt)
        ,a_Duration
        ,a_Premium
        ,z.PvNpfLvlTgt[Irc7702::Get4PctBasis(a_DBOpt)][a_Duration]
        ,z.PvNpfLvlExc[Irc7702::Get4PctBasis(a_DBOpt)][a_Duration]
        );
}

currency gpt_specamt::CalculateGSPSpecAmt
    (BasicValues const& a_Values
    ,int                a_Duration
    ,double             a_Premium
    )
{
    Irc7702 const& z(safely_dereference_as<Irc7702>(a_Values.Irc7702_.get()));
    return CalculateSpecAmt
        (a_Values
        ,Irc7702::Opt1Int6Pct
        ,a_Duration
        ,a_Premium
        ,z.PvNpfSglTgt[Irc7702::Opt1Int6Pct][a_Duration]
        ,z.PvNpfSglExc[Irc7702::Opt1Int6Pct][a_Duration]
        );
}

class FindSpecAmt
{
    typedef Irc7702::EIOBasis EIOBasis;
    BasicValues const& Values_;
    Irc7702     const& Irc7702_;
    EIOBasis    const  EIOBasis_;
    int         const  Duration;
    double      const  Premium;
    double      const  NetPmtFactorTgt;
    double      const  NetPmtFactorExc;

  public:
    FindSpecAmt
        (BasicValues const& a_Values
        ,Irc7702 const&     a_IRC7702
        ,EIOBasis           a_EIOBasis
        ,int                a_Duration
        ,double             a_Premium
        ,double             a_NetPmtFactorTgt
        ,double             a_NetPmtFactorExc
        )
        :Values_         {a_Values}
        ,Irc7702_        {a_IRC7702}
        ,EIOBasis_       {a_EIOBasis}
        ,Duration        {a_Duration}
        ,Premium         {a_Premium}
        ,NetPmtFactorTgt {a_NetPmtFactorTgt}
        ,NetPmtFactorExc {a_NetPmtFactorExc}
        {
        }
    // CURRENCY !! decimal_root() expects this; but see 'ihs_avsolve.cpp'.
    double operator()(double a_Trial) const
        {
        currency const SpecAmt = Values_.round_min_specamt().c(a_Trial);
        return
                Irc7702_.CalculatePremium
                    (EIOBasis_
                    ,Duration
                    ,a_Trial
                    ,a_Trial
                    ,a_Trial
                    ,NetPmtFactorTgt
                    ,NetPmtFactorExc
                    ,dblize(Values_.GetAnnualTgtPrem(Duration, SpecAmt))
                    )
            -   Premium
            ;
        }
};

/// CalculatePremium() implements an analytic solution, while CalculateSpecAmt()
/// uses iteration. Reason: we anticipate that no parameter depends on premium
/// except load (up to target vs. excess), so the direct solution isn't too
/// complicated. But when SpecAmt is unknown, we cannot know either the actual
/// specified-amount (underwriting) or ADD charge if they apply only up to some
/// maximum, or the target. So here we have eight special cases rather than
/// two, and adding another QAB like ADD could double the eight cases.
///
/// Return value is both specamt and bftamt; we name it 'specamt'
/// because it is typically used to set an input parameter, and
/// specamt is such a parameter whereas DB is not.

currency gpt_specamt::CalculateSpecAmt
    (BasicValues const& a_Values
    ,EIOBasis           a_EIOBasis
    ,int                a_Duration
    ,double             a_Premium
    ,double             a_NetPmtFactorTgt
    ,double             a_NetPmtFactorExc
    )
{
    LMI_ASSERT(0.0 != a_Premium);
    LMI_ASSERT(0.0 != a_NetPmtFactorTgt);
    LMI_ASSERT(0.0 != a_NetPmtFactorExc);

    Irc7702 const& z(safely_dereference_as<Irc7702>(a_Values.Irc7702_.get()));

    FindSpecAmt const fsa
        (a_Values
        ,z
        ,a_EIOBasis
        ,a_Duration
        ,a_Premium
        ,a_NetPmtFactorTgt
        ,a_NetPmtFactorExc
        );

    // No amount solved for can plausibly reach one billion dollars.
    // No amount lower than the product's minimum should be used.
    //
    // AccountValue::Solve() case 'mce_solve_specamt' solves for the
    // base specified amount, whereas this function sets the total;
    // their minimums deliberately differ. Using the lower minimum
    // might violate the "total" minimum for a product with a term
    // rider; that's okay when the user requests a solve, but not for
    // the strategy implemented here, which should work more robustly.
    root_type const solution = decimal_root
        (fsa
        ,dblize(a_Values.min_issue_spec_amt())
        ,999999999.99
        ,bias_higher
        ,z.round_min_specamt.decimals()
        ,64
        );

    // Because it is implausible that the upper bound is too low,
    // failure in practice implies that the solution would be lower
    // than the product minimum--in which case, return that minimum.
    switch(solution.validity)
        {
        case root_is_valid:
            {return a_Values.round_specamt().c(solution.root);}
        case root_not_bracketed:
            {return a_Values.min_issue_spec_amt();}
        case improper_bounds:
            {throw "CalculateSpecAmt: improper bounds.";}
        }
    throw "Unreachable--silences a compiler diagnostic.";
}
