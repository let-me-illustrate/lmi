// Rates and unprojected values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: basic_values.hpp,v 1.6 2005-08-22 15:35:53 chicares Exp $

#ifndef basic_values_hpp
#define basic_values_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "round_to.hpp"
#include "xenumtypes.hpp"
#include "xrangetypes.hpp"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

// Let's do this through once for UL, then think about extending it to other
// forms. At this time I'm thinking of term, ISWL, and par WL as well as UL.
// For instance, term might need mortality rates, but not loads or interest
// rates. For now, I'll be happy enough if we can isolate the essential
// building blocks so that we can make these other things easily when we want.
//
// Any of these can be made into a multilife form mainly by changing the way
// mortality rates are set. That seems to require inserting a virtual function
// that we can override. Another thought is to use a class template, so that
// we could specify what collection of objects we want, and let the compiler
// build the class for us.

class death_benefits;
class FundData;
class InputParms;
class InterestRates;
class Irc7702;
class Irc7702A;
class Loads;
class MortalityRates;
class Outlay;
class stratified_charges;
class SurrChgRates;
class TDatabase;
class TProductData;
class rounding_rules;

// See GetTable(). This idea may be poor, but we're OK as long as we
// don't use MustBlend. Here's the problem. The MustBlend case uses
// the input male % and nonsmoker %. But we might really want to use
// a predetermined % instead, like 80% for guar COI. Especially in
// that case, we're better off not creating a blended table because
// NAIC has published blended tables.
enum EBlend
    {
    CannotBlend,
    CanBlend,
    MustBlend
    };

class LMI_EXPIMP BasicValues
{
  public:
    BasicValues();
    BasicValues(InputParms const* input);
    // This signature is preferable to the version that takes a const*;
    // for now, it's used only by the lmi branch.
    BasicValues(InputParms const& input);
    BasicValues // GPT server only.
        (std::string  const& a_ProductName
        ,e_gender     const& a_Gender
        ,e_class      const& a_UnderwritingClass
        ,e_smoking    const& a_Smoker
        ,int                 a_IssueAge
        ,e_uw_basis   const& a_UnderwritingBasis
        ,e_state      const& a_StateOfJurisdiction
        ,double              a_FaceAmount
        ,e_dbopt_7702 const& a_DBOpt
        ,bool                a_ADDInForce
        ,double              a_TargetPremium
        );
    virtual ~BasicValues();

    void Init();
    void GPTServerInit();

    int                   GetLength()                 const;
    int                   GetIssueAge()               const;
    int                   GetRetAge()                 const;
    e_ledger_type  const& GetLedgerType()             const;
    e_state        const& GetStateOfJurisdiction()    const;
    e_state        const& GetStateOfDomicile()        const;
    double                GetPremTaxRate()            const;
    double                GetLowestPremTaxRate()      const;
    double                GetDomiciliaryPremTaxRate() const;
    rounding_rules const& GetRoundingRules()          const;
    double                InvestmentManagementFee()   const;

    boost::shared_ptr<InputParms const>   Input_;
    boost::shared_ptr<TProductData>       ProductData_;
    boost::shared_ptr<TDatabase>          Database_;
    boost::shared_ptr<FundData>           FundData_;
    boost::shared_ptr<rounding_rules>     RoundingRules_;
    boost::shared_ptr<stratified_charges> TieredCharges_;
    boost::shared_ptr<MortalityRates>     MortalityRates_;
    boost::shared_ptr<InterestRates>      InterestRates_;
    boost::shared_ptr<SurrChgRates>       SurrChgRates_;
    boost::shared_ptr<death_benefits>     DeathBfts_;
    boost::shared_ptr<Outlay>             Outlay_;
    boost::shared_ptr<Loads>              Loads_;
    boost::shared_ptr<Irc7702>            Irc7702_;
    boost::shared_ptr<Irc7702A>           Irc7702A_;

    double GetTgtPrem
        (int            Year
        ,double         SpecAmt
        ,e_dbopt const& DBOpt
        ,e_mode  const& Mode
        ) const;

    std::vector<double> const& GetCorridorFactor() const;
    e_dbopt_7702 const Get7702EffectiveDBOpt(e_dbopt const& a_DBOpt) const;
    e_dbopt const Get7702EffectiveDBOpt(e_dbopt_7702 const& a_DBOpt) const;
    std::vector<double> const& SpreadFor7702() const;
    std::vector<double> const& GetMly7702iGlp() const;
    std::vector<double> const& GetMly7702qc() const;

    // COI and term rates are blended on the current basis, but not
    // the guaranteed basis. Midpoint rates reflect blending, as a
    // consequence.

