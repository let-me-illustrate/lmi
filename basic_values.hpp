// Rates and unprojected values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifndef basic_values_hpp
#define basic_values_hpp

#include "config.hpp"

#include "actuarial_table.hpp"          // e_actuarial_table_method
#include "dbnames.hpp"                  // e_database_key
#include "mc_enum_type_enums.hpp"
#include "oecumenic_enumerations.hpp"
#include "round_to.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "yare_input.hpp"

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

class FundData;
class Input;
class InterestRates;
class Irc7702;
class Irc7702A;
class Loads;
class MortalityRates;
class SurrChgRates;
class death_benefits;
class modal_outlay;
class premium_tax;
class product_data;
class product_database;
class rounding_rules;
class stratified_charges;

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

class LMI_SO BasicValues
    :private lmi::uncopyable<BasicValues>
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
    virtual ~BasicValues();

    void Init();
    void GPTServerInit();

    int                   GetLength()                  const;
    int                   GetIssueAge()                const;
    int                   GetRetAge()                  const;
    mcenum_ledger_type    GetLedgerType()              const;
    mcenum_state          GetStateOfJurisdiction()     const;
    mcenum_state          GetStateOfDomicile()         const;
    mcenum_state          GetPremiumTaxState()         const;
    bool                  IsSubjectToIllustrationReg() const;
    double                InvestmentManagementFee()    const;

    boost::shared_ptr<Input const>        Input_;
    yare_input                            yare_input_;
    boost::shared_ptr<product_data>       ProductData_;
    boost::shared_ptr<product_database>   Database_;
    boost::shared_ptr<FundData>           FundData_;
    boost::shared_ptr<rounding_rules>     RoundingRules_;
    boost::shared_ptr<stratified_charges> StratifiedCharges_;
    boost::shared_ptr<MortalityRates>     MortalityRates_;
    boost::shared_ptr<InterestRates>      InterestRates_;
    boost::shared_ptr<SurrChgRates>       SurrChgRates_;
    boost::shared_ptr<death_benefits>     DeathBfts_;
    boost::shared_ptr<modal_outlay>       Outlay_;
    boost::shared_ptr<premium_tax>        PremiumTax_;
    boost::shared_ptr<Loads>              Loads_;
    boost::shared_ptr<Irc7702>            Irc7702_;
    boost::shared_ptr<Irc7702A>           Irc7702A_;

    double GetAnnualTgtPrem(int a_year, double a_specamt) const;

    std::vector<double> const& GetCorridorFactor() const;
    std::vector<double> const& SpreadFor7702() const;
    std::vector<double> const& GetMly7702iGlp() const;
    std::vector<double> const& GetMly7702qc() const;
    std::vector<double> const& GetMlyDcvqc() const;

    // COI and term rates are blended on the current basis, but not
    // the guaranteed basis. Midpoint rates reflect blending, as a
    // consequence.

    std::vector<double> GetCvatCorridorFactors() const;
    std::vector<double> GetIrc7702NspRates() const;
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

  protected:
    double GetModalMinPrem
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalTgtPrem
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalPremMaxNonMec
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalPremTgtFromTable
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalPremCorridor
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalPremGLP
        (int         a_duration
        ,mcenum_mode a_mode
        ,double      a_bft_amt
        ,double      a_specamt
        ) const;
    double GetModalPremGSP
        (int         a_duration
        ,mcenum_mode a_mode
        ,double      a_bft_amt
        ,double      a_specamt
        ) const;
    double GetModalSpecAmtMax      (double annualized_pmt) const;
    double GetModalSpecAmtTgt      (double annualized_pmt) const;
    double GetModalSpecAmtMinNonMec(double annualized_pmt) const;
    double GetModalSpecAmtGLP      (double annualized_pmt) const;
    double GetModalSpecAmtGSP      (double annualized_pmt) const;
    double GetModalSpecAmtCorridor (double annualized_pmt) const;
    double GetModalSpecAmtSalary   (int a_year) const;
    // Deprecated--used only by the antediluvian branch, which does
    // not distinguish ee from er premium.
    double GetModalMaxSpecAmt
        (mcenum_mode a_mode
        ,double      a_pmt
        ) const;
    // Deprecated--used only by the antediluvian branch, which does
    // not distinguish ee from er premium.
    double GetModalTgtSpecAmt
        (mcenum_mode a_mode
        ,double      a_pmt
        ) const;
    std::vector<double> GetTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ,bool               IsTableValid    = true
        ,EBlend      const& CanBlendSmoking = CannotBlend
        ,EBlend      const& CanBlendGender  = CannotBlend
        ) const;

    std::vector<double> const& GetBandedCoiRates
        (mcenum_gen_basis rate_basis
        ,double           a_specamt
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
    mcenum_dbopt_7702            Equiv7702DBO3;
    double                       MaxNAAR;
    int                          EndtAge;
    double                       MinSpecAmt; // Antediluvian.
    double                       MinIssSpecAmt;
    double                       MinIssBaseSpecAmt;
    double                       MinRenlSpecAmt;
    double                       MinRenlBaseSpecAmt;
    bool                         NoLapseOpt1Only;
    bool                         NoLapseUnratedOnly;
    bool                         OptChgCanIncrSA;
    bool                         OptChgCanDecrSA;
    bool                         WDCanDecrSADBO1;
    bool                         WDCanDecrSADBO2;
    bool                         WDCanDecrSADBO3;
    int                          MaxIncrAge;
    bool                         WaivePmTxInt1035;
    bool                         AllowTerm;
    int                          TermForcedConvAge;
    int                          TermForcedConvDur;
    bool                         TermIsDbFor7702;
    bool                         TermIsDbFor7702A;
    double                       ExpPerKLimit;
    oenum_modal_prem_type        MinPremType;
    oenum_modal_prem_type        TgtPremType;
    bool                         TgtPremFixedAtIssue;
    double                       TgtPremMonthlyPolFee;
    double                       CurrCoiTable0Limit;
    double                       CurrCoiTable1Limit;
    e_actuarial_table_method     CoiInforceReentry;
    mcenum_anticipated_deduction MaxWDDed_;
    double                       MaxWdGenAcctValMult;
    double                       MaxWdSepAcctValMult;
    mcenum_anticipated_deduction MaxLoanDed_;
    double                       MaxLoanAVMult;
    int                          NoLapseMinDur;
    int                          NoLapseMinAge;
    oenum_waiver_charge_method   WaiverChargeMethod;
    bool                         LapseIgnoresSurrChg;
    bool                         SurrChgOnIncr;
    bool                         SurrChgOnDecr;
    std::vector<double>          FreeWDProportion;

    double                  AdbLimit;
    double                  WpLimit;
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

    // TODO ?? These two data members make the code clearer and
    // marginally more efficient. They're set from database entities
    // 'DB_DynamicMandE' and 'DB_DynamicSepAcctLoad', which seems
    // regrettable because a database maintainer could overlook them.
    // It would seem better to set them dynamically based on whether
    // the dynamic charges are not zero.

    bool                    MandEIsDynamic;
    bool                    SepAcctLoadIsDynamic;

    std::vector<double>     MinPremIntSpread_;

    std::vector<double>     TieredMEBands;
    std::vector<double>     TieredMECharges;

  private:
    double GetModalPrem
        (int                   a_year
        ,mcenum_mode           a_mode
        ,double                a_specamt
        ,oenum_modal_prem_type a_prem_type
        ) const;
    double GetModalPremMlyDed
        (int         a_year
        ,mcenum_mode a_mode
        ,double      a_specamt
        ) const;
    double GetModalSpecAmt
        (double                annualized_pmt
        ,oenum_modal_prem_type premium_type
        ) const;
    double GetModalSpecAmtMlyDed(double annualized_pmt, mcenum_mode) const;
    double GetAnnuityValueMlyDed(int a_year, mcenum_mode a_mode) const;

    std::vector<double> GetActuarialTable
        (std::string const& TableFile
        ,e_database_key     TableID
        ,long int           TableNumber
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

    mcenum_ledger_type  LedgerType_;
    bool                IsSubjectToIllustrationReg_;
    mcenum_state        StateOfJurisdiction_;
    mcenum_state        StateOfDomicile_;
    mcenum_state        PremiumTaxState_;
    mutable double      InitialTargetPremium;

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

inline mcenum_ledger_type BasicValues::GetLedgerType() const
{
    return LedgerType_;
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

inline bool BasicValues::IsSubjectToIllustrationReg() const
{
    return IsSubjectToIllustrationReg_;
}

// TODO ?? Use a configuration file instead. These deprecated
// functions are used only by the antediluvian branch.
char const* CurrentTableFile();
char const* GuaranteedTableFile();

#endif // basic_values_hpp

