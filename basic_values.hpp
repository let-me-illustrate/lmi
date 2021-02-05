// Rates and unprojected values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef basic_values_hpp
#define basic_values_hpp

#include "config.hpp"

#include "actuarial_table.hpp"          // e_actuarial_table_method
#include "currency.hpp"
#include "database.hpp"
#include "dbnames.hpp"                  // e_database_key
#include "mc_enum_type_enums.hpp"
#include "oecumenic_enumerations.hpp"
#include "product_data.hpp"
#include "round_to.hpp"
#include "so_attributes.hpp"
#include "yare_input.hpp"

#include <memory>                       // shared_ptr
#include <string>
#include <utility>                      // pair
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

class FundData;
class Input;
class InterestRates;
class Irc7702;
class Irc7702A;
class Loads;
class MortalityRates;
class death_benefits;
class lingo;
class modal_outlay;
class premium_tax;
class rounding_rules;
class stratified_charges;

// See GetTable(). This idea may be poor, but we're OK as long as we
// don't use MustBlend. Here's the problem. The MustBlend case uses
// the input male % and nonsmoker %. But we might really want to use
// a predetermined % instead, like 80% for guar COI. Especially in
// that case, we're better off not creating a blended table because
// NAIC has published blended tables.
enum EBlend
    {CannotBlend
    ,CanBlend
    ,MustBlend
    };

class LMI_SO BasicValues
{
  public:
    BasicValues(Input const& input);
    BasicValues // GPT server only.
        (std::string const& a_ProductName
        ,mcenum_gender      a_Gender
        ,mcenum_class       a_UnderwritingClass
        ,mcenum_smoking     a_Smoker
        ,int                a_IssueAge
        ,mcenum_uw_basis    a_UnderwritingBasis
        ,mcenum_state       a_StateOfJurisdiction
        ,double             a_FaceAmount
        ,mcenum_dbopt_7702  a_DBOpt
        ,bool               a_AdbInForce
        ,double             a_TargetPremium
        );
    BasicValues(BasicValues&&) = default;
    virtual ~BasicValues();

    void Init();

    int                   GetLength()                  const;
    int                   GetIssueAge()                const;
    int                   GetRetAge()                  const;
    mcenum_ledger_type    ledger_type               () const;
    bool                  nonillustrated            () const;
    bool                  no_can_issue              () const;
    bool                  IsSubjectToIllustrationReg() const;
    mcenum_state          GetStateOfJurisdiction()     const;
    mcenum_state          GetStateOfDomicile()         const;
    mcenum_state          GetPremiumTaxState()         const;

    auto const& partial_mortality_qx () const {return partial_mortality_qx_ ;}
    auto const& partial_mortality_tpx() const {return partial_mortality_tpx_;}
    auto const& partial_mortality_lx () const {return partial_mortality_lx_ ;}

    double                InvestmentManagementFee()    const;

    yare_input                                yare_input_;

    std::shared_ptr<product_data       const> product_;
    product_database                   const  database_;
    std::shared_ptr<lingo              const> lingo_;
    std::shared_ptr<FundData           const> FundData_;
    std::shared_ptr<rounding_rules     const> RoundingRules_;
    std::shared_ptr<stratified_charges const> StratifiedCharges_;

    std::shared_ptr<MortalityRates>     MortalityRates_;
    std::shared_ptr<InterestRates>      InterestRates_;
    std::shared_ptr<death_benefits>     DeathBfts_;
    std::shared_ptr<modal_outlay>       Outlay_;
    std::shared_ptr<premium_tax>        PremiumTax_;
    std::shared_ptr<Loads>              Loads_;
    std::shared_ptr<Irc7702>            Irc7702_;
    std::shared_ptr<Irc7702A>           Irc7702A_;

    product_data     const& product () const {return *product_;}
    product_database const& database() const {return database_;}

    currency GetAnnualTgtPrem(int a_year, currency a_specamt) const;