    std::vector<double> GetCvatCorridorFactors() const;
    std::vector<double> GetCurrCOIRates0()   const;
    std::vector<double> GetCurrCOIRates1()   const;
    std::vector<double> GetCurrCOIRates2()   const;
    // Guar COI rates usually use;
    //   non-smoker-differentiated table for unismoke;
    //   a published table (e.g. table D) for unisex.
    std::vector<double> GetGuarCOIRates()    const;
    // This function custom blends unismoke and unisex rates, using
    //   the same table as guaranteed COI rates.
    std::vector<double> GetSmokerBlendedGuarCOIRates()   const;
    std::vector<double> GetWPRates()                     const;
    std::vector<double> GetADDRates()                    const;
    std::vector<double> GetChildRiderRates()             const;
    std::vector<double> GetCurrentSpouseRiderRates()     const;
    std::vector<double> GetGuaranteedSpouseRiderRates()  const;
    std::vector<double> GetCurrentTermRates()            const;
    std::vector<double> GetGuaranteedTermRates()         const;
    std::vector<double> GetTableYRates()     const;
    std::vector<double> GetTAMRA7PayRates()  const;
    std::vector<double> GetTgtPremRates()    const;
    std::vector<double> GetIRC7702Rates()    const;
    std::vector<double> Get83GamRates()      const;

  protected:
    double GetModalMinPrem
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalTgtPrem
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalPremMaxNonMec
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalPremTgtFromTable
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalPremCorridor
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalPremGLP
        (int           a_Duration
        ,e_mode const& a_Mode
        ,double        a_BftAmt
        ,double        a_SpecAmt
        ) const;
    double GetModalPremGSP
        (int           a_Duration
        ,e_mode const& a_Mode
        ,double        a_BftAmt
        ,double        a_SpecAmt
        ) const;
    double GetModalSpecAmtMax
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetModalSpecAmtTgt
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetModalSpecAmtMinNonMec
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetModalSpecAmtCorridor
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetModalSpecAmtGLP
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetModalSpecAmtGSP
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    // Deprecated--used only by the lmi branch, which does not
    // distinguish ee from er premium.
    double GetModalMaxSpecAmt
        (e_mode const& Mode
        ,double Pmt
        ) const;
    // Deprecated--used only by the lmi branch, which does not
    // distinguish ee from er premium.
    double GetModalTgtSpecAmt
        (e_mode const& Mode
        ,double Pmt
        ) const;
    std::vector<double> GetTable
        (std::string const& TableFile
        ,long int    const& TableID
        ,EBlend      const& CanBlendSmoking = CannotBlend
        ,EBlend      const& CanBlendGender = CannotBlend
        ) const;

    std::vector<double> const& GetBandedCoiRates
        (e_basis const& rate_basis
        ,double specamt
        ) const;

    void CalculateNon7702CompliantCorridor() const;

    // TODO ?? A priori, protected data is a defect.
    int                     Length;
    int                     IssueAge;
    int                     SpouseIssueAge;
    int                     RetAge;

    // Invariant data.
    void                    SetPermanentInvariants();
    void                    SetLowestPremTaxRate();
    void                    TestPremiumTaxLoadConsistency();
    void                    SetMaxSurvivalDur();
    double                  MaxSurvivalDur;
    e_defn_life_ins         DefnLifeIns;
    e_defn_material_change  DefnMaterialChange;
    e_dbopt_7702            Equiv7702DBO3;
    double                  MaxNAAR;
    int                     EndtAge;
    double                  MinRenlBaseFace;
    double                  MinRenlFace;
    bool                    NoLapseOpt1Only;
    bool                    NoLapseUnratedOnly;
    bool                    OptChgCanIncrSA;
    bool                    OptChgCanDecrSA;
    bool                    WDCanDecrSADBO1;
    bool                    WDCanDecrSADBO2;
    bool                    WDCanDecrSADBO3;
    int                     MaxIncrAge;
    bool                    WaivePmTxInt1035;
    bool                    AllowTerm;
    double                  ExpPerKLimit;
    e_anticipated_deduction MaxWDDed;
    double                  MaxWDAVMult;
    e_anticipated_deduction MaxLoanDed;
    double                  MaxLoanAVMult;
    int                     NoLapseMinDur;
    int                     NoLapseMinAge;
    e_waiver_charge_method  WaiverChargeMethod;
    bool                    LapseIgnoresSurrChg;
    bool                    SurrChgOnIncr;
    bool                    SurrChgOnDecr;
    std::vector<double>     FreeWDProportion;

