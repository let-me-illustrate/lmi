// Internal Revenue Code section 7702 (definition of life insurance).
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ihs_irc7702.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "commutation_functions.hpp"
#include "et_vector.hpp"
#include "materially_equal.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"

#include <algorithm>                    // max(), min(), reverse()
#include <numeric>                      // partial_sum()
#include <string>

// TAXATION !! Update this block comment, or simply delete it. The
// client-server model is important, but not predominantly so. It is
// no longer desirable to document basic C++ for C programmers here.
//
// The corridor factor may as well reside on the client system: it's
// just a constant vector of 101 numbers. We make it available for
// any client that wants us to supply it.
//
// This is a standard idiom for expressing a static object of user-
// defined type; it guarantees that the object will be initialized
// before its first use. C programmers who wonder why we don't just
// declare a static global variable are referred to Stroustrup,
// _The C++ Programming Language_, 3rd edition (Stroustrup3),
// section B.2.3, which notes that ISO standard C++ deprecates
// such usage of 'static'.
//
namespace
{
    std::vector<double> const& CompleteGptCorridor()
        {
        static int const n = 101;
        static double const d[n] =
            {
            //0     1     2     3     4     5     6     7     8     9
            2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 00
            2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 10
            2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 20
            2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, 2.50, // 30
            2.50, 2.43, 2.36, 2.29, 2.22, 2.15, 2.09, 2.03, 1.97, 1.91, // 40
            1.85, 1.78, 1.71, 1.64, 1.57, 1.50, 1.46, 1.42, 1.38, 1.34, // 50
            1.30, 1.28, 1.26, 1.24, 1.22, 1.20, 1.19, 1.18, 1.17, 1.16, // 60
            1.15, 1.13, 1.11, 1.09, 1.07, 1.05, 1.05, 1.05, 1.05, 1.05, // 70
            1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, // 80
            1.05, 1.04, 1.03, 1.02, 1.01, 1.00, 1.00, 1.00, 1.00, 1.00, // 90
            1.00                                                        // 100
            };
        static std::vector<double> const v(d, d + n);
        return v;
        }

    // Use 7702 int rate for DB discount in NAAR. TAXATION !! Does it
    // make sense to retain this?
    bool g_UseIcForIg = true;
} // Unnamed namespace.

// TAXATION !! General concerns
//
// TAXATION !! Explain why flat extras are generally ignored.
//
// TAXATION !! Support off-anniversary adjustment events, though not
// in illustrations.
//
// TAXATION !! Are forceouts signalled properly by the server? in illustrations?
//
// TAXATION !! Signal if gp limit becomes negative; always allow min pmt to keep
// contract in force.
//
// TAXATION !! What if maturity duration is beyond age 100?
//
// TAXATION !! Are riders adequately supported?
//
// TAXATION !! 7702(f)(7)(B-E)

// TAXATION !! Document these points in some more appropriate fashion:
// Server questions and answers:
// check each pmt? --no, admin system does that
// maintain current and last GSP, GLP --no, admin system does that
// who tracks cum pmt during year? --admin system
// who tracks cum pmt for prior years? --admin system
// return forceout; ... or return remaining GP limit? --admin system does that
// fractional duration? --admin system does that

