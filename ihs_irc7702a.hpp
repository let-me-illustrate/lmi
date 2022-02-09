// Internal Revenue Code section 7702A (MEC testing).
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

#ifndef ihs_irc7702a_hpp
#define ihs_irc7702a_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "mec_state.hpp"
#include "round_to.hpp"
#include "so_attributes.hpp"

#include <vector>

LMI_SO void TestIrc7702A();

// TAXATION !! TODO ?? Known defects:
//   need to handle withdrawals correctly;
//   7-pay premium strategy changes premium when spec amt changes;
//   should optionally calculate factors e.g. 7pp, NSP from first principles.

// Implicitly-declared special member functions do the right thing.

class Irc7702A final
{
  public:
    enum e_death_benefit_definition {e_death_benefit_7702A, e_specamt_7702A};

    Irc7702A
        (mcenum_defn_life_ins        a_DefnLifeIns
        ,mcenum_defn_material_change a_DefnMaterialChange
        ,bool                        a_IsSurvivorship
        ,mcenum_mec_avoid_method     a_AvoidMec
        ,bool                        a_Use7PPTable
        ,bool                        a_UseNSPTable
        ,std::vector<double> const&  a_SevenPPRateVec// TODO ?? TAXATION !! Assume table passed, for now.
        ,std::vector<double> const&  a_NSPVec        // TODO ?? TAXATION !! Assume table passed, for now.
        ,round_to<double>    const&  a_RoundNonMecPrem
// TAXATION !! Either use these arguments or eliminate them.
//      ,int                         a_IssueAge
//      ,double                      a_Face
//      ,mcenum_dbopt_7702           a_DBOpt
//      ,std::vector<double> const&  a_q
//      ,std::vector<double> const&  a_i
//      ,std::vector<double> const&  a_PremLoad
//      ,std::vector<double> const&  a_PerKCharge
//      ,std::vector<double> const&  a_PolFee
// TAXATION !! probably other arguments are needed for reproposals
        );
    ~Irc7702A() = default;

    void Initialize7702A
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
        );
    void UpdateBOY7702A
        (int a_PolicyYear
        );
    void UpdateBOM7702A
        (int a_PolicyMonth
        );
    bool UpdateEOM7702A();
    void Update1035Exch7702A
        (double& a_DeemedCashValue
        ,double  a_Net1035Amount
        ,double  a_Bft
        );
    double UpdatePmt7702A
        (double a_DeemedCashValue
        ,double a_Payment
        ,bool   a_ThisPaymentIsUnnecessary
        ,double a_TargetPrem
        ,double a_LoadTarget
        ,double a_LoadExcess
        ,double a_CashValue
        );
    double UpdateBft7702A
        (double  // a_DeemedCashValue // TAXATION !! Is this argument useful?
        ,double  a_NewDB
        ,double  a_OldDB
        ,bool    a_IsInCorridor
        ,double  a_NewSA
        ,double  a_OldSA
        ,double  a_CashValue
        );
    void InduceMaterialChange();
    void RedressMatChg
        (double& a_DeemedCashValue
        ,double  a_UnnecPrem
        ,double  a_NecPrem
        ,double  a_CashValue
        );

    bool IsMaterialChangeInQueue() const;
    double MaxNonMecPremium
        (double a_DeemedCashValue
        ,double a_TargetPrem
        ,double a_LoadTarget
        ,double a_LoadExcess
        ,double a_CashValue
        ) const;
    double MaxNecessaryPremium
        (double a_DeemedCashValue
        ,double a_TargetPrem
        ,double a_LoadTarget
        ,double a_LoadExcess
        ,double a_CashValue
        ) const;
    // TAXATION !! Consider using accessors like Irc7702::RoundedGLP()
    // to encapsulate rounding within this class.
    double GetPresent7pp() const        {return SevenPP;}
    bool IsMecAlready() const           {return IsMec;}

    mec_state const& state() {return state_;}

    // Share internals with routines that display monthly details
    // of account value calculations. These Debug- functions are all
    // const accessors that cannot change the internal state. They
    // are designed to be called only by monthly-trace routines, so they
    // are not designed to be elegant or fast. They should not be used
    // for other purposes. They may be changed or eliminated in future
    // versions. I don't regard them as part of the public interface,
    // although they physically happen to be so.
    // TODO ?? TAXATION !! IOW, 'friend' would be better?
    int     DebugGetTestDur         () const
        {return TestPeriodDur - 1;} // "- 1": already incremented
    double  DebugGet7ppRate         () const
        {return SevenPPRateVec[PolicyYear];}
    double  DebugGetNsp             () const
        {return Ax * LowestBft;}
    double  DebugGetLowestBft       () const
        {return LowestBft;}
    double  DebugGetSavedDCV        () const
        {return SavedDCV;}
    double  DebugGetNetMaxNecPm     () const
        {return NetNecessaryPrem;}
    double  DebugGetGrossMaxNecPm   () const
        {return GrossNecessaryPrem;}
    double  DebugGetUnnecPm         () const
        {return UnnecPrem;}
    double  DebugGetDbAdj           () const
        {return
            Saved7PPRate
            *   (SavedAVBeforeMatChg + SavedNecPrem)
            /   SavedNSP
            ;
        }
    double  DebugGet7pp             () const
        {return SevenPP;}
    double  DebugGetCumPmts         () const
        {return CumPmts;}
    double  DebugGetCum7pp          () const
        {return CumSevenPP;}
    bool    DebugGetIsMatChg        () const
        {return IsMatChg;}

  private:
    void    TestBftDecrease
        (double a_NewBft
        );
    void    Determine7PP
        (double a_Bft
        ,bool   a_TriggeredByBftDecrease
        ,bool   a_TriggeredByMatChg
        ,bool   a_TriggeredByUnnecPrem
        ,double a_AVBeforeMatChg
        ,double a_NecPrem
        );
    // TODO ?? TAXATION !! Wouldn't this need to be public?
    double  SAIncreaseToAvoidMec
        (bool a_TriggeredByUnnecPrem
        );
    double  DetermineLowestBft() const;

    mutable mec_state state_;