    double                  MinSpecAmt;
    double                  ADDLimit;
    double                  WPLimit;
    double                  SpecAmtLoadLimit;
    double                  MinWD;
    double                  WDFee;
    double                  WDFeeRate;

    bool                    AllowChangeToDBO2;
    bool                    AllowSAIncr;
    bool                    NoLapseAlwaysActive;
    std::vector<double>     DBDiscountRate;

    std::vector<double>     AssetComp;
    std::vector<double>     CompTarget;
    std::vector<double>     CompExcess;

    bool                    UseUnusualCOIBanding;
    bool                    COIIsDynamic;
    bool                    MandEIsDynamic;
    bool                    SepAcctLoadIsDynamic;

    std::vector<double>     MinPremIntSpread_;

    std::vector<double>     TieredMEBands;
    std::vector<double>     TieredMECharges;

    double                  FirstYearPremiumRetaliationLimit;
    bool                    PremiumTaxLoadIsTieredInStateOfDomicile;
    bool                    PremiumTaxLoadIsTieredInStateOfJurisdiction;

    round_to<double>        round_specamt;
    round_to<double>        round_death_benefit;
    round_to<double>        round_naar;
    round_to<double>        round_coi_rate;
    round_to<double>        round_coi_charge;
    round_to<double>        round_gross_premium;
    round_to<double>        round_net_premium;
    round_to<double>        round_interest_rate;
    round_to<double>        round_interest_credit;
    round_to<double>        round_withdrawal;
    round_to<double>        round_loan;
    round_to<double>        round_corridor_factor;
    round_to<double>        round_surrender_charge;
    round_to<double>        round_irr;
    round_to<double>        round_min_specamt;
    round_to<double>        round_max_specamt;
    round_to<double>        round_min_premium;
    round_to<double>        round_max_premium;
    round_to<double>        round_interest_rate_7702;

  private:
    double GetModalPrem
        (int                      Year
        ,e_mode            const& Mode
        ,double                   SpecAmt
        ,e_modal_prem_type const& PremType
        ) const;
    double GetModalPremMlyDed
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GetModalSpecAmt
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ,e_modal_prem_type a_PremType
        ) const;
    double GetModalSpecAmtMlyDed
        (e_mode const& a_EeMode
        ,double        a_EePmt
        ,e_mode const& a_ErMode
        ,double        a_ErPmt
        ) const;
    double GetAnnuityValueMlyDed
        (int           Year
        ,e_mode const& Mode
        ) const;

    std::vector<double> GetInforceAdjustedTable
        (std::string const& TableFile
        ,long int           TableID
        ,long int           TableNumber
        ) const;

    std::vector<double> GetUnblendedTable
        (std::string const& TableFile
        ,long int           TableID
        ) const;

    std::vector<double> GetUnblendedTable
        (std::string const& TableFile
        ,long int           TableID
        ,e_gender    const& gender
        ,e_smoking   const& smoking
        ) const;

    e_ledger_type       LedgerType;
    e_state             StateOfJurisdiction;
    e_state             StateOfDomicile;
    double              PremTaxRate;
    double              LowestPremTaxRate;
    double              DomiciliaryPremTaxRate;
    mutable double      InitialTargetPremium;

    void                Init7702();
    void                Init7702A();
    std::vector<double> SpreadFor7702_;
    std::vector<double> Mly7702iGlp;
    std::vector<double> Mly7702iGsp;
    std::vector<double> Mly7702ig;
    std::vector<double> Mly7702qc;
    mutable std::vector<double> Non7702CompliantCorridor;
};

inline int BasicValues::GetLength() const
{
    return Length;
}

inline int BasicValues::GetIssueAge() const
{
    return IssueAge;
}

inline int BasicValues::GetRetAge() const
{
    return RetAge;
}

inline e_ledger_type const& BasicValues::GetLedgerType() const
{
    return LedgerType;
}

inline e_state const& BasicValues::GetStateOfJurisdiction() const
{
    return StateOfJurisdiction;
}

inline e_state const& BasicValues::GetStateOfDomicile() const
{
    return StateOfDomicile;
}

inline double BasicValues::GetPremTaxRate() const
{
    return PremTaxRate;
}

inline double BasicValues::GetLowestPremTaxRate() const
{
    return LowestPremTaxRate;
}

inline double BasicValues::GetDomiciliaryPremTaxRate() const
{
    return DomiciliaryPremTaxRate;
}

inline rounding_rules const& BasicValues::GetRoundingRules() const
{
    return *RoundingRules_;
}

// TODO ?? Use a configuration file instead. These deprecated
// functions are used only by the lmi branch.
char const* CurrentTableFile();
char const* GuaranteedTableFile();

#endif // basic_values_hpp