    std::vector<double> const& GetCorridorFactor() const;
    std::vector<double> const& SpreadFor7702() const;
    std::vector<double> const& GetMly7702iGlp() const;
    std::vector<double> const& GetMly7702qc() const;
    std::vector<double> const& GetMlyDcvqc() const;

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
    std::vector<double> GetSmokerBlendedGuarCOIRates()  const;
    std::vector<double> GetWpRates()                    const;
    std::vector<double> GetAdbRates()                   const;
    std::vector<double> GetChildRiderRates()            const;
    std::vector<double> GetCurrentSpouseRiderRates()    const;
    std::vector<double> GetGuaranteedSpouseRiderRates() const;
    std::vector<double> GetCurrentTermRates()           const;
    std::vector<double> GetGuaranteedTermRates()        const;
    std::vector<double> GetGroupProxyRates()            const;
    std::vector<double> GetSevenPayRates()              const;
    std::vector<double> GetMinPremRates()               const;
    std::vector<double> GetTgtPremRates()               const;
    std::vector<double> GetIrc7702QRates()              const;
    std::vector<double> GetPartialMortalityRates()      const;
    std::vector<double> GetSubstdTblMultTable()         const;
    std::vector<double> GetCurrSpecAmtLoadTable()       const;
    std::vector<double> GetGuarSpecAmtLoadTable()       const;

    round_to<double> const& round_specamt           () const {return round_specamt_           ;}
    round_to<double> const& round_death_benefit     () const {return round_death_benefit_     ;}
    round_to<double> const& round_naar              () const {return round_naar_              ;}
    round_to<double> const& round_coi_rate          () const {return round_coi_rate_          ;}
    round_to<double> const& round_coi_charge        () const {return round_coi_charge_        ;}
    round_to<double> const& round_rider_charges     () const {return round_rider_charges_     ;}
    round_to<double> const& round_gross_premium     () const {return round_gross_premium_     ;}
    round_to<double> const& round_net_premium       () const {return round_net_premium_       ;}
    round_to<double> const& round_interest_rate     () const {return round_interest_rate_     ;}
    round_to<double> const& round_interest_credit   () const {return round_interest_credit_   ;}
    round_to<double> const& round_withdrawal        () const {return round_withdrawal_        ;}
    round_to<double> const& round_loan              () const {return round_loan_              ;}
    round_to<double> const& round_interest_rate_7702() const {return round_interest_rate_7702_;}
    round_to<double> const& round_corridor_factor   () const {return round_corridor_factor_   ;}
    round_to<double> const& round_nsp_rate_7702     () const {return round_nsp_rate_7702_     ;}
    round_to<double> const& round_seven_pay_rate    () const {return round_seven_pay_rate_    ;}
    round_to<double> const& round_surrender_charge  () const {return round_surrender_charge_  ;}
    round_to<double> const& round_irr               () const {return round_irr_               ;}
    round_to<double> const& round_min_specamt       () const {return round_min_specamt_       ;}
    round_to<double> const& round_max_specamt       () const {return round_max_specamt_       ;}
    round_to<double> const& round_min_premium       () const {return round_min_premium_       ;}
    round_to<double> const& round_max_premium       () const {return round_max_premium_       ;}
    round_to<double> const& round_minutiae          () const {return round_minutiae_          ;}

