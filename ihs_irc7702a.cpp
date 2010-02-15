// Internal Revenue Code section 7702A (MEC testing).
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

// TODO ?? Make this a server app. Consider where to store DB, SA history.

// TODO ?? Do we need a separate function to handle withdrawals?

// TODO ?? Treat ROP increases as material changes exactly where needed.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_irc7702a.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"            // minmax<T>()
#include "stratified_algorithms.hpp" // TieredNetToGross()

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>                   // std::accumulate()
#include <stdexcept>

namespace
{
static int const         months_per_year              = 12          ;
static long double const years_per_month              = 1.0L / 12.0L;
static int const         statutory_max_endowment_age  = 100         ;
// TODO ?? Test period not limited to seven years for survivorship.
static int const         usual_test_period_length     = 7           ;
}

//============================================================================
Irc7702A::Irc7702A
    (int                         a_magic
    ,mcenum_defn_life_ins        a_DefnLifeIns
    ,mcenum_defn_material_change a_DefnMaterialChange
    ,bool                        a_IsSurvivorship
    ,mcenum_mec_avoid_method     a_AvoidMec
    ,bool                        a_Use7PPTable
    ,bool                        a_UseNSPTable
    ,std::vector<double> const&  a_SevenPPRateVec
    ,std::vector<double> const&  a_NSPVec
    ,round_to<double>    const&  a_RoundNonMecPrem
    )
    :state_               ()
    ,magic                (a_magic)
    ,DefnLifeIns          (a_DefnLifeIns)
    ,DefnMaterialChange   (a_DefnMaterialChange)
    ,UnnecPremIsMatChg    (false)
    ,ElectiveIncrIsMatChg (true)
    ,Exch1035IsMatChg     (true)
    ,CorrHidesIncr        (false)
    ,InterpolateNspOnly   (false)
    ,IsSurvivorship       (a_IsSurvivorship)
    ,AvoidMec             (a_AvoidMec)
    ,Use7PPTable          (a_Use7PPTable)
    ,UseNSPTable          (a_UseNSPTable)
    ,SevenPPRateVec       (a_SevenPPRateVec)
    ,NSPVec               (a_NSPVec)
    ,RoundNonMecPrem      (a_RoundNonMecPrem)
    ,DBDefn               (e_specamt_7702A)
    ,Ignore               (false)
    ,IsMec                (false)
    ,IsMatChg             (false)
    ,SevenPP              (0.0)
    ,CumSevenPP           (0.0)
    ,CumPmts              (0.0)
    ,LowestBft            (0.0)
    ,UnnecPrem            (0.0)
    ,UnnecPremPaid        (false)
    ,TestPeriodLen        (months_per_year * usual_test_period_length)
    ,TestPeriodDur        (0)
    ,PolicyYear           (0)
    ,PolicyMonth          (0)
    ,AssumedBft           (0.0)
    ,Saved7PPRate         (0.0)
    ,SavedAVBeforeMatChg  (0.0)
    ,SavedNecPrem         (0.0)
    ,SavedNSP             (0.0)
    ,SavedDCV             (0.0)
    ,NetNecessaryPrem     (0.0)
    ,GrossNecessaryPrem   (0.0)
{
    if(mce_cvat != DefnLifeIns && mce_gpt != DefnLifeIns)
        {
        Ignore = true;
        return;
        }

    // Set flags for implementation choices. Assigning values here
    // looks superfluous because it duplicates the initializer-list;
    // they're grouped here to emphasize them for maintainers.
    InterpolateNspOnly      = false;
    Exch1035IsMatChg        = true; // TODO ?? Silly--remove.
    CorrHidesIncr           = false;

    switch(DefnMaterialChange)
        {
        case mce_unnecessary_premium:
            {
            HOPEFULLY(mce_cvat == DefnLifeIns);
            UnnecPremIsMatChg    = true;
            ElectiveIncrIsMatChg = false;
            DBDefn = e_specamt_7702A;
            }
            break;
        case mce_benefit_increase:
            {
            HOPEFULLY(mce_cvat == DefnLifeIns);
            UnnecPremIsMatChg    = false;
            ElectiveIncrIsMatChg = true;
            DBDefn = e_death_benefit_7702A;
            }
            break;
        case mce_later_of_increase_or_unnecessary_premium:
            {
            // TODO ?? Not implemented yet.
            fatal_error()
                << "mce_later_of_increase_or_unnecessary_premium not implemented."
                << LMI_FLUSH
                ;
            HOPEFULLY(mce_cvat == DefnLifeIns);
            UnnecPremIsMatChg    = true;
            ElectiveIncrIsMatChg = true;
            DBDefn = e_specamt_7702A;
            }
            break;
        case mce_earlier_of_increase_or_unnecessary_premium:
            {
            HOPEFULLY(mce_cvat == DefnLifeIns);
            UnnecPremIsMatChg    = true;
            ElectiveIncrIsMatChg = true;
            DBDefn = e_specamt_7702A;
            }
            break;
        case mce_adjustment_event:
            {
            HOPEFULLY(mce_gpt == DefnLifeIns);
            UnnecPremIsMatChg    = false;
            ElectiveIncrIsMatChg = false;
            DBDefn = e_death_benefit_7702A;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << DefnMaterialChange
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Make sure the 7pp and NSP factors are all in (0, 1].

    minmax<double> extrema_7pp(SevenPPRateVec);
    LMI_ASSERT(0.0 < extrema_7pp.minimum() && extrema_7pp.maximum() <= 1.0);

    minmax<double> extrema_nsp(NSPVec);
    LMI_ASSERT(0.0 < extrema_nsp.minimum() && extrema_nsp.maximum() <= 1.0);
}

//============================================================================
Irc7702A::~Irc7702A()
{
}

//============================================================================
// Set initial values on issue or inforce date.
void Irc7702A::Initialize7702A
    (bool   a_Ignore
    ,bool   a_MecAtIssue
    ,int    a_IssueAge
    ,int    a_EndtAge
    ,int    a_PolicyYear
    ,int    a_PolicyMonth
    ,int    a_ContractYear
    ,int    a_ContractMonth
    ,double a_AVBeforeMatChg
    ,double a_LowestBft
    ,std::vector<double> const& a_Pmts
    ,std::vector<double> const& a_Bfts
    )
{
    LMI_ASSERT(a_ContractYear <= a_PolicyYear);
    state_.B0_deduced_policy_year   = a_PolicyYear;
    state_.B1_deduced_contract_year = a_ContractYear;
    LMI_ASSERT(0 < a_Bfts.size());
    double lowest_bft = *std::min_element(a_Bfts.begin(), a_Bfts.end());
    // Allow Bfts to be zero for solves.
    HOPEFULLY(0.0 <= lowest_bft);
    // TODO ?? Should we assert that this equals 'a_LowestBft'?
    // If we can, then we don't need the latter as an argument.

    Ignore = false;
    if(a_Ignore || (mce_cvat != DefnLifeIns && mce_gpt != DefnLifeIns))
        {
        Ignore = true;
        // TODO ?? An early return here as a speed optimization is not yet
        // safe. Routines to print optional monthly calculation detail rely
        // on sane values for some of the variables below. We could make
        // those routines depend on the early return condition here, or
        // figure out which variables we must initialize and do so before
        // any early return.
        // return;
        }

    UnnecPremPaid = false;
    IsMec = false;
    // TODO ?? I dislike this variable name because a contract can become a MEC
    // at issue even when the variable's value is false.
    if(a_MecAtIssue)
        {
        IsMec = true;
        // TODO ?? An early return here as a speed optimization is not yet
        // safe. Routines to print optional monthly calculation detail rely
        // on sane values for some of the variables below. We could make
        // those routines depend on the early return condition here, or
        // figure out which variables we must initialize and do so before
        // any early return.
        // return;
        }

    // MlyInterpNSP has 13 values, to cover beg and end of each of 12 months,
    // so that we can perform material change calculations at the end of the
    // twelfth month if desired.
    MlyInterpNSP.assign(1 + months_per_year, 0.0);

    PolicyYear          = a_PolicyYear;
    PolicyMonth         = a_PolicyMonth;

    // TODO ?? Not for survivorship.
    TestPeriodLen   = months_per_year * usual_test_period_length;
    TestPeriodDur   = a_ContractMonth + months_per_year * a_ContractYear;

    SavedAVBeforeMatChg = a_AVBeforeMatChg;

    unsigned int max_years =
            std::min(a_EndtAge, statutory_max_endowment_age)
        -   a_IssueAge
        ;
    // TODO ?? Do we really need '1 +'?
    unsigned int max_dur = 1 + months_per_year * max_years;
    Pmts.assign(max_dur, 0.0);
    Bfts.assign(max_dur, 0.0);

    HOPEFULLY(a_Pmts.size() <= max_years);
    for(unsigned int j = 0; j < a_Pmts.size(); ++j)
        {
        // TODO ?? OK to treat premium history as annual?
        Pmts[j * months_per_year] = a_Pmts[j];
        }
    HOPEFULLY(a_Bfts.size() <= max_years);
    for(unsigned int j = 0; j < a_Bfts.size(); ++j)
        {
        for(int k = 0; k < months_per_year; ++k)
            {
            Bfts[k + j * months_per_year] = a_Bfts[j];
            }
        }

    CumPmts         = std::accumulate(a_Pmts.begin(), a_Pmts.end(), 0.0);

    AssumedBft      = a_LowestBft; // TODO ?? Is this needed? Is it not always Bfts[0]?
    LowestBft       = a_LowestBft;

    HOPEFULLY
        (   static_cast<unsigned int>(PolicyYear - a_ContractYear)
        <   SevenPPRateVec.size()
        );
    Saved7PPRate       = SevenPPRateVec[std::max(0, PolicyYear - a_ContractYear)];
    state_.B2_deduced_px7_rate = SevenPPRateVec[std::max(0, PolicyYear - a_ContractYear)];
    SavedNecPrem    = 0.0;
    UnnecPrem       = 0.0;
    SavedNSP        = NSPVec[PolicyYear]; // TODO ?? Ignores interpolation.
    state_.B3_deduced_nsp_rate = NSPVec[PolicyYear];

    Determine7PP
        (a_LowestBft         // a_Bft
        ,false               // a_TriggeredByBftDecrease
        ,false               // a_TriggeredByMatChg
        ,false               // a_TriggeredByUnnecPrem
        ,SavedAVBeforeMatChg // a_AVBeforeMatChg
        ,0.0                 // a_NecPrem
        );
    CumSevenPP = SevenPP * (a_ContractYear + (0 != a_ContractMonth));

    if(!Use7PPTable || !UseNSPTable)
        {
        fatal_error()
            << "Present implementation requires 7702A factor tables."
            << LMI_FLUSH
            ;
        }

    NetNecessaryPrem   = 0.0;
    GrossNecessaryPrem = 0.0;

    LMI_ASSERT(TestPeriodDur < static_cast<int>(Bfts.size()));

    state_.C0_init_bft    = Bfts[TestPeriodDur];
    state_.C1_init_ldb    = a_LowestBft;
    state_.C2_init_amt_pd = 0.0;
    state_.C3_init_is_mc  = false;
    state_.C4_init_dcv    = 0.0;
    state_.C5_init_px7    = SevenPP;
    state_.C6_init_mec    = IsMec;

    state_.Q4_cum_px7    = CumSevenPP;
    state_.Q5_cum_amt_pd = CumPmts;
}

//============================================================================
// interpolate NSP; update cum 7pp
void Irc7702A::UpdateBOY7702A(int a_PolicyYear)
{
    if(Ignore || IsMec)
        {
        return;
        }

    state_.B0_deduced_policy_year = a_PolicyYear;
    PolicyYear = a_PolicyYear;

    // A negative policy year makes no sense
    HOPEFULLY(0 <= PolicyYear);

    // Update cumulative 7pp
    if(TestPeriodDur < TestPeriodLen)
        {
        CumSevenPP += SevenPP;
        }

    if(!InterpolateNspOnly)
        {
        MlyInterpNSP.assign(MlyInterpNSP.size(), NSPVec[PolicyYear]);
        }
    else
        {
        // We are aware of one system that uses monthly interpolation
        // for 7702A NSP but not for any other purpose (such as the
        // CVAT "corridor", which ought to be the reciprocal). This is
        // an extraordinary and unsound notion with no support in the
        // statute or the legislative history. Interpolation per se
        // may be in accordance with generally accepted actuarial
        // principles, but making up a new calculation rule and
        // applying it irregularly is beyond the pale. We emulate this
        // ill-advised behavior only as an exercise in matching
        // another system. Do not use this for production.
        double NSPBeg       = NSPVec[PolicyYear];
        double NSPEnd       = NSPVec[1 + PolicyYear];
        double lo_decrement = NSPBeg    * years_per_month;
        double hi_increment = NSPEnd    * years_per_month;
        double lo_val       = NSPBeg;
        double hi_val       = 0.0;
        for
            (std::vector<double>::iterator nsp_iter = MlyInterpNSP.begin()
            ;nsp_iter != MlyInterpNSP.end()
            ;nsp_iter++
            )
            {
            *nsp_iter = lo_val + hi_val;
            lo_val -= lo_decrement;
            hi_val += hi_increment;
            }

        // If correct increments and decrements were applied the correct number
        // of times, then
        //   lo_val should equal zero minus lo_decrement, and
        //   hi_val should equal the next year's NSP plus hi_increment
        HOPEFULLY(materially_equal(lo_val, (-lo_decrement)));
        HOPEFULLY(materially_equal(hi_val, (NSPEnd + hi_increment)));
        // The average of the interpolated values should equal the average
        // of the endpoints
        double avg_interp =
                std::accumulate(MlyInterpNSP.begin(), MlyInterpNSP.end(), 0.0)
            /   MlyInterpNSP.size();
        double avg_endpts =
            (NSPBeg + NSPEnd)
            /   2.0;
        HOPEFULLY(materially_equal(avg_interp, avg_endpts));
        // We do not assert that NSP increases by duration. That might not be
        // true in the case of a high substandard rating that is "forgiven"
        // after some period of time.
        }

    // state_.Q4_cum_px7 and state_.Q5_cum_amt_pd are not updated here
    // even though this function modifies CumSevenPP. Perhaps that
    // modification is a mistake: this function is called at the
    // beginning of each policy year, but the premium limit applies
    // to contract years.
}

//============================================================================
void Irc7702A::UpdateBOM7702A(int a_PolicyMonth)
{
    if(Ignore || IsMec)
        {
        return;
        }

    IsMatChg = false;
    PolicyMonth = a_PolicyMonth;
    // A negative policy month makes no sense
    HOPEFULLY(0 <= PolicyMonth);
}

//============================================================================
// Increment policy month; return MEC status.
bool Irc7702A::UpdateEOM7702A()
{
    if(!(Ignore || IsMec))
        {
        TestPeriodDur++;
        }
    return IsMec;
}

//============================================================================
// Process 1035 exchange as material change
// TODO ?? Unnecessary premium tested later, not here?
// TODO ?? Second argument won't be needed after we redo class AccountValue.
void Irc7702A::Update1035Exch7702A
    (double& a_DeemedCashValue
    ,double  a_Net1035Amount
    ,double  a_Bft
    )
{
    HOPEFULLY(0.0 <= a_Net1035Amount);
    a_DeemedCashValue = a_Net1035Amount;

    if(Ignore)
        {
        return;
        }

    // If the net 1035 amount is more than the NSP, then the contract
    // becomes a MEC as of the issue date. But the exchanged contract
    // is generally assigned to the new insurer, which then surrenders
    // it, and the proceeds are received later--but applied as of the
    // issue date. So if the amount eventually received is too high,
    // an irremediable MEC results. We hesitate to "avoid" a MEC by
    // increasing the initial benefit because of this.
    if(a_Bft * MlyInterpNSP[PolicyMonth] < a_Net1035Amount)
        {
        IsMec = true;
        }

    if(IsMec)
        {
        SevenPP = 0.0;
        }
    else
        {
        // For illustrations, allow 1035 only at issue
        LMI_ASSERT(0 == PolicyYear && 0 == PolicyMonth);
        Bfts[TestPeriodDur] = a_Bft;

        if(Exch1035IsMatChg && 0.0 != a_Net1035Amount)
            {
            IsMatChg = true;
            RedressMatChg
                (a_DeemedCashValue
                ,0.0                // a_UnnecPrem
                ,0.0                // a_NecPrem
                ,a_Net1035Amount
                );
            IsMatChg = false;
            // 'SavedDCV' is used only for monthly trace.
            SavedDCV = a_DeemedCashValue;
            }
        }

    state_.C0_init_bft    = Bfts[TestPeriodDur];
//  state_.C1_init_ldb      does not change here.
//  state_.C2_init_amt_pd   does not change here.
//  state_.C3_init_is_mc    does not change here.
    state_.C4_init_dcv    = a_DeemedCashValue;
    state_.C5_init_px7    = SevenPP;
    state_.C6_init_mec    = IsMec;

    state_.Q0_net_1035    = a_Net1035Amount;
}

//============================================================================
bool Irc7702A::IsMaterialChangeInQueue() const
{
    return IsMatChg;
}

//============================================================================
// If a queued material change is pending, then it is to be
// redressed after accepting necessary premium but before
// accepting unnecessary premium. Necessary premium affects
// cash value, which affects the seven-pay premium--so this
// calculation has a removable circularity:
//
// 7pp = 7Px * (Benefit - (CV + net necessary prem) / Ax)
//
// To determine the gross max 7pp, G, assuming it's all necessary:
// G = 7Px * (Benefit - (CV + G * (1 - prem_load)) / Ax)
// G = 7Px * Benefit - 7Px * (CV + G * (1 - prem_load)) / Ax
// G = 7Px * Benefit - 7Px * CV / Ax - 7Px * G * (1 - prem_load) / Ax
// G + 7Px * G * (1 - prem_load) / Ax = 7Px * Benefit - 7Px * CV / Ax
// G * (1 + 7Px * (1 - prem_load) / Ax) = 7Px * Benefit - 7Px * CV / Ax
// G = (7Px * Benefit - 7Px * CV / Ax) / (1 + 7Px * (1 - prem_load) / Ax)
// G = (Ax * 7Px * Benefit - 7Px * CV) / (Ax + 7Px * (1 - prem_load))
// G = 7Px * (Ax * Benefit - CV) / (Ax + 7Px * (1 - prem_load))
//
// Thus,
//
//                   Ax * Benefit - CV
//     G = 7Px * --------------------------
//               Ax + 7Px * (1 - prem_load)
//
// as long as G <= target; otherwise,
//
//               Ax * Benefit - CV + Target * (target_load - excess_load)
//     G = 7Px * --------------------------------------------------------
//                             Ax + 7Px * (1 - excess_load)
//
// If the result is less than the maximum necessary premium, then the
// calculation is conservative.
//
// This implementation assumes that a scalar premium tax is passed as
// part of the loads if it's recognized at all. AK and SD have tiered
// premium tax rates with a break at $100,000 as of 2003-09, which is
// unlikely to equal the target premium; they could be handled here,
// but for simplicity are ignored, so the rate for the lowest tier
// should be passed, making the calculation conservative. It would
// seem that we could look ahead and find the effective scalar premium
// tax rate for all modal premiums in a policy year, but that ignores
// timing of modal premiums--some necessary premiums would be too high
// and others too low--and, worse, we cannot look ahead at premiums
// that may be altered as a result of these 7702A calculations because
// of some MEC avoidance strategy. Furthermore, AK and SD premium tax
// is calculated by policy year, not by 7702A contract year. DE's
// tiered premium tax would be even harder to deal with here because
// it applies to all premiums paid by the same corporation, aggregated
// together, so any particular contract's actual premium tax depends
// on the order in which all contracts owned by the same corporation
// are processed--and also because DE tiering is done by calendar year
// rather than by policy year or 7702A contract year.
//
// Necessary premium can also affect the corridor death benefit.
// This implementation assumes that "death benefit" for 7702A is
// defined as specamt.
//
double Irc7702A::MaxNonMecPremium
    (double a_DeemedCashValue
    ,double a_TargetPrem
    ,double a_LoadTarget
    ,double a_LoadExcess
    ,double a_CashValue
    ) const
{
    // state_.B4_deduced_target_premium etc. are not set here because
    // this function is a mere inquiry, not an essential transaction.
    // However, state_.Q6_max_non_mec_prem is recorded because it's
    // useful to test despite all that.

    if(Ignore || IsMec)
        {
        state_.Q6_max_non_mec_prem = std::numeric_limits<double>::max();
        return state_.Q6_max_non_mec_prem;
        }

    // We queue all material change events arising on the same day,
    // then combine them and process only one material change.
    // Therefore, here we must first ascertain whether any such event
    // is in the queue, due, for instance, to a benefit increase.
    //
    // If the queue is empty
    //   then within a seven-pay period, max is the 7pp
    //   else max is the maximum necessary premium, which makes the
    //     contract paid up on a 7702A basis
    // else if the queue is not empty
    //   then we can look through the pending material change to
    //     determine its effect on the 7pp
    //
    if(!IsMaterialChangeInQueue())
        {
        if(TestPeriodDur < TestPeriodLen)
            {
            HOPEFULLY(CumPmts <= CumSevenPP);
            state_.Q6_max_non_mec_prem = RoundNonMecPrem(CumSevenPP - CumPmts);
            return state_.Q6_max_non_mec_prem;
            }
        else
            {
            state_.Q6_max_non_mec_prem = MaxNecessaryPremium
                (a_DeemedCashValue
                ,a_TargetPrem
                ,a_LoadTarget
                ,a_LoadExcess
                ,a_CashValue
                );
            return state_.Q6_max_non_mec_prem;
            }
        }
    else
        {
        // TODO ?? For GPT, this is presumed valid but not yet tested.
        //
        // Initially assume the result is less than target. If it
        // turns out to be greater, then use a different formula.
        double g =
                  SevenPPRateVec[PolicyYear]
                * ( MlyInterpNSP[PolicyMonth] * Bfts[TestPeriodDur]
                  - a_CashValue
                  )
                / ( MlyInterpNSP[PolicyMonth]
                  + SevenPPRateVec[PolicyYear] * (1.0 - a_LoadTarget)
                  )
                ;
        if(a_TargetPrem < g)
            {
            g =
                  SevenPPRateVec[PolicyYear]
                * ( MlyInterpNSP[PolicyMonth] * Bfts[TestPeriodDur]
                  - a_CashValue
                  - a_TargetPrem * (a_LoadTarget - a_LoadExcess)
                  )
                / ( MlyInterpNSP[PolicyMonth]
                  + SevenPPRateVec[PolicyYear] * ( 1.0 - a_LoadTarget)
                  )
                ;
            }
        state_.Q6_max_non_mec_prem = RoundNonMecPrem(g);
        return state_.Q6_max_non_mec_prem;
        }
}

//============================================================================
double Irc7702A::MaxNecessaryPremium
    (double a_DeemedCashValue
    ,double a_TargetPrem
    ,double a_LoadTarget
    ,double a_LoadExcess
    ,double a_CashValue
    ) const
{
    state_.B4_deduced_target_premium = a_TargetPrem;
    state_.B5_deduced_target_load    = a_LoadTarget;
    state_.B6_deduced_excess_load    = a_LoadExcess;
    if(Ignore || IsMec || mce_gpt == DefnLifeIns)
        {
        state_.Q1_max_nec_prem_net   = std::numeric_limits<double>::max();
        state_.Q2_max_nec_prem_gross = std::numeric_limits<double>::max();
        return std::numeric_limits<double>::max();
        }

    DetermineLowestBft();
    double nsp = MlyInterpNSP[PolicyMonth] * LowestBft;

    HOPEFULLY(0.0 <= a_DeemedCashValue);
    // We don't assert
    //  HOPEFULLY(0.0 <= a_CashValue);
    // because allowing account (hence cash) value to be negative
    // makes solves easier.

    NetNecessaryPrem = nsp - std::min(a_CashValue, a_DeemedCashValue);
    NetNecessaryPrem = std::max(0.0, NetNecessaryPrem);

    // Gross up NetNecessaryPrem for premium load only. Other charges and fees
    // are ignored because they are imposed whether or not premium is paid
    // and are not necessarily synchronized with premium.
    GrossNecessaryPrem = TieredNetToGross
        (NetNecessaryPrem
        ,a_TargetPrem
        ,a_LoadTarget
        ,a_LoadExcess
        );

    state_.Q1_max_nec_prem_net = NetNecessaryPrem;
    state_.Q2_max_nec_prem_gross = RoundNonMecPrem(GrossNecessaryPrem);
    return state_.Q2_max_nec_prem_gross;
}

//============================================================================
// record and test monthly Pmts
double Irc7702A::UpdatePmt7702A
    (double a_DeemedCashValue
    ,double a_Payment
    ,bool   a_ThisPaymentIsUnnecessary
    ,double // a_TargetPrem
    ,double // a_LoadTarget
    ,double // a_LoadExcess
    ,double // a_CashValue
    )
{
    if(Ignore || IsMec)
        {
        return a_Payment;
        }

// TODO ?? Not necessarily true if we net out WD?
//  HOPEFULLY(0.0 <= a_Payment);

    // As long as we're MEC testing, this function should be called whenever a
    // payment is set, and no more than once each month. Therefore, when it's
    // called, the payment for the current duration should have its default
    // value of zero.
    //
    // TODO ?? But then how will we handle a WD? Separate function?
    //
    // Changed anyway: called twice a month when there's a material change.
//    HOPEFULLY(0.0 == Pmts[TestPeriodDur]);

    // During the test period (only), we accumulate premiums and compare
    // to the seven-pay limit. We store premium history for this period(or longer?)
    // so that we can perform this comparison afresh in retrospect when
    // Bfts decreases.
    if(TestPeriodDur < TestPeriodLen)
        {
        // Reduce pmt to seven-pay limit if desired before recording it.
        // This won't avoid a retrospective MEC if Bfts later decreases.
        // TODO ?? This is unnecessary now--premium limited in caller.
        /*
        if(mce_reduce_prem == AvoidMec)
            {
            a_Payment = std::min
                (a_Payment
                ,MaxNonMecPremium
                    (a_TargetPrem
                    ,a_LoadTarget
                    ,a_LoadExcess
                    ,a_CashValue
                    )
                );
            }
        */
        CumPmts += a_Payment;
        if(CumSevenPP < CumPmts)
            {
            IsMec = true;
/* TODO ?? Reenable after testing.
            if(mce_reduce_prem == AvoidMec)
                {
                warning()
                    << "Failed to avoid MEC by reducing premium."
                    << LMI_FLUSH
                    ;
                }
*/
            state_.Q4_cum_px7    = CumSevenPP;
            state_.Q5_cum_amt_pd = CumPmts;
// Don't return before recording state_.
//            return a_Payment;
            }
        }

    // Test for unnecessary premium, which we deem to be a material change

// TODO ?? Under GPT always MatChg if (iff?) GLP increased or if pmt increases ROP Bfts.

    if(mce_gpt == DefnLifeIns)
        {
        Pmts[TestPeriodDur] = a_Payment;
//      state_.F0_nec_pm_bft       does not change here.
//      state_.F1_nec_pm_ldb       does not change here.
        state_.F2_nec_pm_amt_pd  = Pmts[TestPeriodDur];
//      state_.F3_nec_pm_is_mc     does not change here.
//      state_.F4_nec_pm_dcv       does not change here.
//      state_.F5_nec_pm_px7       does not change here.
        state_.F6_nec_pm_mec     = IsMec;
        state_.Q4_cum_px7    = CumSevenPP;
        state_.Q5_cum_amt_pd = CumPmts;
        return a_Payment;
        }

    // Reduce pmt to necessary premium if desired before accepting it.
    // This won't avoid a retrospective MEC if Bfts later decreases.
    // TODO ?? Anyway, why limit premium to necessary?
    if(mce_reduce_prem == AvoidMec)
        {
    // TODO ?? Not needed unless we try handling MEC avoidance here.
/*
    double max_necessary_prem = MaxNecessaryPremium
        (a_DeemedCashValue
        ,a_TargetPrem
        ,a_LoadTarget
        ,a_LoadExcess
        ,a_CashValue
        );
        a_Payment = std::min(a_Payment, max_necessary_prem);
*/
        }

    Pmts[TestPeriodDur] = a_Payment;

    UnnecPrem = 0.0;
    // If unnecessary premium was paid...
    if(a_ThisPaymentIsUnnecessary)
        {
        LMI_ASSERT(mce_cvat == DefnLifeIns);
        UnnecPrem = a_Payment;
        UnnecPremPaid = true;
        // One school of thought deems any unnecessary premium to be a MatChg.
        // This is conservative: a MatChg might be recognized only upon any
        // subsequent increase in Bfts (including increases due to the
        // corridor, or to DB increments for option 2), which might
        // never occur.
        //
        // However, a material change must be processed before any
        // unnecessary premium is accepted. This code defectively
        // accepts it while raising a flag calling for the material
        // change to be processed later. That's okay as long as a
        // material change has just been processed--but in that case
        // the 'IsMatChg' flag shouldn't be raised here.
        if(UnnecPremIsMatChg) // TRUE
            {
            IsMatChg = true;
            Pmts[TestPeriodDur] = UnnecPrem;
/*
            RedressMatChg
                (a_DeemedCashValue
                ,UnnecPrem
                ,NetNecessaryPrem
                ,a_CashValue
                );
*/
            }
        // a_DeemedCashValue is used only for setting state_. It isn't
        // changed by this function, because the caller might handle
        // it in some special manner unrelated to 7702A calculations:
        // for instance, rounding it piecewise for allocation among
        // separate accounts.
//      state_.H0_unnec_pm_bft      does not change here.
//      state_.H1_unnec_pm_ldb      does not change here.
        state_.H2_unnec_pm_amt_pd = Pmts[TestPeriodDur];
        state_.H3_unnec_pm_is_mc  = IsMatChg;
        state_.H4_unnec_pm_dcv    = a_DeemedCashValue;
//      state_.H5_unnec_pm_px7      does not change here.
        state_.H6_unnec_pm_mec    = IsMec;
        }
    else
        {
//      state_.F0_nec_pm_bft       does not change here.
//      state_.F1_nec_pm_ldb       does not change here.
        state_.F2_nec_pm_amt_pd  = Pmts[TestPeriodDur];
//      state_.F3_nec_pm_is_mc     does not change here.
        state_.F4_nec_pm_dcv     = a_DeemedCashValue;
//      state_.F5_nec_pm_px7       does not change here.
        state_.F6_nec_pm_mec     = IsMec;
        }

    state_.Q4_cum_px7    = CumSevenPP;
    state_.Q5_cum_amt_pd = CumPmts;

    return a_Payment;
}

//============================================================================
// record and test monthly Bfts
// This function always returns zero, so it shouldn't return anything.
double Irc7702A::UpdateBft7702A
    (double // a_DeemedCashValue // TODO ?? Not used.
    ,double  a_NewDB
    ,double  a_OldDB
    ,bool    a_IsInCorridor
    ,double  a_NewSA
    ,double  a_OldSA
    ,double  // a_CashValue // TODO ?? Not used.
    )
{
    if(Ignore || IsMec)
        {
        return 0.0;
        }

    // Allow Bfts to be zero for solves.
    HOPEFULLY(0.0 <= a_NewDB);
    HOPEFULLY(0.0 <= a_OldDB);
    HOPEFULLY(0.0 <= a_NewSA);
    HOPEFULLY(0.0 <= a_OldSA);

    // I believe that the death benefit, unlike the premium, can be set more
    // than once per month in the present code. I do not know whether or not
    // this can be avoided. TODO ?? Figure this out.
    HOPEFULLY(TestPeriodDur < static_cast<int>(Bfts.size()));

    double current_bft = 0.0;
    if(e_death_benefit_7702A == DBDefn)
        {
        current_bft = a_NewDB;
        }
    // Defining 7702A Bfts to be spec amt avoids real practical problems,
    // such as a lot of little decreases with option 2 due to increasing
    // charges at older ages.
    else if(e_specamt_7702A == DBDefn)
        {
        current_bft = a_NewSA;
        }
    else
        {
        throw std::logic_error("7702A definition of 'death benefit' unknown.");
        }

    Bfts[TestPeriodDur] = current_bft;
    if(current_bft == AssumedBft)
        {
        return 0.0;
        }

    if(AssumedBft < current_bft)
        {
        state_.D0_incr_bft    = current_bft;
//      state_.D1_incr_ldb      does not change here.
//      state_.D2_incr_amt_pd   does not change here.
//      state_.D3_incr_is_mc    may change below.
//      state_.D4_incr_dcv      does not change here.
//      state_.D5_incr_px7      does not change here.
//      state_.D6_incr_mec      does not change here.
        }
    else
        {
        TestBftDecrease(current_bft);
        }

    // One school of thought deems payment of unnecessary premium to be a
    // MatChg, and disregards all increases.
    bool is_material_change = false;
    // One school of thought treats any elective increase as a MatChg.
    // This code will also pick up SA increases due to certain option changes.
    // TRICKY !! This is not the same criterion as
    //   AssumedBft < current_bft
    // used above. Suppose SA is initially $3M, is reduced to $1M
    // after seven years (avoiding the reduction rule), and is then
    // increased to $2M. Then:
    //   $3M = AssumedBft
    //   $1M = a_OldSA
    //   $2M = a_NewSA
    // and this code triggers a material change for that "increase",
    // even though the new SA is less than that used last time 7PP
    // was calculated (i.e., LDB, which is $3M throughout this
    // example). Therefore, this assertion:
    //   LMI_ASSERT(AssumedBft < current_bft);
    // had to be suppressed below when a material change is noted.
    if(ElectiveIncrIsMatChg && a_OldSA < a_NewSA)
        {
        // Some adherents of that school however ignore SA increases
        // that do not affect the DB because it's in the corridor.
        if(CorrHidesIncr && a_IsInCorridor)
            {
            }
        else
            {
            is_material_change = true;
            }
        }
    // One school of thought treats any Bft increase following any unnec prem
    // as a MatChg.
#ifdef THIS_IS_NOT_DEFINED
    // TODO ?? Recognizing a MatChg wipes the slate clean: it is as though no
    // unnecessary premium had ever been paid. So 'UnnecPremEver' is
    // a nonsensical notion.
    if(UnnecPremPaid && AssumedBft < current_bft)
        {
        is_material_change = true;
        }
#endif

    if(is_material_change)
        {
// Suppressed--see note above:
//        LMI_ASSERT(AssumedBft < current_bft); // No decrease is a MC.
        IsMatChg = true;
        state_.D3_incr_is_mc = true;
/*
        RedressMatChg
            (a_DeemedCashValue
            ,0.0            // TODO ?? a_UnnecPrem
            ,0.0            // a_NecPrem
            ,a_CashValue
            );
*/
        }

    return 0.0;
}

//============================================================================
// if within a test period:
// recalculate 7pp and apply retroactively to beginning of 7 yr period
void Irc7702A::TestBftDecrease(double a_NewBft)
{
    // TODO ?? Is AssumedBft always equal to LowestBft?

    // Bfts reductions during any seven-year test period need to be tested.
    // In addition, only for second-to-die (but not first-to-die) contracts,
    // Bfts reductions have to be tested whenever they occur, even if it's
    // outside any seven-year test period; but such testing covers only
    // seven years in any event.
    // VERIFY Bfts decrease at end of period, and just beyond; also dur zero
    // VERIFY same with survivorship; also in last contract month
    if(!(IsSurvivorship || TestPeriodDur < TestPeriodLen))
        {
        return;
        }

    // If the new (reduced) Bft is not lower than the Bft assumed in
    // calculating the last 7pp, then there's no need to retest. If
    // that was the case, we shouldn't have gotten here.
    HOPEFULLY(a_NewBft < LowestBft);
    LowestBft = std::min(LowestBft, a_NewBft);

    // Recalculate 7pp to reflect lower death benefit.
    //
    // TODO ?? If the benefit decreases subsequent to a MatChg, then the 7pp
    // recalculation should use AV, NecPrem, and NSP as of the last MatChg.
    Determine7PP
        (a_NewBft
        ,true       // a_TriggeredByBftDecrease
        ,false      // a_TriggeredByMatChg
        ,false      // a_TriggeredByUnnecPrem
        ,0.0        // a_AVBeforeMatChg doesn't matter
        ,0.0        // a_NecPrem doesn't matter
        );

    // Retest all premiums from beginning of test period.
    //
    // TODO ?? We also need to test for unnecessary premium in retrospect.
    //
    // Recalculate CumSevenPP because SevenPP changed.
    CumSevenPP = 0.0;
    double cum_prem = 0.0;

    std::vector<double>::const_iterator prem_iter = Pmts.begin();
    for
        (int j = 0
        ; j < std::min(TestPeriodLen, 1 + TestPeriodDur) && prem_iter != Pmts.end()
        ; j++, prem_iter++
        )
        {
        cum_prem += *prem_iter;
        if(0 == j % 12)
            {
            CumSevenPP += SevenPP;
            }
        if(CumSevenPP < cum_prem)
            {
            IsMec = true;
            break;
            }
        }

    if(!IsMec && !materially_equal(cum_prem, CumPmts))
        {
        // The premium we just accumulated since the start of the test
        // period should equal the value saved in CumPmts, as long as we
        // did not exit the above loop early. If we exited it early, due
        // to becoming a MEC in a past year, then we didn't add all the
        // payments together; but we don't need to, since we're a MEC
        // and the cumulative payments are no longer relevant.
        fatal_error()
            << "Cumulative premium during most recent seven-pay period"
            << " should be " << CumPmts
            << " but is " << cum_prem
            << "; discrepancy is " << (cum_prem - CumPmts) << "."
            << LMI_FLUSH
            ;
        }
    else
        {
        // ...but even if we exited the loop early, let's update CumPmts
        // for the monthly trace. This doesn't matter for any other purpose,
        // because if we got here, the policy is a MEC.
        CumPmts = cum_prem;
        }

    state_.E0_decr_bft    = a_NewBft;
    state_.E1_decr_ldb    = LowestBft;
//  state_.E2_decr_amt_pd   does not change here.
//  state_.E3_decr_is_mc    does not change here.
//  state_.E4_decr_dcv      does not change here.
    state_.E5_decr_px7    = SevenPP;
    state_.E6_decr_mec    = IsMec;

    state_.Q4_cum_px7    = CumSevenPP;
    state_.Q5_cum_amt_pd = CumPmts;
}

//============================================================================
void Irc7702A::InduceMaterialChange()
{
    IsMatChg = true;
}

//============================================================================
// handle material change:
// recalculate 7pp
// determine whether MEC
// start new 7 pay period; terminate old one
void Irc7702A::RedressMatChg
    (double& a_DeemedCashValue
    ,double  a_UnnecPrem
    ,double  a_NecPrem
    ,double  a_CashValue
    )
{
// TODO ?? I think all public functions in this class need this test:
    if(Ignore || IsMec || !IsMaterialChangeInQueue())
        {
        return;
        }

    state_.Q3_cv_before_last_mc = a_CashValue;

    UnnecPremPaid = false;

    // Set DCV = AV upon material change
    a_DeemedCashValue = std::max(0.0, a_CashValue);
    // 'SavedDCV' is used only for monthly trace.
    SavedDCV = a_DeemedCashValue;

    // TODO ?? What if account value is less than zero here?

    // Recalculate 7pp to reflect material change
    Determine7PP
// TODO ?? If MatChg due to unnec prem, should Bfts reflect any corridor increase?
// Yes.
        (Bfts[TestPeriodDur]    // a_Bft
        ,false                  // a_TriggeredByBftDecrease
        ,true                   // a_TriggeredByMatChg
        ,0.0 < a_UnnecPrem      // a_TriggeredByUnnecPrem
        ,a_CashValue            // a_AVBeforeMatChg
        ,a_NecPrem              // a_NecPrem
        );

    // Drop old Bfts and premium history, as though the contract were reissued
    Bfts.erase
        (Bfts.begin()
        ,Bfts.begin() + TestPeriodDur
        );
    Pmts.erase
        (Pmts.begin()
        ,Pmts.begin() + TestPeriodDur
        );
// TODO ?? Is the latest payment still there?

    // Start new 7 pay period
    TestPeriodDur = 0;

    // A new contract is deemed to be issued, wiping out old history.
    LowestBft = Bfts[0];
//    Pmts[0] = a_UnnecPrem;
//    CumPmts = Pmts[0];
    Pmts[0] = 0.0;
    CumPmts = 0.0;

    // Apply 7 pay test.
    CumSevenPP = SevenPP;
    if(CumSevenPP < CumPmts)
        {
        IsMec = true;
        }

//  state_.G0_do_mc_bft      does not change here.
    state_.G1_do_mc_ldb    = LowestBft;
//  state_.G2_do_mc_amt_pd   does not change here.
    state_.G3_do_mc_is_mc  = false;
    state_.G4_do_mc_dcv    = a_DeemedCashValue;
    state_.G5_do_mc_px7    = SevenPP;
    state_.G6_do_mc_mec    = IsMec;

    state_.Q4_cum_px7    = CumSevenPP;
    state_.Q5_cum_amt_pd = CumPmts;
}

//============================================================================
// update 7pp
// Restructure this--too many TriggeredBy's
void Irc7702A::Determine7PP
    (double a_Bft
    ,bool   // a_TriggeredByBftDecrease
    ,bool   a_TriggeredByMatChg
    ,bool   a_TriggeredByUnnecPrem
    ,double a_AVBeforeMatChg
    ,double a_NecPrem
    )
{
    // We always treat payment of unnecessary premium as a material change
    // So this test is unnecessary; so is the parm in the conditional.
    // TODO ?? NO LONGER TRUE.
    if(a_TriggeredByUnnecPrem)
        {
        HOPEFULLY(a_TriggeredByMatChg);
        }

    AssumedBft = a_Bft;

    // Store new values for
    //  Saved7PPRate, SavedAVBeforeMatChg, SavedNecPrem, and SavedNSP
    // iff 7pp recalculation due to material change. (But how could
    // a material change mutate 'Saved7PPRate'?)
    // But leave those values undisturbed if triggered by Bfts decrease.
    if(a_TriggeredByMatChg)
        {
        Saved7PPRate = SevenPPRateVec[PolicyYear];
        SavedAVBeforeMatChg = a_AVBeforeMatChg;
        SavedNSP = MlyInterpNSP[PolicyMonth];
        // Save the necessary portion of the premium only if unnecessary
        // premium was paid. Otherwise, the premium, which was all necessary,
        // simply went into the AV. SavedNecPrem is used only to adjust
        // the AV saved prior to a material change.
        // TODO ?? Is that rigorous?
        if(a_TriggeredByUnnecPrem)
            {
            SavedNecPrem = a_NecPrem;
            }
        else
            {
            SavedNecPrem = 0.0;
            }
        }
    else
        {
        // No material change--either we're initializing, or processing
        // a Bfts decrease. NO...could be an inforce case.
// TODO ?? expunge
//        HOPEFULLY
//            (   a_TriggeredByBftDecrease
//            ||  (0 == PolicyYear && 0 == PolicyMonth)
//            );
        // When recalculating premium due to a death benefit decrease,
        // use the values stored (at the beginning of the test period) for
        //  Saved7PPRate, SavedAVBeforeMatChg, SavedNecPrem, and SavedNSP
        // at the last material change. When initializing, those variables
        // should have their proper initial values.
        if(0 == PolicyYear && 0 == PolicyMonth)
            {
//          HOPEFULLY(0.0                == SavedAVBeforeMatChg);
// SavedAVBeforeMatChg should be zero unless there was a 1035, in
// which case it should be the net 1035 amount. I'm not sure we should
// either rely on 'SavedDCV' (which was added for monthly trace only) or
// pass DCV as an argument; maybe we should drop the assertion. Note
// that 'SavedDCV', because of its limited purpose, isn't reliably
// initialized; I'm not sure we should promote that variable to a
// first-class citizen by initializing it carefully.
            HOPEFULLY
                (   0.0                 == SavedAVBeforeMatChg
                ||  materially_equal(SavedDCV, SavedAVBeforeMatChg)
                );
            HOPEFULLY(0.0                == SavedNecPrem);
            HOPEFULLY(materially_equal(SevenPPRateVec[0], Saved7PPRate));
            HOPEFULLY(materially_equal(NSPVec[0], SavedNSP));
            }
/* TODO ?? Expunge this perhaps. Not sure what we should do if someone
tries running an inforce case as of month 0, year 0.
        // In either of these two cases--initialization or Bfts reduction--the
        // a_AVBeforeMatChg and a_NecPrem arguments are not used, so give a
        // warning if they were nonzero.
//        if
//            (   a_TriggeredByBftDecrease
//            ||  (0 == PolicyYear && 0 == PolicyMonth)
//            )
//            {
//            HOPEFULLY(0.0 == a_AVBeforeMatChg);
//            HOPEFULLY(0.0 == a_NecPrem);
//            }
*/
        }

    // The AV just before the material change is increased by the
    // maximum necessary premium. Reason: premium up to the maximum
    // necessary is first accepted, and then it is any unnecessary
    // excess that triggers a MatChg. This applies to 1035 exchanges as
    // long as the net 1035 amount is already in the AV prior to the MatChg.
    // This applies to benefit increases as long as they are not
    // accompanied by any unnecessary premium (TODO ?? combine these TX's?).
    //
    // We prefer this
    //   P * [Bfts - (AV+NecP)/A]
    // to
    //   P * Bfts * [1 - (AV+NecP)/(A*Bfts)]
    // because it avoids a division. The reason is not so much the cost
    // of a division as its inherent risk--this way, we don't have to
    // worry about whether the denominator is zero. For instance, Bfts
    // is allowed to be zero for solves.
    double bft_adjustment = SavedAVBeforeMatChg / SavedNSP;
// TODO ?? expunge
//    double bft_adjustment =
//            (SavedAVBeforeMatChg + SavedNecPrem)
//        /   SavedNSP
//        ;
    double adjusted_bft = AssumedBft - bft_adjustment;
    if(0.0 < adjusted_bft)
        {
        SevenPP = Saved7PPRate * adjusted_bft;
        }
    else
        {
        SevenPP = 0.0;
        }
// TODO ?? AssumedBft should reflect any Bfts increase--AFTER the MatChg?
}

//============================================================================
// update LowestBft dynamically
double Irc7702A::DetermineLowestBft() const
{
    LMI_ASSERT(0 <= TestPeriodLen && 0 <= TestPeriodDur);
    std::vector<double>::const_iterator last_bft_in_test_period = std::min
        (Bfts.end()
        ,Bfts.begin() + std::min(TestPeriodLen, TestPeriodDur)
        );
    LMI_ASSERT(Bfts.begin() <= last_bft_in_test_period);
    LowestBft = *std::min_element(Bfts.begin(), last_bft_in_test_period);
    return LowestBft;
}

//============================================================================
// determine lowest non-MEC spec amt
double Irc7702A::SAIncreaseToAvoidMec(bool a_TriggeredByUnnecPrem)
{
// TODO ?? Specs say DB, but isn't this SA?
    double av = SavedAVBeforeMatChg;
    if(a_TriggeredByUnnecPrem)  // TODO ?? iff MatChg triggered by unnec prem?
        {
        av += SavedNecPrem;
        }
    // Why aren't we using something like "SavedUnnecPrem" instead?
    double new_bft =
            UnnecPrem / Saved7PPRate
        +   av
            /   SavedNSP
        ;

// Specs say this formula is for Bfts increase to avoid MEC when unnec prem
// paid. Also need to handle case where prem exceeds 7pp. Also need to handle
// dumpins. Also need to handle Bfts decrease (complicated). Also need to
// reflect new 7pp upon MatChg. TODO ?? Where should we do all this?
//
// Events that are MatChgs:
//   1035 exchange--but we treat that as a special case
//   payment of unnecessary premium
//   elective SA increase or ROP Bfts increase
//
// We cannot increase the Bfts to avoid the MatChg altogether, since a MatChg is
// caused by increasing the SA to raise the DB.

    return new_bft;
}

/*
 * // Entry points
 *
 * Triggers:
 * OK  whenever prem paid
 * OK  elective Bfts increase
 * OK  whenever Bfts decreases
 *     whenever GP limit increases
 *     upon any ROP increase (GPT only)
 *     [internal] whenever unnec prem paid
 *
 * /////////////////
 *
 * (obsolete design)
 * redress MatChg
 *     triggers:
 *         GPT:
 *             whenever GP limit increases
 *             upon any ROP increase
 *                 doesn`t that increase the GP limit?
 *         CVAT:
 *             whenever unnec prem paid
 *             upon any elected Bfts increase
 *
 * // Variables: scalars
 * MecYear, MecMonth
 *     Consider months 0-1199 for:
 * test period beginning year, month
 * cum prem less deductible WDs
 * DCV
 *
 * // life contingencies
 * unit 7pp: rounding?
 * NSP
 */