//============================================================================
Irc7702::Irc7702
    (mcenum_defn_life_ins       a_Test7702
    ,int                        a_IssueAge
    ,int                        a_EndtAge
    ,std::vector<double> const& a_Qc
    ,std::vector<double> const& a_GLPic
    ,std::vector<double> const& a_GSPic
    ,std::vector<double> const& a_Ig
    ,std::vector<double> const& a_IntDed
    ,double                     a_PresentBftAmt
    ,double                     a_PresentSpecAmt
    ,double                     a_LeastBftAmtEver
    ,mcenum_dbopt_7702          a_PresentDBOpt
    ,std::vector<double> const& a_AnnChgPol
    ,std::vector<double> const& a_MlyChgPol
    ,std::vector<double> const& a_MlyChgSpecAmt
    ,double                     a_SpecAmtLoadLimit
    ,std::vector<double> const& a_MlyChgADD
    ,double                     a_ADDLimit
    ,std::vector<double> const& a_LoadTgt
    ,std::vector<double> const& a_LoadExc
    ,double                     a_TargetPremium
    ,round_to<double>    const& a_round_min_premium
    ,round_to<double>    const& a_round_max_premium
    ,round_to<double>    const& a_round_min_specamt
    ,round_to<double>    const& a_round_max_specamt
    ,int                        a_InforceYear
    ,int                        a_InforceMonth
    ,double                     a_InforceGLP
    ,double                     a_InforceCumGLP
    ,double                     a_InforceGSP
    ,double                     a_InforceCumPremsPaid
    )
    :Test7702           {a_Test7702}
    ,IssueAge           {a_IssueAge}
    ,EndtAge            {a_EndtAge}
    ,Qc                 {a_Qc}
    ,GLPic              {a_GLPic}
    ,GSPic              {a_GSPic}
    ,Ig                 {a_Ig}
    ,IntDed             {a_IntDed}
    ,PresentBftAmt      {a_PresentBftAmt}
    ,PriorBftAmt        {a_PresentBftAmt}
    ,PresentSpecAmt     {a_PresentSpecAmt}
    ,PriorSpecAmt       {a_PresentSpecAmt}
    ,LeastBftAmtEver    {a_LeastBftAmtEver}
    ,PresentDBOpt       {a_PresentDBOpt}
    ,PriorDBOpt         {a_PresentDBOpt}
    ,AnnChgPol          {a_AnnChgPol}
    ,MlyChgPol          {a_MlyChgPol}
    ,MlyChgSpecAmt      {a_MlyChgSpecAmt}
    ,SpecAmtLoadLimit   {a_SpecAmtLoadLimit}
    ,MlyChgADD          {a_MlyChgADD}
    ,ADDLimit           {a_ADDLimit}
    ,LoadTgt            {a_LoadTgt}
    ,LoadExc            {a_LoadExc}
    ,TargetPremium      {a_TargetPremium}
    ,round_min_premium  {a_round_min_premium}
    ,round_max_premium  {a_round_max_premium}
    ,round_min_specamt  {a_round_min_specamt}
    ,round_max_specamt  {a_round_max_specamt}
    ,InforceYear        {a_InforceYear}
    ,InforceMonth       {a_InforceMonth}
    ,InforceGLP         {a_InforceGLP}
    ,InforceCumGLP      {a_InforceCumGLP}
    ,InforceGSP         {a_InforceGSP}
    ,InforceCumPremsPaid{a_InforceCumPremsPaid}
    ,PresentGLP         {a_InforceGLP}
    ,PriorGLP           {a_InforceGLP}
    ,CumGLP             {a_InforceCumGLP}
    ,PresentGSP         {a_InforceGSP}
    ,PriorGSP           {a_InforceGSP}
    ,GptLimit           {std::max(a_InforceCumGLP, a_InforceGSP)}
    ,CumPmts            {a_InforceCumPremsPaid}
{
    LMI_ASSERT(a_PresentSpecAmt  <= a_PresentBftAmt );
    LMI_ASSERT(a_LeastBftAmtEver <= a_PresentSpecAmt);
    LMI_ASSERT(0.0 <= a_TargetPremium);
    // TAXATION !! Wrong for a contract in force one day. When this is
    // reimplemented, use 'effective date == inforce date' instead.
    if(0 == InforceYear && 0 == InforceMonth)
        {
        LMI_ASSERT(0.0 == PresentGLP);
        LMI_ASSERT(0.0 == PriorGLP  );
        LMI_ASSERT(0.0 == CumGLP    );
        LMI_ASSERT(0.0 == PresentGSP);
        LMI_ASSERT(0.0 == PriorGSP  );
        LMI_ASSERT(0.0 == GptLimit  );
        LMI_ASSERT(0.0 == CumPmts   );
        }
    Init();
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor is not
/// implemented inside the class definition, because the header
/// forward-declares one or more classes that are held by
/// std::unique_ptr, so their destructors are visible only here.

Irc7702::~Irc7702() = default;

//============================================================================
void Irc7702::ProcessGptPmt
    (int     a_Duration
    ,double& a_Pmt
    )
{
// TAXATION !! Should Duration somehow be used for verification? and
// if so, should it be maintained internally in this class?
    if(mce_gpt != Test7702)
        {
        return;
        }
    // This assertion seems like a good idea:
//    LMI_ASSERT(CumPmts <= GptLimit);
    // but it's incorrect when this function is called by
    // AccountValue::TxTakeWD(): in that case, the cumulative GLP
    // has been improperly updated by calling UpdateBOY7702() before
    // ProcessAdjustableEvent().
    if(GptLimit < CumPmts)
        {
        warning()
            << "Apparent GPT violation before applying a "
            << value_cast<std::string>(a_Pmt)
            << " payment at duration " << a_Duration << ":"
            << "\n  " << value_cast<std::string>(GptLimit) << " limit is less than"
            << "\n  " << value_cast<std::string>(CumPmts ) << " cumulative premiums paid."
            << LMI_FLUSH
            ;
        }
    a_Pmt = std::min(a_Pmt ,round_max_premium(GptLimit - CumPmts));
    CumPmts += a_Pmt;
}

//============================================================================
// TAXATION !! Update this block comment.
// Adjustable events processed here:
//  actual changes in DB arising from:
//      changes in SA
//      deductible (TODO ?? section 72--not supported here) WD
//      ROP DB changed by prem (TODO ?? not supported yet) or deductible WD
//  changes in DBOpt
// Other adjustable events such as
//  reduction in substandard charge
//  liberalization of current charges
//  discovery of misstatement of age or gender
// are not treated because they are problematic or extraordinary. And
//  changes in QABs
// are not treated: WP is ignored, ADD is assumed not to change, term
// rider is assumed not to pose any problem, and it is assumed that no
// other QAB exists. This is correct only for term riders that are
// integrated with the base to conserve a total amount, that cannot be
// increased or decreased, and that continue unconditionally to the
// next monthiversary even when funds are insufficient to pay the term
// rider deduction.
//
//  A = guideline premium before change
//  B = guideline premium at attained age for new SA and new DBO
//  C = guideline premium at attained age for old SA and old DBO
//  New guideline premium = A + B - C
//
void Irc7702::ProcessAdjustableEvent
    (int               a_Duration
    ,double            a_NewBftAmt
    ,double            a_PriorBftAmt
    ,double            a_NewSpecAmt
    ,double            a_PriorSpecAmt
    ,mcenum_dbopt_7702 a_NewDBOpt
    ,mcenum_dbopt_7702 a_PriorDBOpt
    ,double            a_TargetPremium
    )
{
    LMI_ASSERT(a_PriorSpecAmt <= a_PriorBftAmt);
// We do not assert this:
//  LMI_ASSERT(materially_equal(PresentBftAmt, a_PriorBftAmt));
// because a_PriorBftAmt is now DB as of the beginning of the current day,
// before any transactions are applied, which is not necessarily the same
// as DB as of the last adjustment event.
    LMI_ASSERT(materially_equal(PresentSpecAmt, a_PriorSpecAmt));
    LMI_ASSERT(PresentDBOpt == a_PriorDBOpt);
    LMI_ASSERT(0.0 <= a_TargetPremium);
    // TAXATION !! Update this block comment:
    // Should be called only when something actually changed: either
    //   dbopt changed; or
    //   specamt changed, causing an actual change in bftamt.
    //   TAXATION !! ...or bft amt, if 7702 DB is defined as that
    // For illustrations only, we assume:
    //   no changes in ratings
    //   no liberalizations in charges
    //   no changes in QABs, except term
    //   integrated term rider treated as DB; early termination changes DB
    bool adj_event =
            (
                !materially_equal(a_NewSpecAmt, a_PriorSpecAmt)
            &&  !materially_equal(a_NewBftAmt, a_PriorBftAmt)
            )
        ||  a_NewDBOpt != a_PriorDBOpt
        ;
    LMI_ASSERT(adj_event);

    // Post target and {BftAmt, SpecAmt, DBOpt} changes to local state.
    PriorBftAmt     = PresentBftAmt;
    PresentBftAmt   = a_NewBftAmt;
    PriorSpecAmt    = PresentSpecAmt;
    PresentSpecAmt  = a_NewSpecAmt;
    PriorDBOpt      = PresentDBOpt;
    PresentDBOpt    = a_NewDBOpt;
    TargetPremium   = a_TargetPremium;

    // Apply A + B - C method for both GLP and GSP.

// TAXATION !! The name should certainly be changed; should the
// old behavior be retained conditionally?
// We changed our interpretation, but it'd be nice to preserve
// the old functionality, conditional on a behavior flag. And
// the name is poor: shouldn't it just be 'EndowmentBenefit'?
//
// TAXATION !! lmi seems to track the lowest benefit since the
// issue date and use that value for the endowment benefit of
// A, B, and C, which conflicts with '7702.html' [4/8].
    LeastBftAmtEver = std::min(LeastBftAmtEver, a_NewBftAmt);

    double b_level = CalculateGLP
        (a_Duration
        ,PresentBftAmt
        ,PresentSpecAmt
        ,PresentSpecAmt // TAXATION !! LeastBftAmtEver
        ,PresentDBOpt
        );
    double c_level = CalculateGLP
        (a_Duration
        ,PriorBftAmt
        ,PriorSpecAmt
        ,PriorSpecAmt // TAXATION !! LeastBftAmtEver
        ,PriorDBOpt
        );
    PriorGLP = PresentGLP;
    PresentGLP = PriorGLP + b_level - c_level;

    double b_single = CalculateGSP
        (a_Duration
        ,PresentBftAmt
        ,PresentSpecAmt
        ,PresentSpecAmt // TAXATION !! LeastBftAmtEver
        );
    double c_single = CalculateGSP
        (a_Duration
        ,PriorBftAmt
        ,PriorSpecAmt
        ,PriorSpecAmt // TAXATION !! LeastBftAmtEver
        );
    PriorGSP = PresentGSP;
    PresentGSP = PriorGSP + b_single - c_single;
// Test for negative guideline...then do what if negative?
// --refer to actuarial department (see specs)
// TAXATION !! shouldn't that be handled here?
    CumGLP = CumGLP + PresentGLP - PriorGLP; // TAXATION !! Eh? Because prior GLP was already added?
    GptLimit = std::max(CumGLP, PresentGSP);
}

//============================================================================
double Irc7702::Forceout()
{
    // Return forceout amount, if any, and assume that the caller
    // disburses it to maintain compliance with the guideline limit.
    if(GptLimit < CumPmts)
        {
        double forceout = round_min_premium(CumPmts - GptLimit);
        CumPmts -= forceout;
        return forceout;
        }
    else
        {
        return 0.0;
        }
}

//============================================================================
void Irc7702::Init()
{
    Length = lmi::ssize(Qc);
    LMI_ASSERT(IssueAge <= EndtAge);
    LMI_ASSERT(            EndtAge <= 100);
    LMI_ASSERT(Length == EndtAge - IssueAge);

    // TAXATION !! "For now": a decade has passed...
    // For now, always perform both GPT and CVAT calculations.
    // GLP might be wanted for some purpose in a CVAT product.
    // The extra overhead is not enormous.

    InitCommFns();
    InitCorridor();
    InitPvVectors(Opt1Int4Pct);
    InitPvVectors(Opt2Int4Pct);
    InitPvVectors(Opt1Int6Pct);
    // TODO ?? We can delete the commutation functions here, rather than in
    // the dtor, to save some space. We defer doing so until the
    // program is complete. TAXATION !! It would be better not to use pointers--see header.
}

//============================================================================
void Irc7702::InitCommFns()
{
    std::vector<double> glp_naar_disc_rate;
    std::vector<double> gsp_naar_disc_rate;
    std::vector<double> const zero(Length, 0.0);

    // g_UseIcForIg indicates whether the 7702 rates should be used for the NAAR
    // discount factor. We interpret a guar rate (IG) of zero in all years as
    // no NAAR discount factor.

    if(!g_UseIcForIg)
        {
        // if the flag is not set, use guar rates for NAAR discount factor
        glp_naar_disc_rate = Ig;
        gsp_naar_disc_rate = Ig;
        }
    else if(zero == Ig)
        {
        // if guar rate is zero, we will always use it for the NAAR discount factor
        glp_naar_disc_rate = Ig;
        gsp_naar_disc_rate = Ig;
        }
    else
        {
        // if the flag is true, and the guar rate !=0, use the 7702 rates for
        // the NAAR discount factor
        glp_naar_disc_rate = GLPic;
        gsp_naar_disc_rate = GSPic;
        }

    // Commutation functions using min i = iglp(): both options 1 and 2
    CommFns[Opt1Int4Pct].reset
        (::new ULCommFns
            (Qc
            ,GLPic
            ,glp_naar_disc_rate
            ,mce_option1_for_7702
            ,mce_monthly
            )
        );
    DEndt[Opt1Int4Pct] = CommFns[Opt1Int4Pct]->aDomega();

    CommFns[Opt2Int4Pct].reset
        (::new ULCommFns
            (Qc
            ,GLPic
            ,glp_naar_disc_rate
            ,mce_option2_for_7702
            ,mce_monthly
            )
        );
    DEndt[Opt2Int4Pct] = CommFns[Opt2Int4Pct]->aDomega();

    // Commutation functions using min i = igsp(): always option 1
    CommFns[Opt1Int6Pct].reset
        (::new ULCommFns
            (Qc
            ,GSPic
            ,gsp_naar_disc_rate
            ,mce_option1_for_7702
            ,mce_monthly
            )
        );
    DEndt[Opt1Int6Pct] = CommFns[Opt1Int6Pct]->aDomega();
}

/// Set GPT and CVAT corridor factors respecting IssueAge.
///
/// The GPT corridor is prescribed by statute.
///
/// The CVAT corridor is calculated as the reciprocal of NSP:
///   1 / NSP = Dx / (Mx + Domega)
/// Consistent with '7702.html' [14.2] and Eckley's paper cited there,
/// D is "annual", and M is "monthly", in the sense that "monthly"
/// functions are "annual" times Eckley's "a''(12)" [his eq. 28]
/// because UL mortality charges are assessed on a monthly basis.

void Irc7702::InitCorridor()
{
    // TODO ?? Substandard: set last NSP to 1.0? ignore flats? set NSP[omega] to 1?
    // TAXATION !! --better to ignore susbstandard

    CvatCorridor.resize(Length);
    CvatCorridor +=
           CommFns[Opt1Int4Pct]->aD()
        / (CommFns[Opt1Int4Pct]->kM() + DEndt[Opt1Int4Pct])
        ;

    GptCorridor.assign
        (CompleteGptCorridor().begin() + IssueAge
        ,CompleteGptCorridor().begin() + EndtAge
        );
}

/// Initialize present-value vectors: '7702.html' [14].
///
/// kD * MlyChg implies k == mly; it would be more general to say
/// "modal" instead. But that's still not perfectly general, because
/// we may need commutation functions on more than one non-annual
/// mode. For instance, a policy might deduct the policy fee monthly
/// but the account value load daily. Any specific changes like that
/// are straightforward, but we don't want to spend time calculating
/// functions on every conceivable mode unless we're actually going
/// to use them.
///
/// TAXATION !! Eliminate aliasing references.
/// TAXATION !! Rename '[46]Pct' to 'g[ls]p'.
/// TAXATION !! Write a utility function for rotate-partial_sum_rotate.
/// TAXATION !! Add unit tests.

void Irc7702::InitPvVectors(EIOBasis const& a_EIOBasis)
{
    // We may need to recalculate these every year for a
    // survivorship policy, depending on how its account
    // value accumulation is specified.

    ULCommFns const& comm_fns = *CommFns[a_EIOBasis];

    // Present value of charges per policy

    LMI_ASSERT(Length == lmi::ssize(AnnChgPol));
    LMI_ASSERT(Length == lmi::ssize(LoadExc));
    LMI_ASSERT(Length == lmi::ssize(LoadTgt));
    LMI_ASSERT(Length == lmi::ssize(MlyChgADD));
    LMI_ASSERT(Length == lmi::ssize(MlyChgPol));
    LMI_ASSERT(Length == lmi::ssize(MlyChgSpecAmt));
    LMI_ASSERT(Length == lmi::ssize(comm_fns.aD()));
    LMI_ASSERT(Length == lmi::ssize(comm_fns.kC()));
    LMI_ASSERT(Length == lmi::ssize(comm_fns.kD()));

    std::vector<double>& chg_pol = PvChgPol[a_EIOBasis];
    chg_pol.resize(Length);
    chg_pol += AnnChgPol * comm_fns.aD() + MlyChgPol * comm_fns.kD();

    // ET !! This is just APL written verbosely in a funny C++ syntax.
    // Perhaps we could hope for an expression-template library to do this:
    //   chg_pol = chg_pol.reverse().partial_sum().reverse();
    // but that would require a special type: 'chg_pol' couldn't be a
    // std::vector anymore.
    std::reverse(chg_pol.begin(), chg_pol.end());
    std::partial_sum(chg_pol.begin(), chg_pol.end(), chg_pol.begin());
    std::reverse(chg_pol.begin(), chg_pol.end());

    // Present value of charges per $1 specified amount

    // APL: chg_sa gets rotate plus scan rotate MlyChgSpecAmt times kD
    std::vector<double>& chg_sa = PvChgSpecAmt[a_EIOBasis];
    chg_sa.resize(Length);
    chg_sa += MlyChgSpecAmt * comm_fns.kD();
    std::reverse(chg_sa.begin(), chg_sa.end());
    std::partial_sum(chg_sa.begin(), chg_sa.end(), chg_sa.begin());
    std::reverse(chg_sa.begin(), chg_sa.end());

    // APL: chg_add gets rotate plus scan rotate MlyChgADD times kD
    std::vector<double>& chg_add = PvChgADD[a_EIOBasis];
    chg_add.resize(Length);
    chg_add += MlyChgADD * comm_fns.kD();
    std::reverse(chg_add.begin(), chg_add.end());
    std::partial_sum(chg_add.begin(), chg_add.end(), chg_add.begin());
    std::reverse(chg_add.begin(), chg_add.end());

    // APL: chg_mort gets rotate plus scan rotate kC
    std::vector<double>& chg_mort = PvChgMort[a_EIOBasis];
    chg_mort = comm_fns.kC();
    std::reverse(chg_mort.begin(), chg_mort.end());
    std::partial_sum(chg_mort.begin(), chg_mort.end(), chg_mort.begin());
    std::reverse(chg_mort.begin(), chg_mort.end());

    // Present value of 1 - target premium load

    std::vector<double>& npf_sgl_tgt = PvNpfSglTgt[a_EIOBasis];
    npf_sgl_tgt.resize(Length);
    npf_sgl_tgt += (1.0 - LoadTgt) * comm_fns.aD();

    std::vector<double>& npf_lvl_tgt = PvNpfLvlTgt[a_EIOBasis];
    npf_lvl_tgt = npf_sgl_tgt;
    std::reverse(npf_lvl_tgt.begin(), npf_lvl_tgt.end());
    std::partial_sum(npf_lvl_tgt.begin(), npf_lvl_tgt.end(), npf_lvl_tgt.begin());
    std::reverse(npf_lvl_tgt.begin(), npf_lvl_tgt.end());

    // Present value of 1 - excess premium load

    std::vector<double>& npf_sgl_exc = PvNpfSglExc[a_EIOBasis];
    npf_sgl_exc.resize(Length);
    npf_sgl_exc += (1.0 - LoadExc) * comm_fns.aD();

    std::vector<double>& npf_lvl_exc = PvNpfLvlExc[a_EIOBasis];
    npf_lvl_exc = npf_sgl_exc;
    std::reverse(npf_lvl_exc.begin(), npf_lvl_exc.end());
    std::partial_sum(npf_lvl_exc.begin(), npf_lvl_exc.end(), npf_lvl_exc.begin());
    std::reverse(npf_lvl_exc.begin(), npf_lvl_exc.end());
}

/// For illustrations, we can't initialize everything in the ctor.
///
/// For instance, specamt might need to be calculated as a function
/// of GLP or GSP, so it cannot always be known before the GPT
/// calculations are available; and guideline premiums cannot be
/// determined until specamt is set. Therefore, we need this function
/// to initialize these things after specamt has been set. The server
/// doesn't use it.
///
/// Furthermore, cumulative values must be reinitialized between solve
/// iterations, and this function is the right place to do that.

void Irc7702::Initialize7702
    (double            a_BftAmt
    ,double            a_SpecAmt
    ,mcenum_dbopt_7702 a_DBOpt // TODO ?? TAXATION !! Is there any reason why dbopt would be changed here?
    ,double            a_TargetPremium
    )
{
    LMI_ASSERT(a_SpecAmt <= a_BftAmt);
    LMI_ASSERT(0.0 <= a_TargetPremium);
    PresentDBOpt        = a_DBOpt;
    PriorDBOpt          = PresentDBOpt;
    PresentSpecAmt      = a_SpecAmt;
    PriorSpecAmt        = PresentSpecAmt;
    PresentBftAmt       = a_BftAmt;
    PriorBftAmt         = PresentBftAmt;
// TAXATION !! This:
//  LeastBftAmtEver     = PresentBftAmt;
// would appear correct: ...BftAmt assigned from ...BftAmt. However,
// as pointed out above, 'EndowmentBenefit' would be a better name,
// so initializing it to PresentSpecAmt as elsewhere is actually
// correct.
    LeastBftAmtEver     = PresentSpecAmt;
    TargetPremium       = a_TargetPremium;

    // TAXATION !! Wrong for a contract in force one day. When this is
    // reimplemented, use 'effective date == inforce date' instead.
    if(0 == InforceYear && 0 == InforceMonth)
        {
        PresentGLP = CalculateGLP
            (0
            ,PresentBftAmt
            ,PresentSpecAmt
            ,LeastBftAmtEver
            ,PresentDBOpt
            );
        PriorGLP = PresentGLP;
        PresentGSP = CalculateGSP
            (0
            ,PresentBftAmt
            ,PresentSpecAmt
            ,LeastBftAmtEver
            );
        PriorGSP = PresentGSP;
        CumGLP     = 0.0;
        GptLimit   = std::max(CumGLP, PresentGSP);
        CumPmts    = 0.0;
        }
    else
        {
        // TAXATION !! Revisit this.
        // TODO ?? None of this should be necessary, but this function
        // is called for every basis, though it probably should be
        // called only once, for basis 'mce_run_gen_curr_sep_full'.
        // For other bases, many GPT data are not updated; e.g.,
        // GLP isn't changed, so cumulative GLP isn't correct even
        // though it's correctly initialized here. If this block can
        // be expunged, then the 'Inforce*' members may be unneeded.
        PresentGLP = InforceGLP;
        PriorGLP   = PresentGLP;
        CumGLP     = InforceCumGLP;
        PresentGSP = InforceGSP;
        PriorGSP   = PresentGSP;
        GptLimit   = std::max(CumGLP, PresentGSP);
        CumPmts    = InforceCumPremsPaid;
        }
}

//============================================================================
void Irc7702::UpdateBOY7702()
{
    // Update guideline limits.
    // TODO ?? This assumes no off-anniversary change. That will probably
    // be OK because illustration systems confine most transactions
    // to anniversaries and we envision that an admin system client
    // will do this calculation itself.
    // TAXATION !! Rewrite that comment. It is true that off-anniversary
    // changes aren't handled at present, but the work done here would
    // never be done at any other time.
    CumGLP += PresentGLP;
    GptLimit = std::max(CumGLP, PresentGSP);
}

//============================================================================
std::vector<double> const& Irc7702::Corridor() const
{
    // The 7702 test might indeed be neither CVAT nor GPT for a non-US
    // contract, but in that case this code shouldn't be reached.
    if(mce_gpt == Test7702)
        {
        return GptCorridor;
        }
    else if(mce_cvat == Test7702)
        {
        return CvatCorridor;
        }
    else
        {
        alarum() << "7702 test is neither GPT nor CVAT." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

//============================================================================
Irc7702::EIOBasis Irc7702::Get4PctBasis
    (mcenum_dbopt_7702 a_DBOpt
    )
{
    switch(a_DBOpt)
        {
        case mce_option1_for_7702: return Opt1Int4Pct;
        case mce_option2_for_7702: return Opt2Int4Pct;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
double Irc7702::CalculateGLP
    (int               a_Year
    ,double            a_BftAmt
    ,double            a_SpecAmt
    ,double            a_LeastBftAmtEver
    ,mcenum_dbopt_7702 a_DBOpt
    ) const
{
    LMI_ASSERT(a_SpecAmt <= a_BftAmt);
    return CalculatePremium
        (Get4PctBasis(a_DBOpt)
        ,a_Year
        ,a_BftAmt
        ,a_SpecAmt
        ,a_LeastBftAmtEver
        ,PvNpfLvlTgt[Get4PctBasis(a_DBOpt)][a_Year]
        ,PvNpfLvlExc[Get4PctBasis(a_DBOpt)][a_Year]
        ,TargetPremium
        );
}

//============================================================================
double Irc7702::CalculateGSP
    (int    a_Year
    ,double a_BftAmt
    ,double a_SpecAmt
    ,double a_LeastBftAmtEver
    ) const
{
    LMI_ASSERT(a_SpecAmt <= a_BftAmt);
    return CalculatePremium
        (Opt1Int6Pct
        ,a_Year
        ,a_BftAmt
        ,a_SpecAmt
        ,a_LeastBftAmtEver
        ,PvNpfSglTgt[Opt1Int6Pct][a_Year]
        ,PvNpfSglExc[Opt1Int6Pct][a_Year]
        ,TargetPremium
        );
}

/// Calculate a guideline premium.
///
/// This function encompasses both GLP and GSP. It is designed to have
/// no side effects, and to depend only on its arguments and on data
/// members that are set in the ctor and not subsequently changed--so
/// it's safe for FindSpecAmt::operator()() to call it iteratively.

double Irc7702::CalculatePremium
    (EIOBasis const& a_EIOBasis
    ,int             a_Year
    ,double          a_BftAmt
    ,double          a_SpecAmt
    ,double          a_LeastBftAmtEver
    ,double          a_NetPmtFactorTgt
    ,double          a_NetPmtFactorExc
    ,double          a_TargetPremium
    ) const
{
    LMI_ASSERT(a_SpecAmt <= a_BftAmt);
    LMI_ASSERT(0.0 != a_NetPmtFactorTgt);
    LMI_ASSERT(0.0 != a_NetPmtFactorExc);

    // TAXATION !! Deal with this:
    // TODO ?? This implementation is correct only if target premium
    // is fixed forever at issue; otherwise, distinct target premiums
    // must be passed for each of the quantities A, B, and C. Should
    // those targets be calculated for status x+[t], or x+t? (The
    // latter is okay if the B and C contracts in the A+B-C formula
    // are deemed to have current issue dates, but the former is more
    // consistent with the way durational loads are treated here.)
    double z =
        (   DEndt[a_EIOBasis] * a_LeastBftAmtEver
        +   PvChgPol[a_EIOBasis][a_Year]
        +   std::min(SpecAmtLoadLimit, a_SpecAmt) * PvChgSpecAmt[a_EIOBasis][a_Year]
        +   std::min(ADDLimit, a_SpecAmt) * PvChgADD[a_EIOBasis][a_Year]
        +   a_BftAmt * PvChgMort[a_EIOBasis][a_Year]
        )
        /
        a_NetPmtFactorTgt
        ;
    if(z <= a_TargetPremium)
        {
        return z;
        }

    return
        (   DEndt[a_EIOBasis] * a_LeastBftAmtEver
        +   PvChgPol[a_EIOBasis][a_Year]
        +   std::min(SpecAmtLoadLimit, a_SpecAmt) * PvChgSpecAmt[a_EIOBasis][a_Year]
        +   std::min(ADDLimit, a_SpecAmt) * PvChgADD[a_EIOBasis][a_Year]
        +   a_BftAmt * PvChgMort[a_EIOBasis][a_Year]
        +       a_TargetPremium
        // TAXATION !! Is this right? '7702.html' [14.3/5] says:
        // + TgtPrem[t] * (LoadTgt[t] - LoadExc[t]) * aD[t]
        // so perhaps the sign has been reversed.
            *   (a_NetPmtFactorExc - a_NetPmtFactorTgt)
        )
        /
        a_NetPmtFactorExc
        ;
}

// TAXATION !! Deal with this stuff.
// Nothing past this point is intended for use by the GPT server

// General concerns outside the scope of the standalone server
//
// combine txs on same date TAXATION !! isn't that up to the server?
//
// track cum pmts less wds and forceouts TAXATION !! is it necessary
// to add code here to accumulate those debits?
//
// current mort for substd TAXATION !! is that outside the scope of this code?
//
// set SA at issue to reflect dumpins and 1035s TAXATION !! That could be
// done for option two, but is probably a mistake.
//
// naming--SA vs SpecAmt, etc. TAXATION !! a grand renaming is desirable
//

/*
//============================================================================
void Irc7702::InitSevenPayPrem()
{
        // 7PP = MO / (N0-N7) (limit 7 to maturity year)
        // TAXATION !! add flat extras to 7PP?
        double denom = CFFourPctMin->N()[j];
        if((7 + j) < lmi::ssize(q))
            {
            denom -= CFFourPctMin->N()[7 + j];
            }
        LMI_ASSERT(0.0 != denom);
        mep_rate[j] =
            (   CFFourPctMin->M()[j]
            /   denom
            );
        }
}
*/

//============================================================================
double Irc7702::GetLeastBftAmtEver() const
{
    return LeastBftAmtEver;
}

//============================================================================
double Irc7702::RoundedGLP() const
{
    return round_max_premium(PresentGLP);
}

//============================================================================
double Irc7702::RoundedGSP() const
{
    return round_max_premium(PresentGSP);
}

/// Trivial accessor for monthly trace.

double Irc7702::glp() const
{
    return PresentGLP;
}

/// Trivial accessor for monthly trace.

double Irc7702::cum_glp() const
{
    return CumGLP;
}

/// Trivial accessor for monthly trace.

double Irc7702::gsp() const
{
    return PresentGSP;
}

/// Trivial accessor for monthly trace.

double Irc7702::premiums_paid() const
{
    return CumPmts;
}

// TAXATION !! TODO ?? This should be a separate, standalone unit test.
#if 0

#include "ihs_timer.hpp"

#include <iomanip>
#include <iostream>

int main()
{
// TAXATION !! Update or remove these timings.
// timing to construct Irc7702:
// RW: about 37 msec
// OS: about 93 msec; about 41 if we disable index checking
//   in std::vector operator[]()

// SOA table 120: "1980 CSO 50% Male Age nearest"
    std::vector<double>q
        {
         .00354,.00097,.00091,.00089,.00085,.00083,.00079,.00077,.00073,.00072,
         .00071,.00072,.00078,.00087,.00097,.00110,.00121,.00131,.00139,.00144,
         .00148,.00149,.00150,.00149,.00149,.00147,.00147,.00146,.00148,.00151,
         .00154,.00158,.00164,.00170,.00179,.00188,.00200,.00214,.00231,.00251,
         .00272,.00297,.00322,.00349,.00375,.00406,.00436,.00468,.00503,.00541,
         .00583,.00630,.00682,.00742,.00807,.00877,.00950,.01023,.01099,.01181,
         .01271,.01375,.01496,.01639,.01802,.01978,.02164,.02359,.02558,.02773,
         .03016,.03296,.03629,.04020,.04466,.04955,.05480,.06031,.06606,.07223,
         .07907,.08680,.09568,.10581,.11702,.12911,.14191,.15541,.16955,.18445,
         .20023,.21723,.23591,.25743,.28381,.32074,.37793,.47661,.65644,1.0000,
        };

    std::vector<double>i            (100, 0.07);
    std::vector<double>LoadTgt      (100, 0.05);
    std::vector<double>MlyChgSpecAmt(100, 0.00);
    std::vector<double>MlyChgADD    (100, 0.00);
    std::vector<double>PolFee       (100, 5.00);

    Timer timer;

    Irc7702* Irc7702_ = ::new Irc7702
        (CVAT
        ,45
        ,100000.0
        ,Option1
        ,q
        ,i
        ,LoadTgt
        ,MlyChgSpecAmt
        ,10000000.0
        ,MlyChgADD
        ,10000000.0
        ,PolFee
        );
    std::cout << timer.stop().elapsed_msec_str();
    delete Irc7702_;
}
#endif // 0