  protected:
    currency GetModalMinPrem
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalTgtPrem
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalPremMaxNonMec
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalPremMinFromTable
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalPremTgtFromTable
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalPremProxyTable
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ,double      a_table_multiplier
        ) const;
    currency GetModalPremCorridor
        (int         a_year
        ,mcenum_mode a_mode
        ,currency    a_specamt
        ) const;
    currency GetModalPremGLP
        (int         a_duration
        ,mcenum_mode a_mode
        ,currency    a_bft_amt
        ,currency    a_specamt
        ) const;
    currency GetModalPremGSP
        (int         a_duration
        ,mcenum_mode a_mode
        ,currency    a_bft_amt
        ,currency    a_specamt
        ) const;
    std::pair<currency,currency> GetModalPremMlyDedEx
        (int         year
        ,mcenum_mode mode
        ,currency    specamt
        ,currency    termamt
        ) const;
    currency GetListBillPremMlyDed
        (int         year
        ,mcenum_mode mode
        ,currency    specamt
        ) const;
    std::pair<currency,currency> GetListBillPremMlyDedEx
        (int         year
        ,mcenum_mode mode
        ,currency    specamt
        ,currency    termamt
        ) const;
    currency GetModalSpecAmtMax      (currency annualized_pmt) const;
    currency GetModalSpecAmtTgt      (currency annualized_pmt) const;
    currency GetModalSpecAmtMinNonMec(currency annualized_pmt) const;
    currency GetModalSpecAmtGLP      (currency annualized_pmt) const;
    currency GetModalSpecAmtGSP      (currency annualized_pmt) const;
    currency GetModalSpecAmtCorridor (currency annualized_pmt) const;
    currency GetModalSpecAmtSalary   (int a_year) const;
    // Deprecated--used only by the antediluvian branch, which does
    // not distinguish ee from er premium.
    currency GetModalMaxSpecAmt
        (mcenum_mode a_mode
        ,currency    a_pmt
        ) const;
    // Deprecated--used only by the antediluvian branch, which does
    // not distinguish ee from er premium.
    currency GetModalTgtSpecAmt
        (mcenum_mode a_mode
        ,currency    a_pmt
        ) const;
    std::vector<double> GetTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ,bool               IsTableValid    = true
        ,EBlend             CanBlendSmoking = CannotBlend
        ,EBlend             CanBlendGender  = CannotBlend
        ) const;

    std::vector<double> const& GetBandedCoiRates
        (mcenum_gen_basis rate_basis
        ,currency         a_specamt
        ) const;

    // TODO ?? A priori, protected data is a defect.
    int                     Length;
    int                     IssueAge;
    int                     SpouseIssueAge;
    int                     RetAge;

    // Invariant data.
    void                         SetPermanentInvariants();
    void                         SetMaxSurvivalDur();
    double                       MaxSurvivalDur;
    mcenum_defn_life_ins         DefnLifeIns_;
    mcenum_defn_material_change  DefnMaterialChange_;
    mcenum_dbopt_7702            Effective7702DboRop;
    currency                     MaxNAAR;
    int                          EndtAge;
    currency                     MinSpecAmt; // Antediluvian.
    currency                     MinIssSpecAmt;
    currency                     MinIssBaseSpecAmt;
    currency                     MinRenlSpecAmt;
    currency                     MinRenlBaseSpecAmt;
    bool                         NoLapseDboLvlOnly;
    bool                         NoLapseUnratedOnly;
    bool                         OptChgCanIncrSA;
    bool                         OptChgCanDecrSA;
    bool                         WdDecrSpecAmtDboLvl;
    bool                         WdDecrSpecAmtDboInc;
    bool                         WdDecrSpecAmtDboRop;
    int                          MaxIncrAge;
    bool                         WaivePmTxInt1035;
    bool                         TermIsNotRider;
    int                          TermForcedConvAge;
    int                          TermForcedConvDur;
    bool                         TermIsDbFor7702;
    bool                         TermIsDbFor7702A;
    oenum_modal_prem_type        MinPremType;
    oenum_modal_prem_type        TgtPremType;
    bool                         TgtPremFixedAtIssue;
    currency                     TgtPremMonthlyPolFee;
    currency                     CurrCoiTable0Limit;
    currency                     CurrCoiTable1Limit;
    e_actuarial_table_method     CoiInforceReentry;
    mcenum_anticipated_deduction MaxWDDed_;
    double                       MaxWdGenAcctValMult;
    double                       MaxWdSepAcctValMult;
    bool                         AllowPrefLoan;
    mcenum_anticipated_deduction MaxLoanDed_;
    double                       MaxLoanAVMult;
    int                          FirstPrefLoanYear;
    int                          NoLapseMinDur;
    int                          NoLapseMinAge;
    oenum_waiver_charge_method   WaiverChargeMethod;
    bool                         AllowCashValueEnh;
    std::vector<double>          CashValueEnhMult;
    bool                         LapseIgnoresSurrChg;
    bool                         SurrChgOnIncr;
    bool                         SurrChgOnDecr;
    std::vector<double>          FreeWDProportion;

    currency                AdbLimit;
    currency                WpLimit;
    currency                SpecAmtLoadLimit;
    currency                MinWD;
    currency                WDFee;
    double                  WDFeeRate;

    bool                    AllowChangeToDBO2;
    bool                    AllowSAIncr;
    bool                    NoLapseAlwaysActive;
    std::vector<double>     DBDiscountRate;

    bool                    UseUnusualCOIBanding;

    // TODO ?? These two data members make the code clearer and
    // marginally more efficient. They're set from database entities
    // 'DB_DynamicMandE' and 'DB_DynamicSepAcctLoad', which seems
    // regrettable because a database maintainer could overlook them.
    // It would seem better to set them dynamically based on whether
    // the dynamic charges are not zero.

    bool                    MandEIsDynamic;
    bool                    SepAcctLoadIsDynamic;

    std::vector<double>     MinPremIntSpread_;

    std::vector<currency>   TieredMEBands;
    std::vector<double>     TieredMECharges;

  private:
    BasicValues(BasicValues const&) = delete;
    BasicValues& operator=(BasicValues const&) = delete;

    void set_partial_mortality();
    double GetPartMortQ(int year) const;

    double mly_ded_discount_factor(int year, mcenum_mode mode) const;
    std::pair<double,double> approx_mly_ded
        (int      year
        ,currency specamt
        ) const;
    std::pair<double,double> approx_mly_ded_ex
        (int      year
        ,currency specamt
        ,currency termamt
        ) const;
    currency GetModalPremMlyDed
        (int         year
        ,mcenum_mode mode
        ,currency    specamt
        ) const;
    currency GetModalSpecAmtMlyDed(currency annualized_pmt, mcenum_mode) const;

    std::vector<double> GetActuarialTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ,int                TableNumber
        ) const;

    std::vector<double> GetUnblendedTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ) const;

    std::vector<double> GetUnblendedTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ,mcenum_gender      gender
        ,mcenum_smoking     smoking
        ) const;

    mcenum_ledger_type  ledger_type_;
    bool                nonillustrated_;
    bool                no_can_issue_;
    bool                IsSubjectToIllustrationReg_;
    mcenum_state        StateOfJurisdiction_;
    mcenum_state        StateOfDomicile_;
    mcenum_state        PremiumTaxState_;

    std::vector<double> partial_mortality_qx_;
    std::vector<double> partial_mortality_tpx_;
    std::vector<double> partial_mortality_lx_;

    // This data member is not of type 'currency' because it merely
    // stores an input parameter for the GPT server, and is used for
    // no other purpose.
    double              InitialTargetPremium;

    void                Init7702();
    void                Init7702A();
    std::vector<double> SpreadFor7702_;
    std::vector<double> Mly7702iGlp;
    std::vector<double> Mly7702iGsp;
    std::vector<double> Mly7702ig;
    std::vector<double> Mly7702qc;
    std::vector<double> MlyDcvqc;
    mutable std::vector<double> Non7702CompliantCorridor;

    void SetRoundingFunctors();
    round_to<double> round_specamt_           ;
    round_to<double> round_death_benefit_     ;
    round_to<double> round_naar_              ;
    round_to<double> round_coi_rate_          ;
    round_to<double> round_coi_charge_        ;
    round_to<double> round_rider_charges_     ;
    round_to<double> round_gross_premium_     ;
    round_to<double> round_net_premium_       ;
    round_to<double> round_interest_rate_     ;
    round_to<double> round_interest_credit_   ;
    round_to<double> round_withdrawal_        ;
    round_to<double> round_loan_              ;
    round_to<double> round_interest_rate_7702_;
    round_to<double> round_corridor_factor_   ;
    round_to<double> round_nsp_rate_7702_     ;
    round_to<double> round_seven_pay_rate_    ;
    round_to<double> round_surrender_charge_  ;
    round_to<double> round_irr_               ;
    round_to<double> round_min_specamt_       ;
    round_to<double> round_max_specamt_       ;
    round_to<double> round_min_premium_       ;
    round_to<double> round_max_premium_       ;
    round_to<double> round_minutiae_          ;
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

inline mcenum_ledger_type BasicValues::ledger_type() const
{
    return ledger_type_;
}

inline bool BasicValues::nonillustrated() const
{
    return nonillustrated_;
}

inline bool BasicValues::no_can_issue() const
{
    return no_can_issue_;
}

inline bool BasicValues::IsSubjectToIllustrationReg() const
{
    return IsSubjectToIllustrationReg_;
}

inline mcenum_state BasicValues::GetStateOfJurisdiction() const
{
    return StateOfJurisdiction_;
}

inline mcenum_state BasicValues::GetStateOfDomicile() const
{
    return StateOfDomicile_;
}

inline mcenum_state BasicValues::GetPremiumTaxState() const
{
    return PremiumTaxState_;
}

// IHS !! Use a configuration file instead. These deprecated
// functions are used only by the antediluvian branch.
char const* CurrentTableFile();
char const* GuaranteedTableFile();

#endif // basic_values_hpp
