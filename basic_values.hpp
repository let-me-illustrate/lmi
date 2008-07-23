// Rates and unprojected values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: basic_values.hpp,v 1.38 2008-07-23 23:19:38 chicares Exp $

#ifndef basic_values_hpp
#define basic_values_hpp

#include "config.hpp"

#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "oecumenic_enumerations.hpp"
#include "round_to.hpp"
#include "so_attributes.hpp"
#include "yare_input.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

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
class InputParms;
class InterestRates;
class Irc7702;
class Irc7702A;
class Loads;
class MortalityRates;
class SurrChgRates;
class TDatabase;
class TProductData;
class death_benefits;
class modal_outlay;
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
    :virtual private boost::noncopyable
{
  public:
    BasicValues();
    BasicValues(InputParms const* input);
    // This signature is preferable to the version that takes a const*;
    // for now, it's used only by the lmi branch.
    BasicValues(InputParms const& input);
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
    mce_state      const& GetStateOfJurisdiction()     const;
    mce_state      const& GetStateOfDomicile()         const;
    double                PremiumTaxRate()             const;
    double                LowestPremiumTaxLoad()       const;
    double                DomiciliaryPremiumTaxLoad()  const;
    bool                  IsPremiumTaxLoadTiered()     const;
    bool                  IsSubjectToIllustrationReg() const;
    rounding_rules const& GetRoundingRules()           const;
    double                InvestmentManagementFee()    const;

    boost::shared_ptr<InputParms const>   Input_;
    yare_input                            yare_input_;
    boost::shared_ptr<TProductData>       ProductData_;
    boost::shared_ptr<TDatabase>          Database_;
    boost::shared_ptr<FundData>           FundData_;
    boost::shared_ptr<rounding_rules>     RoundingRules_;
    boost::shared_ptr<stratified_charges> StratifiedCharges_;
    boost::shared_ptr<MortalityRates>     MortalityRates_;
    boost::shared_ptr<InterestRates>      InterestRates_;
    boost::shared_ptr<SurrChgRates>       SurrChgRates_;
    boost::shared_ptr<death_benefits>     DeathBfts_;
    boost::shared_ptr<modal_outlay>       Outlay_;
    boost::shared_ptr<Loads>              Loads_;
    boost::shared_ptr<Irc7702>            Irc7702_;
    boost::shared_ptr<Irc7702A>           Irc7702A_;

    double GetTgtPrem
        (int          a_year
        ,double       a_specamt
        ,mcenum_dbopt a_dbopt
        ,mcenum_mode  a_mode
        ) const;

    std::vector<double> const& GetCorridorFactor() const;
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
    std::vector<double> GetWpRates()                     const;
    std::vector<double> GetAdbRates()                    const;
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
    double GetModalSpecAmtMax
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetModalSpecAmtTgt
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetModalSpecAmtMinNonMec
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetModalSpecAmtCorridor
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetModalSpecAmtGLP
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetModalSpecAmtGSP
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    // Deprecated--used only by the lmi branch, which does not
    // distinguish ee from er premium.
    double GetModalMaxSpecAmt
        (mcenum_mode a_mode
        ,double      a_pmt
        ) const;
    // Deprecated--used only by the lmi branch, which does not
    // distinguish ee from er premium.
    double GetModalTgtSpecAmt
        (mcenum_mode a_mode
        ,double      a_pmt
        ) const;
    std::vector<double> GetTable
        (std::string const& TableFile
        ,long int    const& TableID
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
    void                         AssertAgeAndDobAreConsistent(bool use_anb);
    void                         SetPermanentInvariants();
    void                         SetLowestPremiumTaxLoad();
    void                         TestPremiumTaxLoadConsistency();
    void                         SetMaxSurvivalDur();
    double                       MaxSurvivalDur;
    mcenum_defn_life_ins         DefnLifeIns_;
    mcenum_defn_material_change  DefnMaterialChange_;
    mcenum_dbopt_7702            Equiv7702DBO3;
    double                       MaxNAAR;
    int                          EndtAge;
    double                       MinRenlBaseFace;
    double                       MinRenlFace;
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
    double                       ExpPerKLimit;
    mcenum_anticipated_deduction MaxWDDed_;
    double                       MaxWDAVMult;
    mcenum_anticipated_deduction MaxLoanDed_;
    double                       MaxLoanAVMult;
    int                          NoLapseMinDur;
    int                          NoLapseMinAge;
    oenum_waiver_charge_method   WaiverChargeMethod;
    bool                         LapseIgnoresSurrChg;
    bool                         SurrChgOnIncr;
    bool                         SurrChgOnDecr;
    std::vector<double>          FreeWDProportion;

    double                  MinSpecAmt;
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
        (mcenum_mode           a_ee_mode
        ,double                a_ee_pmt
        ,mcenum_mode           a_er_mode
        ,double                a_er_pmt
        ,oenum_modal_prem_type a_prem_type
        ) const;
    double GetModalSpecAmtMlyDed
        (mcenum_mode a_ee_mode
        ,double      a_ee_pmt
        ,mcenum_mode a_er_mode
        ,double      a_er_pmt
        ) const;
    double GetAnnuityValueMlyDed
        (int         a_year
        ,mcenum_mode a_mode
        ) const;

    std::vector<double> GetActuarialTable
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
        ,mcenum_gender      gender
        ,mcenum_smoking     smoking
        ) const;

    mcenum_ledger_type  LedgerType_;
    bool                IsSubjectToIllustrationReg_;
    mce_state           StateOfJurisdiction_;
    mce_state           StateOfDomicile_;
    double              PremiumTaxRate_;
    double              LowestPremiumTaxLoad_;
    double              DomiciliaryPremiumTaxLoad_;
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

inline mcenum_ledger_type BasicValues::GetLedgerType() const
{
    return LedgerType_;
}

inline mce_state const& BasicValues::GetStateOfJurisdiction() const
{
    return StateOfJurisdiction_;
}

inline mce_state const& BasicValues::GetStateOfDomicile() const
{
    return StateOfDomicile_;
}

inline double BasicValues::PremiumTaxRate() const
{
    return PremiumTaxRate_;
}

inline double BasicValues::LowestPremiumTaxLoad() const
{
    return LowestPremiumTaxLoad_;
}

inline double BasicValues::DomiciliaryPremiumTaxLoad() const
{
    return DomiciliaryPremiumTaxLoad_;
}

inline bool BasicValues::IsPremiumTaxLoadTiered() const
{
    return PremiumTaxLoadIsTieredInStateOfJurisdiction;
}

inline bool BasicValues::IsSubjectToIllustrationReg() const
{
    return IsSubjectToIllustrationReg_;
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