// NOTE: table lookup really means supplied via arguments ?
// TODO ?? TAXATION !! Need to calculate if not table lookup.

    mcenum_defn_life_ins DefnLifeIns;
    mcenum_defn_material_change DefnMaterialChange;

    bool UnnecPremIsMatChg;
    bool ElectiveIncrIsMatChg;
    bool CorrHidesIncr;

    bool const      IsSurvivorship; // is policy multilife and not first to die?
    mcenum_mec_avoid_method AvoidMec; // TODO ?? TAXATION !! Document what this does.
    bool const      Use7PPTable;    // get SevenPP from table, not calculation
    bool const      UseNSPTable;    // get NSP from table, not calculation

    std::vector<double> const&  SevenPPRateVec; // seven-pay premiums per $
    std::vector<double> const&  NSPVec;         // net single premiums per $

    round_to<double> RoundNonMecPrem;

    // Note that death benefit and pmts may not have their usual meanings.
    // TAXATION !! [Explain why.]
    // Pmts is premium net of deductible withdrawals. [TAXATION !! globally prefer "nontaxable" to "deductible"]
    // Bfts is SA, plus tax basis for ROP. The corridor is conservatively
    //   ignored to prevent the policy from becoming a MEC retroactively
    //   due to poor investment performance.
    // TODO ?? TAXATION !! Maybe not--Bfts might be DB.
    std::vector<double> Bfts;           // "death benefit" for 7702A
    std::vector<double> Pmts;           // premium net of deductible withdrawals
    double Ax;

    e_death_benefit_definition DBDefn;

    // Once either Ignore or IsMec is set, we stop MEC testing.
    // This saves some work, and there's no point in doing it anyway.
    // We implement this by testing the flags at entry in every function
    // in the public interface (other than the ctor and dtor). That is
    // sufficient because the private functions are called only by the
    // public functions.
    bool            Ignore;         // basis is not current, so ignore
    bool            IsMec;          // is contract a MEC
    bool            IsMatChg;       // is there a material change this month
    double          SevenPP;        // seven-pay premium in $
    double          CumSevenPP;     // cumulative seven-pay premium in $
    double          CumPmts;        // cumulative Pmts
    mutable double  LowestBft;      // lowest death bft in test period
    double          UnnecPrem;      // most recent unnecessary premium
    bool            UnnecPremPaid;  // was any unnec prem paid in test period?
    int             TestPeriodLen;  // length (months) of test period
    int             TestPeriodDur;  // duration (months) since beginning of
                                    //   current test period
    int             PolicyYear;     // full years since issue
    int             PolicyMonth;    // full months since policy anniversary
    double          AssumedBft;     // death bft assumed in setting last SevenPP

    // These values are saved at each material change, and used for
    // recalculating the seven pay premium in the event of a Bfts decrease.
    double          Saved7PPRate;   // seven-pay premium per $
    double          SavedAVBeforeMatChg;// account value prior to material change
    double          SavedNecPrem;   // maximum necessary premium
    double          SavedNSP;
    double          SavedDCV;       // deemed cash value

    // TAXATION !! Need these be 'mutable'?
    mutable double  NetNecessaryPrem;   // maximum necessary premium, no loads
    mutable double  GrossNecessaryPrem; // max nec prem grossed up for prem load

// TAXATION !! Is this stuff useful? q and i might be, but the others?
//  std::vector<double> const&  q;
//  std::vector<double> const&  i;
//  std::vector<double> const&  PremLoad;
//  std::vector<double> const&  PerKCharge;
//  std::vector<double> const&  PolFee;
};

#endif // ihs_irc7702a_hpp
