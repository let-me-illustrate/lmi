// Determine specamt from GLP or GSP.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "gpt_specamt.hpp"

#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "safely_dereference_as.hpp"
#include "zero.hpp"

double gpt_specamt::CalculateGLPSpecAmt
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

double gpt_specamt::CalculateGSPSpecAmt
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
    double             SpecAmt;

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
        ,SpecAmt         {0.0}
        {
        }
    double operator()(double a_Trial)
        {
        SpecAmt = a_Trial;
        return
                Irc7702_.CalculatePremium
                    (EIOBasis_
                    ,Duration
                    ,a_Trial
                    ,a_Trial
                    ,a_Trial
                    ,NetPmtFactorTgt
                    ,NetPmtFactorExc
                    ,Values_.GetAnnualTgtPrem(Duration, SpecAmt)
                    )
            -   Premium
            ;
        }
    double Get()
        {
        return SpecAmt;
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

double gpt_specamt::CalculateSpecAmt
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

    FindSpecAmt fsa
        (a_Values
        ,z
        ,a_EIOBasis
        ,a_Duration
        ,a_Premium
        ,a_NetPmtFactorTgt
        ,a_NetPmtFactorExc
        );

    // TODO ?? The upper bound ideally wouldn't be hard coded; but if
    // it must be, then it can't plausibly reach one billion dollars.
    decimal_root
        (0.0
        ,999999999.99
        ,bias_higher
        ,z.round_min_specamt.decimals()
        ,fsa
        ,true
        );

    return fsa.Get();
}

