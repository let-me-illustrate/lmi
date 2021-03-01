// Basic values.
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

#include "pchfile.hpp"

#include "basic_values.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_tables.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "data_directory.hpp"
#include "death_benefits.hpp"
#include "et_vector.hpp"
#include "financial.hpp"                // list_bill_premium()
#include "fund_data.hpp"
#include "global_settings.hpp"
#include "gpt_specamt.hpp"
#include "i7702.hpp"
#include "ieee754.hpp"                  // ldbl_eps_plus_one_times()
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "lingo.hpp"
#include "loads.hpp"
#include "math_functions.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "mortality_rates.hpp"
#include "oecumenic_enumerations.hpp"
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "rounding_rules.hpp"
#include "stl_extensions.hpp"           // nonstd::power()
#include "stratified_charges.hpp"
#include "value_cast.hpp"

#include <algorithm>                    // min(), transform()
#include <cfenv>                        // fesetround()
#include <cmath>                        // nearbyint(), pow()
#include <functional>                   // minus, multiplies
#include <limits>
#include <numeric>                      // accumulate(), partial_sum()
#include <stdexcept>

//============================================================================
BasicValues::BasicValues(Input const& input)
    :yare_input_         (input)
    ,product_            (product_data::read_via_cache(filename_from_product_name(yare_input_.ProductName)))
    ,database_           (yare_input_)
    ,lingo_              (lingo::read_via_cache
        (AddDataDir(product().datum("LingoFilename"))))
    ,FundData_           (FundData::read_via_cache
        (AddDataDir(product().datum("FundFilename"))))
    ,RoundingRules_      (rounding_rules::read_via_cache
        (AddDataDir(product().datum("RoundingFilename"))))
    ,StratifiedCharges_  (stratified_charges::read_via_cache
        (AddDataDir(product().datum("TierFilename"))))
    ,i7702_              {std::make_unique<i7702>(database(), *StratifiedCharges_)}
    ,DefnLifeIns_        {mce_cvat}
    ,DefnMaterialChange_ {mce_unnecessary_premium}
    ,Effective7702DboRop {mce_option1_for_7702}
    ,MaxWDDed_           {mce_twelve_times_last}
    ,MaxLoanDed_         {mce_twelve_times_last}
    ,StateOfJurisdiction_{mce_s_CT}
    ,StateOfDomicile_    {mce_s_CT}
    ,PremiumTaxState_    {mce_s_CT}
    ,InitialTargetPremium{0.0}
{
    Init();
}

//============================================================================
// Designed for GPT server, but available for general use.
BasicValues::BasicValues
    (std::string  const& a_ProductName
    ,mcenum_gender       a_Gender
    ,mcenum_class        a_UnderwritingClass
    ,mcenum_smoking      a_Smoker
    ,int                 a_IssueAge
    ,mcenum_uw_basis     a_UnderwritingBasis
    ,mcenum_state        a_StateOfJurisdiction
    ,double              a_FaceAmount
    ,mcenum_dbopt_7702   a_DBOptFor7702
    ,bool                a_AdbInForce
    ,double              a_TargetPremium
    // TODO ?? Need loan rate type here?
    )
    :yare_input_         (Input{})
    ,product_            (product_data::read_via_cache(filename_from_product_name(a_ProductName)))
    ,database_
        (a_ProductName
        ,a_Gender
        ,a_UnderwritingClass
        ,a_Smoker
        ,a_IssueAge
        ,a_UnderwritingBasis
        ,a_StateOfJurisdiction
        )
    ,lingo_              (lingo::read_via_cache
        (AddDataDir(product().datum("LingoFilename"))))
    ,FundData_           (FundData::read_via_cache
        (AddDataDir(product().datum("FundFilename"))))
    ,RoundingRules_      (rounding_rules::read_via_cache
        (AddDataDir(product().datum("RoundingFilename"))))
    ,StratifiedCharges_  (stratified_charges::read_via_cache
        (AddDataDir(product().datum("TierFilename"))))
    ,i7702_              {std::make_unique<i7702>(database(), *StratifiedCharges_)}
    ,DefnLifeIns_        {mce_cvat}
    ,DefnMaterialChange_ {mce_unnecessary_premium}
    ,Effective7702DboRop {a_DBOptFor7702}
    ,MaxWDDed_           {mce_twelve_times_last}
    ,MaxLoanDed_         {mce_twelve_times_last}
    ,StateOfJurisdiction_{a_StateOfJurisdiction}
    ,StateOfDomicile_    {a_StateOfJurisdiction}
    ,PremiumTaxState_    {a_StateOfJurisdiction}
    ,InitialTargetPremium{a_TargetPremium}
{
    yare_input_.IssueAge                   = a_IssueAge           ;
    yare_input_.RetirementAge              = a_IssueAge           ;
    yare_input_.Gender                     = a_Gender             ;
    yare_input_.Smoking                    = a_Smoker             ;
    yare_input_.UnderwritingClass          = a_UnderwritingClass  ;
    if(a_AdbInForce)
        {
        yare_input_.AccidentalDeathBenefit     = mce_yes          ;
        }
    else
        {
        yare_input_.AccidentalDeathBenefit     = mce_no           ;
        }
    yare_input_.GroupUnderwritingType      = a_UnderwritingBasis  ;
    yare_input_.ProductName                = a_ProductName        ;
    yare_input_.PremiumTaxState            = a_StateOfJurisdiction;
    yare_input_.StateOfJurisdiction        = a_StateOfJurisdiction;
    yare_input_.DefinitionOfLifeInsurance  = mce_gpt              ;
    yare_input_.DefinitionOfMaterialChange = mce_adjustment_event ;

    int const db_len = database().length();

    yare_input_.SpecifiedAmount           .assign(db_len, a_FaceAmount);
    // The tax law recognizes no "supplemental" amount, but
    // class death_benefits expects SupplementalAmount to have
    // the same length as SpecifiedAmount.
    yare_input_.SupplementalAmount        .assign(db_len, 0.0);

    // Cf. effective_dbopt_7702()
    mce_dbopt const z
        (mce_option1_for_7702 == a_DBOptFor7702 ? mce_option1
        :mce_option2_for_7702 == a_DBOptFor7702 ? mce_option2
        :throw std::runtime_error("Unexpected DB option.")
        );
    yare_input_.DeathBenefitOption        .assign(db_len, z.value());

    yare_input_.ExtraMonthlyCustodialFee  .resize(db_len);
    yare_input_.ExtraCompensationOnAssets .resize(db_len);
    yare_input_.ExtraCompensationOnPremium.resize(db_len);
    yare_input_.CurrentCoiMultiplier      .assign(db_len, 1.0);
    yare_input_.FlatExtra                 .resize(db_len);

    // Used by class InterestRates:
    yare_input_.GeneralAccountRateType    = mce_credited_rate;
    yare_input_.SeparateAccountRateType   = mce_gross_rate;
    yare_input_.LoanRateType              = mce_fixed_loan_rate;
    yare_input_.HoneymoonEndorsement      = false;
    yare_input_.GeneralAccountRate        .assign(db_len, 0.0);
    yare_input_.SeparateAccountRate       .assign(db_len, 0.0);
    yare_input_.ExtraCompensationOnAssets .assign(db_len, 0.0);
    yare_input_.AmortizePremiumLoad       = false;
    yare_input_.LoanRate                  = 0.0;
    yare_input_.HoneymoonValueSpread      .assign(db_len, 0.0);
    yare_input_.PostHoneymoonSpread       = 0.0;

    Init();
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor is not
/// implemented inside the class definition, because the header
/// forward-declares one or more classes that are held by
/// std::unique_ptr, so their destructors are visible only here.

BasicValues::~BasicValues() = default;

//============================================================================
void BasicValues::Init()
{
    SetRoundingFunctors();

    SetPermanentInvariants();

    StateOfDomicile_ = mc_state_from_string(product().datum("InsCoDomicile"));
    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    if
        (   !database().query<bool>(DB_StateApproved)
        &&  !global_settings::instance().ash_nazg()
        &&  !global_settings::instance().regression_testing()
        )
        {
        alarum()
            << "Product "
            << yare_input_.ProductName
            << " not approved in state "
            << mc_str(GetStateOfJurisdiction())
            << "."
            << LMI_FLUSH
            ;
        }

    IssueAge = yare_input_.IssueAge;
    RetAge   = yare_input_.RetirementAge;
    LMI_ASSERT(IssueAge < 100);
    LMI_ASSERT(RetAge <= 100);
    LMI_ASSERT(yare_input_.RetireesCanEnroll || IssueAge <= RetAge);

    database().query_into(DB_MaturityAge   , EndtAge);
    Length = EndtAge - IssueAge;
    LMI_ASSERT(database().length() == Length);

    database().query_into(DB_LedgerType    , ledger_type_);
    database().query_into(DB_Nonillustrated, nonillustrated_);
    bool no_longer_issued = database().query<bool>(DB_NoLongerIssued);
    bool is_new_business  = yare_input_.EffectiveDate == yare_input_.InforceAsOfDate;
    no_can_issue_         = no_longer_issued && is_new_business;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(ledger_type());

    if(IssueAge < database().query<int>(DB_MinIssAge))
        {
        alarum()
            << "Issue age "
            << IssueAge
            << " less than minimum "
            << database().query<int>(DB_MinIssAge)
            << '.'
            << LMI_FLUSH
            ;
        }

    if(database().query<int>(DB_MaxIssAge) < IssueAge)
        {
        alarum()
            << "Issue age "
            << IssueAge
            << " greater than maximum "
            << database().query<int>(DB_MaxIssAge)
            << '.'
            << LMI_FLUSH
            ;
        }

    // Multilife contracts will need a vector of mortality-rate objects.

    // Mortality and interest rates require database and rounding.
    // Interest rates require tiered data and 7702 spread.
    MortalityRates_ = std::make_unique<MortalityRates>(*this);
    InterestRates_  = std::make_unique<InterestRates >(*this);
    DeathBfts_      = std::make_unique<death_benefits>
        (GetLength()
        ,yare_input_
        ,round_specamt_
        );
    // Outlay requires only input and rounding; it might someday use
    // interest rates.
    Outlay_         = std::make_unique<modal_outlay>
        (yare_input_
        ,round_gross_premium_
        ,round_withdrawal_
        ,round_loan_
        );
    PremiumTax_     = std::make_unique<premium_tax>
        (PremiumTaxState_
        ,StateOfDomicile_
        ,yare_input_.AmortizePremiumLoad
        ,database()
        ,*StratifiedCharges_
        );
    Loads_          = std::make_unique<Loads>(*this);

    SetMaxSurvivalDur();
    set_partial_mortality();

    Init7702();
    Init7702A();
}

//============================================================================
// TODO ?? Does this belong in the funds class? Consider merging it
// with code in AccountValue::SetInitialValues().
double BasicValues::InvestmentManagementFee() const
{
    if(!database().query<bool>(DB_AllowSepAcct))
        {
        return 0.0;
        }

    if(yare_input_.OverrideFundManagementFee)
        {
        return yare_input_.InputFundManagementFee;
        }

    double z = 0.0;
    double total_sepacct_allocations = 0.0;
    FundData const& Funds = *FundData_;

    for(int j = 0; j < Funds.GetNumberOfFunds(); ++j)
        {
        double weight;
        // If average of all funds, then use equal weights, but
        // disregard "custom" funds--that is, set their weights to
        // zero. Custom funds are those whose name begins with "Custom".
        // Reason: "average" means average of the normally-available
        // funds only. Use the average not only if explicitly chosen
        // in input, but also if all payments are allocated to the
        // general account: in the latter case, some separate-account
        // fee or rate may nonetheless be wanted on output, and the
        // arithmetic mean is more reasonable than zero.
        if(yare_input_.UseAverageOfAllFunds || 0.0 == premium_allocation_to_sepacct(yare_input_))
            {
            bool ignore = 0 == Funds.GetFundInfo(j).ShortName().find("custom");
            weight = ignore ? 0.0 : 1.0;
            }
        // If fund mgmt fee not overridden by average of all funds,
        // then use input weights.
/*
        else
            {
            // Allow hardcoded number of funds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the '.funds' file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of yare_input_.FundAllocations, use a
            // weight of zero.
            bool legal_storage = j < yare_input_.FundAllocations.size();
            double fund_alloc = yare_input_.FundAllocations[j];
            weight = legal_storage ? fund_alloc : 0.0;
            }
*/
        else if(j < static_cast<int>(yare_input_.FundAllocations.size()))
            {
            // TODO ?? Can this be correct? Shouldn't we be looking to
            // the '.funds' file rather than the input class?
            //
            // Allow hardcoded number of funds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the '.funds' file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of yare_input_.FundAllocations, use a
            // weight of zero.
            weight = yare_input_.FundAllocations[j];
            }
        else
            {
            weight = 0.0;
            }

        if(0.0 != weight)
            {
            z += weight * Funds.GetFundInfo(j).ScalarIMF();
            total_sepacct_allocations += weight;
            }
        }

    // Spread over separate account funds only
    if(0.0 != total_sepacct_allocations)
        {
        // Convert from basis points
        z /= 10000.0 * total_sepacct_allocations;
        }

    return z;
}

// TAXATION !! Reconsider unconditional initialization.
// TAXATION !! For 7702A, offer a choice: tables or first principles.

/// Initialize 7702 object.
///
/// This function is called unconditionally, even for CVAT cases that
/// read CVAT corridor factors from a table, for two reasons:
///   - GLP and GSP premium and specamt strategies are always offered;
///   - at least one known product uses GLP as a handy proxy for a
///     minimum no-lapse premium, even when the GPT is not elected.
/// TAXATION !! OTOH, such strategies need not always be offered, and
/// the cited product was simplified to use a table lookup.
///
/// To conform to the practices of certain admin systems, DCV COI
/// rates are stored in a rounded table, but calculations from first
/// principles (GLP, GSP, 7PP, e.g.) use unrounded monthly rates;
/// thus, necessary premium uses both. But this is immaterial.
/// TAXATION !! DATABASE !! The database should offer rounding
/// options; and should this comment be moved to the TUs that
/// implement taxation?

void BasicValues::Init7702()
{
    std::vector<double> Mly7702qc = GetIrc7702QRates();
    double max_coi_rate = database().query<double>(DB_MaxMonthlyCoiRate);
    LMI_ASSERT(0.0 != max_coi_rate);
    max_coi_rate = 1.0 / max_coi_rate;
    assign(Mly7702qc, apply_binary(coi_rate_from_q<double>(), Mly7702qc, max_coi_rate));

    // DCV follows the usual monthiversary mechanics, which involve
    // (optionally) rounding monthly COI rates.
    MlyDcvqc = round_coi_rate()(Mly7702qc);

    // If lmi someday implements VLR, then the current VLR rate on
    // the issue date constitutes a short-term guarantee that must be
    // reflected in the 7702 interest rates (excluding the GLP rate).
    // Until then, assert that VLR is not used, or cannot be used:
    LMI_ASSERT
        (  false == database().query<bool>(DB_AllowVlr)
        || mce_variable_loan_rate != yare_input_.LoanRateType
        );

    // TODO ?? We should avoid reading the rate file again; but
    // the GPT server doesn't initialize a MortalityRates object
    // that would hold those rates. TAXATION !! Rework this.
    std::vector<double> local_mly_charge_add(Length, 0.0);
    if(yare_input_.AccidentalDeathBenefit)
        {
        local_mly_charge_add = GetAdbRates();
        }

    double local_adb_limit = database().query<bool>(DB_AdbIsQAB) ? dblize(AdbLimit) : 0.0;

    // BasicValues::InitialTargetPremium is used only here. For
    // standalone GPT calculations, it's an input field. For
    // illustrations, it's simply initialized to zero: the target
    // premium can't be ascertained yet if the specified amount is
    // to be determined by a strategy, but a downstream call to
    // Irc7702::Initialize7702() takes care of that.

    Irc7702_= std::make_unique<Irc7702>
        (yare_input_.DefinitionOfLifeInsurance
        ,yare_input_.IssueAge
        ,EndtAge
        ,Mly7702qc
        ,i7702_->net_glp()
        ,i7702_->net_gsp()
        ,i7702_->ig()
        ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
        ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
        ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
        ,effective_dbopt_7702(yare_input_.DeathBenefitOption[0], Effective7702DboRop)
        ,dblize(Loads_->annual_policy_fee  (mce_gen_curr))
        ,dblize(Loads_->monthly_policy_fee (mce_gen_curr))
        ,Loads_->specified_amount_load     (mce_gen_curr)
        ,dblize(SpecAmtLoadLimit)
        ,local_mly_charge_add
        ,local_adb_limit
/// TAXATION !! No contemporary authority seems to believe that a
/// change in the premium-tax rate, even if passed through to the
/// policyowner, is a 7702A material change or a GPT adjustment event.
/// These loads should instead reflect the lowest premium-tax rate.
        ,Loads_->target_premium_load_excluding_premium_tax()
        ,Loads_->excess_premium_load_excluding_premium_tax()
        ,InitialTargetPremium
        ,round_min_premium()
        ,round_max_premium()
        ,round_min_specamt()
        ,round_max_specamt()
        ,yare_input_.InforceYear
        ,yare_input_.InforceMonth
        ,yare_input_.InforceGlp
        ,yare_input_.InforceCumulativeGlp
        ,yare_input_.InforceGsp
        ,yare_input_.InforceCumulativeGptPremiumsPaid
        );
}

//============================================================================
void BasicValues::Init7702A()
{
    Irc7702A_= std::make_unique<Irc7702A>
        (DefnLifeIns_
        ,DefnMaterialChange_
        ,false // TODO ?? TAXATION !! Joint life: hardcoded for now.
        ,yare_input_.AvoidMecMethod
        ,true  // TODO ?? TAXATION !! Use table for 7pp: hardcoded for now.
        ,true  // TODO ?? TAXATION !! Use table for NSP: hardcoded for now.
        ,MortalityRates_->SevenPayRates()
        ,MortalityRates_->CvatNspRates()
        ,round_max_premium()
        );
}

/// Public function used for GPT specamt calculation.

currency BasicValues::GetAnnualTgtPrem(int a_year, currency a_specamt) const
{
    return GetModalTgtPrem(a_year, mce_annual, a_specamt);
}

/// Establish up front some values that cannot later change.
///
/// Values set here depend on database_, and thus on yare_input_ and
/// on product_, but not on any other shared_ptr members--so they
/// can reliably be used in initializing those other members.

void BasicValues::SetPermanentInvariants()
{
    database().query_into(DB_MinIssSpecAmt        , MinIssSpecAmt);
    database().query_into(DB_MinIssBaseSpecAmt    , MinIssBaseSpecAmt);
    database().query_into(DB_MinRenlSpecAmt       , MinRenlSpecAmt);
    database().query_into(DB_MinRenlBaseSpecAmt   , MinRenlBaseSpecAmt);
    // Make sure database contents have no excess precision.
    LMI_ASSERT(round_specamt().c(MinIssSpecAmt     ) == MinIssSpecAmt     );
    LMI_ASSERT(round_specamt().c(MinIssBaseSpecAmt ) == MinIssBaseSpecAmt );
    LMI_ASSERT(round_specamt().c(MinRenlSpecAmt    ) == MinRenlSpecAmt    );
    LMI_ASSERT(round_specamt().c(MinRenlBaseSpecAmt) == MinRenlBaseSpecAmt);
    database().query_into(DB_NoLapseDboLvlOnly    , NoLapseDboLvlOnly);
    database().query_into(DB_NoLapseUnratedOnly   , NoLapseUnratedOnly);
    database().query_into(DB_DboChgCanIncrSpecAmt , OptChgCanIncrSA);
    database().query_into(DB_DboChgCanDecrSpecAmt , OptChgCanDecrSA);
    database().query_into(DB_WdDecrSpecAmtDboLvl  , WdDecrSpecAmtDboLvl);
    database().query_into(DB_WdDecrSpecAmtDboInc  , WdDecrSpecAmtDboInc);
    database().query_into(DB_WdDecrSpecAmtDboRop  , WdDecrSpecAmtDboRop);
    database().query_into(DB_MaxIncrAge           , MaxIncrAge);
    database().query_into(DB_WaivePremTaxInt1035  , WaivePmTxInt1035);
    database().query_into(DB_TermIsNotRider       , TermIsNotRider);
    database().query_into(DB_TermForcedConvAge    , TermForcedConvAge);
    database().query_into(DB_TermForcedConvDur    , TermForcedConvDur);
    database().query_into(DB_MinPremType          , MinPremType);
    database().query_into(DB_TgtPremType          , TgtPremType);
    database().query_into(DB_TgtPremFixedAtIssue  , TgtPremFixedAtIssue);
    database().query_into(DB_TgtPremMonthlyPolFee , TgtPremMonthlyPolFee);
    // Make sure database contents have no excess precision.
    LMI_ASSERT(round_gross_premium().c(TgtPremMonthlyPolFee) == TgtPremMonthlyPolFee);
    // Assertion: see comments on GetModalPremTgtFromTable().
    LMI_ASSERT(C0 == TgtPremMonthlyPolFee || oe_modal_table == TgtPremType);
    database().query_into(DB_CurrCoiTable0Limit   , CurrCoiTable0Limit);
    database().query_into(DB_CurrCoiTable1Limit   , CurrCoiTable1Limit);
    LMI_ASSERT(C0                 <= CurrCoiTable0Limit);
    LMI_ASSERT(CurrCoiTable0Limit <= CurrCoiTable1Limit);
    // Make sure database contents have no excess precision.
    LMI_ASSERT(round_specamt().c(CurrCoiTable0Limit) == CurrCoiTable0Limit);
    LMI_ASSERT(round_specamt().c(CurrCoiTable1Limit) == CurrCoiTable1Limit);
    database().query_into(DB_CoiInforceReentry    , CoiInforceReentry);
    database().query_into(DB_MaxWdDed             , MaxWDDed_);
    database().query_into(DB_MaxWdGenAcctValMult  , MaxWdGenAcctValMult);
    database().query_into(DB_MaxWdSepAcctValMult  , MaxWdSepAcctValMult);
    database().query_into(DB_AllowPrefLoan        , AllowPrefLoan);
    database().query_into(DB_MaxLoanDed           , MaxLoanDed_);
    database().query_into(DB_MaxLoanAcctValMult   , MaxLoanAVMult);
    database().query_into(DB_FirstPrefLoanYear    , FirstPrefLoanYear);
    database().query_into(DB_NoLapseMinDur        , NoLapseMinDur);
    database().query_into(DB_NoLapseMinAge        , NoLapseMinAge);
    database().query_into(DB_AdbLimit             , AdbLimit);
    database().query_into(DB_WpLimit              , WpLimit);
    database().query_into(DB_SpecAmtLoadLimit     , SpecAmtLoadLimit);
    // Make sure database contents have no excess precision.
    LMI_ASSERT(round_specamt().c(AdbLimit)         == AdbLimit);
    LMI_ASSERT(round_specamt().c(WpLimit)          == WpLimit);
    LMI_ASSERT(round_specamt().c(SpecAmtLoadLimit) == SpecAmtLoadLimit);
    database().query_into(DB_MinWd                , MinWD);
    database().query_into(DB_WdFee                , WDFee);
    // Make sure database contents have no excess precision.
    LMI_ASSERT(round_withdrawal().c(MinWD) == MinWD);
    LMI_ASSERT(round_withdrawal().c(WDFee) == WDFee);
    database().query_into(DB_WdFeeRate            , WDFeeRate);
    database().query_into(DB_AllowChangeToDbo2    , AllowChangeToDBO2);
    database().query_into(DB_AllowSpecAmtIncr     , AllowSAIncr);
    database().query_into(DB_NoLapseAlwaysActive  , NoLapseAlwaysActive);
    database().query_into(DB_WpChargeMethod       , WaiverChargeMethod);
    database().query_into(DB_AllowCashValueEnh    , AllowCashValueEnh);
    database().query_into(DB_CashValueEnhMult     , CashValueEnhMult);
    database().query_into(DB_LapseIgnoresSurrChg  , LapseIgnoresSurrChg);
    database().query_into(DB_SurrChgOnIncr        , SurrChgOnIncr);
    database().query_into(DB_SurrChgOnDecr        , SurrChgOnDecr);
    LMI_ASSERT(!SurrChgOnIncr); // Surrchg change on increase not supported.
    LMI_ASSERT(!SurrChgOnDecr); // Surrchg change on decrease not supported.

    database().query_into(DB_FreeWdProportion     , FreeWDProportion);

    database().query_into(DB_NaarDiscount         , DBDiscountRate);
    LMI_ASSERT(!contains(DBDiscountRate, -1.0));
// This would be more natural:
//    assign(DBDiscountRate, 1.0 / (1.0 + DBDiscountRate));
// but we avoid it for the nonce because it causes slight regression errors.
// 7702 !! Use the more natural (and more accurate) version once DCV
// has been changed from double to currency.
    assign(DBDiscountRate, 1.0 + DBDiscountRate);
    assign(DBDiscountRate, 1.0 / DBDiscountRate);

    database().query_into(DB_DynamicMandE         , MandEIsDynamic);
    database().query_into(DB_DynamicSepAcctLoad   , SepAcctLoadIsDynamic);

    database().query_into(DB_UnusualCoiBanding    , UseUnusualCOIBanding);

    // 'Unusual' COI banding accommodates a particular idiosyncratic
    // product which has no term rider, so assert the absence of term
    // as a precondition and write simple code for 'unusual' COI
    // banding that ignores term.
    LMI_ASSERT(!(UseUnusualCOIBanding && database().query<bool>(DB_AllowTerm)));

    // Flat extras can be used even with guaranteed issue, e.g., for
    // aviation, occupation, avocation, or foreign travel. Admin
    // systems typically don't distinguish these from medical flats,
    // so neither does lmi--that information wouldn't be available for
    // inforce illustrations.
    //
    // However, table ratings may be restricted, e.g., to medically-
    // underwritten contracts, by setting 'DB_AllowSubstdTable' in a
    // product's database. See the example in 'dbdict.cpp', which
    // varies by yare_input_.GroupUnderwritingType; no restriction is
    // expressly coded here in terms of that field because it is a
    // database axis across which 'DB_AllowSubstdTable' already can
    // vary. Even a rule as apparently reasonable as forbidding table
    // ratings with simplified issue would thus be out of place here:
    // the database can express such a rule handily, and at least one
    // real-world product is known not to follow it. It is important
    // to put aside prior notions of what SI and GI might connote, and
    // realize that for lmi underwriting class is predominantly a
    // database-lookup axis.
    //
    // Table ratings are used only with COI rates for the "Rated"
    // class, so that table multipliers can be applied to a set of COI
    // rates distinct from standard (as is the practice of at least
    // one company). If no such distinction is wanted, then "Rated"
    // and "Standard" can simply point to the same rate table (as
    // happens by default if rates don't vary by underwriting class).
    // The additional "Rated" class induces an extra "Rated" choice in
    // the GUI, which must be selected to enable table ratings; this
    // may be seen as a surprising complication, or as a useful safety
    // feature, but either way no end user has ever objected to it.
    if
        (   mce_table_none != yare_input_.SubstandardTable
        &&  !(database().query<bool>(DB_AllowSubstdTable) && mce_rated == yare_input_.UnderwritingClass)
        )
        {
        alarum() << "Substandard table ratings not permitted." << LMI_FLUSH;
        }

    // SOMEDAY !! WP and ADB shouldn't always be forbidden with table
    // ratings and flat extras. For now, they're not supported due to
    // lack of demand and complexity. These riders are likely to
    // require their own ratings that differ from the base policy's
    // because the insured contingencies differ.
    //
    // Spouse and child riders are not similarly restricted because
    // their rates don't depend on the main insured's health, and the
    // people they cover are unlikely to be underwritten.
    if(is_policy_rated(yare_input_) && yare_input_.WaiverOfPremiumBenefit)
        {
        alarum() << "Substandard waiver of premium not supported." << LMI_FLUSH;
        }
    if(is_policy_rated(yare_input_) && yare_input_.AccidentalDeathBenefit)
        {
        alarum()
            << "Substandard accidental death rider not supported."
            << LMI_FLUSH
            ;
        }

    DefnLifeIns_        = yare_input_.DefinitionOfLifeInsurance;
    DefnMaterialChange_ = yare_input_.DefinitionOfMaterialChange;
    // TAXATION !! For the nonce, input 'DefinitionOfMaterialChange'
    // is ignored without the most-privileged password. In the future,
    // the input class will distinguish CVAT from GPT material-change
    // definitions, and 'DefinitionOfMaterialChange' will be removed.
    if(!global_settings::instance().ash_nazg())
        {
        auto const z = database().query<mcenum_defn_material_change>(DB_CvatMatChangeDefn);
        DefnMaterialChange_ = (mce_gpt == DefnLifeIns_) ? mce_adjustment_event : z;
        }
    database().query_into(DB_Effective7702DboRop, Effective7702DboRop);
    TermIsDbFor7702     = oe_7702_term_is_db == database().query<oenum_7702_term>(DB_TermIsQABOrDb7702 );
    TermIsDbFor7702A    = oe_7702_term_is_db == database().query<oenum_7702_term>(DB_TermIsQABOrDb7702A);
    MaxNAAR             = round_naar().c(yare_input_.MaximumNaar);

    database().query_into(DB_MinPremIntSpread, MinPremIntSpread_);
}

namespace
{
void set_rounding_rule(round_to<double>& functor, rounding_parameters const& z)
{
    functor = round_to<double>(z.decimals(), z.style().value());
}
} // Unnamed namespace.

void BasicValues::SetRoundingFunctors()
{
    set_rounding_rule(round_specamt_           , RoundingRules_->datum("RoundSpecAmt"     ));
    set_rounding_rule(round_death_benefit_     , RoundingRules_->datum("RoundDeathBft"    ));
    set_rounding_rule(round_naar_              , RoundingRules_->datum("RoundNaar"        ));
    set_rounding_rule(round_coi_rate_          , RoundingRules_->datum("RoundCoiRate"     ));
    set_rounding_rule(round_coi_charge_        , RoundingRules_->datum("RoundCoiCharge"   ));
    set_rounding_rule(round_rider_charges_     , RoundingRules_->datum("RoundRiderCharges"));
    set_rounding_rule(round_gross_premium_     , RoundingRules_->datum("RoundGrossPrem"   ));
    set_rounding_rule(round_net_premium_       , RoundingRules_->datum("RoundNetPrem"     ));
    set_rounding_rule(round_interest_rate_     , RoundingRules_->datum("RoundIntRate"     ));
    set_rounding_rule(round_interest_credit_   , RoundingRules_->datum("RoundIntCredit"   ));
    set_rounding_rule(round_withdrawal_        , RoundingRules_->datum("RoundWithdrawal"  ));
    set_rounding_rule(round_loan_              , RoundingRules_->datum("RoundLoan"        ));
    set_rounding_rule(round_interest_rate_7702_, RoundingRules_->datum("RoundIntRate7702" ));
    set_rounding_rule(round_corridor_factor_   , RoundingRules_->datum("RoundCorrFactor"  ));
    set_rounding_rule(round_nsp_rate_7702_     , RoundingRules_->datum("RoundNspRate7702" ));
    set_rounding_rule(round_seven_pay_rate_    , RoundingRules_->datum("RoundSevenPayRate"));
    set_rounding_rule(round_surrender_charge_  , RoundingRules_->datum("RoundSurrCharge"  ));
    set_rounding_rule(round_irr_               , RoundingRules_->datum("RoundIrr"         ));
    set_rounding_rule(round_min_specamt_       , RoundingRules_->datum("RoundMinSpecamt"  ));
    set_rounding_rule(round_max_specamt_       , RoundingRules_->datum("RoundMaxSpecamt"  ));
    set_rounding_rule(round_min_premium_       , RoundingRules_->datum("RoundMinPrem"     ));
    set_rounding_rule(round_max_premium_       , RoundingRules_->datum("RoundMaxPrem"     ));
    set_rounding_rule(round_minutiae_          , RoundingRules_->datum("RoundMinutiae"    ));
}

/// Establish maximum survivorship duration.
///
/// Depends on MortalityRates_ for life-expectancy calculation.

void BasicValues::SetMaxSurvivalDur()
{
    switch(yare_input_.SurviveToType)
        {
        case mce_no_survival_limit:
            {
            MaxSurvivalDur = EndtAge;
            }
            break;
        case mce_survive_to_age:
            {
            MaxSurvivalDur = yare_input_.SurviveToAge - yare_input_.IssueAge;
            }
            break;
        case mce_survive_to_year:
            {
            MaxSurvivalDur = yare_input_.SurviveToYear;
            }
            break;
        case mce_survive_to_expectancy:
            {
            std::vector<double> z(Length);
            assign(z, 1.0 - MortalityRates_->PartialMortalityQ());
            // ET !! In APL, this would be [writing multiplication as '*']
            //   +/*\1-z
            // It would be nice to have a concise representation for that.
            std::partial_sum(z.begin(), z.end(), z.begin(), std::multiplies<double>());
            MaxSurvivalDur = std::accumulate(z.begin(), z.end(), 0.0);
            }
            break;
        }
    LMI_ASSERT(MaxSurvivalDur <= EndtAge);
}

/// Calculate and store actuarial functions for partial mortality.
///
/// Iff partial mortality is used, save qx, tpx, and lx in vectors
/// for use elsewhere in this class and for compositing ledgers.
/// The radix for lx is the number of identical lives that an input
/// cell represents, and qx is forced to unity at the survivorship
/// limit (if any). If partial mortality is not used, then qx is
/// uniformly zero, tpx is one, and lx is the radix.
///
/// tpx and lx both have one more element than qx; dropping the first
/// or last element gives EOY and BOY vectors, respectively.
///
/// Whether a contract continues after its normal maturity date does
/// not matter. It is treated as not expiring on that date because
/// year-end composite values are multiplied by this lx.
///
/// These actuarial functions reflect survivorship only, not lapses.
/// Use AccountValue::InforceLives{E,B}oy() where lapses should be
/// taken into account; cf. Ledger::ZeroInforceAfterLapse().

void BasicValues::set_partial_mortality()
{
    double const inforce_lives = yare_input_.NumberOfIdenticalLives;
    partial_mortality_qx_ .resize(    BasicValues::GetLength());
    partial_mortality_tpx_.resize(1 + BasicValues::GetLength(), 1.0);
    partial_mortality_lx_ .resize(1 + BasicValues::GetLength(), inforce_lives);
    if(yare_input_.UsePartialMortality)
        {
        // The first elements of lx and tpx were set above.
        for(int j = 0; j < BasicValues::GetLength(); ++j)
            {
            partial_mortality_qx_[j] = GetPartMortQ(j);
            double const px = 1.0 - partial_mortality_qx_[j];
            partial_mortality_tpx_[1 + j] = px * partial_mortality_tpx_[j];
            partial_mortality_lx_ [1 + j] = px * partial_mortality_lx_ [j];
            }
        }
}

//============================================================================
double BasicValues::GetPartMortQ(int a_year) const
{
    LMI_ASSERT(a_year <= BasicValues::GetLength());
    if(!yare_input_.UsePartialMortality)
        {
        return 0.0;
        }
    if
        (   MaxSurvivalDur <= a_year
        ||  a_year == BasicValues::GetLength()
        )
        {
        return 1.0;
        }

    double z =
          MortalityRates_->PartialMortalityQ()[a_year]
        * yare_input_.PartialMortalityMultiplier[a_year]
        ;
    return std::max(0.0, std::min(1.0, z));
}

/// Ascertain modal payment for a minimum-premium strategy.
///
/// The term "minimum premium" is overloaded. It may mean the lowest
/// payment that
///  - fulfills a no-lapse guarantee (preventing lapse even if account
///    value would otherwise be negative), or
///  - keeps account value nonnegative (preventing lapse directly).

currency BasicValues::GetModalMinPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    switch(MinPremType)
        {
        case oe_monthly_deduction:
            return GetModalPremMlyDed      (a_year, a_mode, a_specamt);
        case oe_modal_nonmec:
            return GetModalPremMaxNonMec   (a_year, a_mode, a_specamt);
        case oe_modal_table:
            return GetModalPremMinFromTable(a_year, a_mode, a_specamt);
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Ascertain modal payment for a target-premium strategy.

currency BasicValues::GetModalTgtPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    int const target_year = TgtPremFixedAtIssue ? 0 : a_year;
    switch(TgtPremType)
        {
        case oe_monthly_deduction:
            return GetModalPremMlyDed      (target_year, a_mode, a_specamt);
        case oe_modal_nonmec:
            return GetModalPremMaxNonMec   (target_year, a_mode, a_specamt);
        case oe_modal_table:
            return GetModalPremTgtFromTable(target_year, a_mode, a_specamt);
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Calculate premium using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount. Thus, arguments should represent initial
/// premium and mode.

currency BasicValues::GetModalPremMaxNonMec
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    // TAXATION !! No table available if 7PP calculated from first principles.
    double temp = MortalityRates_->SevenPayRates()[0];
    return round_max_premium().c(ldbl_eps_plus_one_times(temp * a_specamt / a_mode));
}

/// Calculate premium using a minimum-premium ratio.
///
/// Only the initial minimum-premium rate is used here, because that's
/// generally fixed at issue. This calculation remains naive in that
/// the initial specified amount may also be fixed at issue, but that
/// choice is left to the caller.

currency BasicValues::GetModalPremMinFromTable
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    return round_max_premium().c
        (ldbl_eps_plus_one_times
            (
                a_specamt * MortalityRates_->MinimumPremiumRates()[0]
            /   a_mode
            )
        );
}

/// Calculate premium using a target-premium ratio.
///
/// Only the initial target-premium rate is used here, because that's
/// generally fixed at issue. This calculation remains naive in that
/// the initial specified amount may also be fixed at issue, but that
/// choice is left to the caller.
///
/// 'TgtPremMonthlyPolFee' is reflected in the result. That's a weird
/// thing to do--GetModalPremMinFromTable() ignores policy fees--but
/// then again the whole notion of a 'TgtPremMonthlyPolFee' is weird,
/// and would have no meaning if it weren't reflected here. And it
/// wouldn't make sense in the other two cases that GetModalTgtPrem()
/// contemplates:
///  - 'oe_monthly_deduction': deductions would naturally include any
///    policy fee;
///  - 'oe_modal_nonmec': 7702A seven-pay premiums are net by their
///    nature; if it is nonetheless desired to add a policy fee to a
///    (conservative) table-derived 7pp, then 'oe_modal_table' should
///    be used instead.
/// Therefore, in those other two cases, 'TgtPremMonthlyPolFee' is
/// asserted to be zero--upstream, so that it'll signal an error even
/// if a target strategy isn't used.

currency BasicValues::GetModalPremTgtFromTable
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    return round_max_premium().c
        (ldbl_eps_plus_one_times
            (
                ( TgtPremMonthlyPolFee * 12.0
                + (a_specamt * MortalityRates_->TargetPremiumRates()[0])
                )
            /   a_mode
            )
        );
}

/// Calculate premium using a tabular proxy for group insurance.

currency BasicValues::GetModalPremProxyTable
    (int         a_year
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ,double      a_table_multiplier
    ) const
{
    return round_gross_premium().c
        (
          a_specamt
        * MortalityRates_->GroupProxyRates()[a_year]
        * a_table_multiplier
        / a_mode
        );
}

/// Calculate premium using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial specified amount and mode.

currency BasicValues::GetModalPremCorridor
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,currency    a_specamt
    ) const
{
    double temp = GetCorridorFactor()[0];
    return round_max_premium().c(ldbl_eps_plus_one_times((a_specamt / temp) / a_mode));
}

//============================================================================
currency BasicValues::GetModalPremGLP
    (int         a_duration
    ,mcenum_mode a_mode
    ,currency    a_bft_amt
    ,currency    a_specamt
    ) const
{
    // TAXATION !! Use GetAnnualTgtPrem() to get target here if needed
    // for GPT reimplementation.
    double z = Irc7702_->CalculateGLP
        (a_duration
        ,dblize(a_bft_amt)
        ,dblize(a_specamt)
        ,Irc7702_->GetLeastBftAmtEver()
        ,effective_dbopt_7702(DeathBfts_->dbopt()[0], Effective7702DboRop)
        );

// TODO ?? TAXATION !! PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium().c(ldbl_eps_plus_one_times(z));
}

//============================================================================
currency BasicValues::GetModalPremGSP
    (int         a_duration
    ,mcenum_mode a_mode
    ,currency    a_bft_amt
    ,currency    a_specamt
    ) const
{
    double z = Irc7702_->CalculateGSP
        (a_duration
        ,dblize(a_bft_amt)
        ,dblize(a_specamt)
        ,Irc7702_->GetLeastBftAmtEver()
        );

// TODO ?? TAXATION !! PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium().c(ldbl_eps_plus_one_times(z));
}

/// Calculate a monthly-deduction discount factor on the fly.
///
/// This factor depends on the general-account rate, which is always
/// specified, even for separate-account-only products.
///
/// This concept is at the same time overelaborate and inadequate.
/// If the crediting rate changes during a policy year, it results in
/// a "pay-deductions" premium that varies between anniversaries, yet
/// may not prevent the contract from lapsing; both those outcomes are
/// likely to frustrate customers.

double BasicValues::mly_ded_discount_factor(int year, mcenum_mode mode) const
{
    LMI_ASSERT(0.0 != mode);
    double spread = 0.0;
    if(mce_monthly != mode)
        {
        spread = MinPremIntSpread_[year] * 1.0 / mode;
        }
    double const z = i_upper_12_over_12_from_i<double>()
        (   yare_input_.GeneralAccountRate[year]
        -   spread
        );
    double const u = 1.0 + std::max
        (z
        ,InterestRates_->GenAcctNetRate
            (mce_gen_guar
            ,mce_monthly_rate
            )[year]
        );
    return 1.0 / u;
}

/// Determine approximate monthly deductions.
///
/// This more or less represents actual monthly deductions, at least
/// for monthly mode on an option B contract, generally favoring
/// sufficiency over minimality, but simplicity most of all.
///
/// For simplicity, certain details are disregarded:
///   - premium loads are often stratified--the rate used here is
///     likely to be the highest that might apply, but deductions at
///     age 99 may well exceed target
///   - account-value loads (including, but not limited to, M&E
///     charges) are presumed to be overcome by interest
///   - the specified amount is taken as a scalar, which might not
///     reflect any value it assumes elsewhere (e.g., as a result of a
///     strategy, or of an initial minimum due to the corridor), and
///     might not be the same as the basis for the accident benefit or
///     the specified-amount load, especially if it includes any term
///     rider amount
///   - any term rider included as specified amount is treated as
///     though its charges equal the base policy's COI rates
///
/// If annual_policy_fee is not zero, then level premium on any mode
/// other than annual cannot precisely cover monthly deductions due
/// to the fee's uneven incidence.
///
/// It might be objected that this function's name suggests that all
/// deductions are monthly, whereas it returns "annual" and "monthly"
/// deductions. Naming rationale: "monthly deduction" is a standard
/// term with an unambiguous meaning; the "annual" policy fee is part
/// of the monthly deduction on anniversary only.
///
/// This function and the modal-premium function implemented in terms
/// of it are intended for use with group UL products for which the
/// employer typically pays the approximate monthly deductions, but
/// may also be used with any UL product when such a payment pattern
/// is desired.
///
/// Returns a pair of deliberately unrounded doubles.

std::pair<double,double> BasicValues::approx_mly_ded
    (int      year
    ,currency specamt
    ) const
{
    double mly_ded = specamt * DBDiscountRate[year];
    mly_ded *= GetBandedCoiRates(mce_gen_curr, specamt)[year];

    if(yare_input_.AccidentalDeathBenefit)
        {
        double const r = MortalityRates_->AdbRates()[year];
        mly_ded += r * std::min(specamt, AdbLimit);
        }

    if(yare_input_.SpouseRider)
        {
        double const r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[year];
        mly_ded += r * yare_input_.SpouseRiderAmount;
        }

    if(yare_input_.ChildRider)
        {
        double const r = MortalityRates_->ChildRiderRates()[year];
        mly_ded += r * yare_input_.ChildRiderAmount;
        }

    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double const r = Loads_->specified_amount_load(mce_gen_curr)[year];
        mly_ded += r * std::min(specamt, SpecAmtLoadLimit);
        }

    mly_ded += dblize(Loads_->monthly_policy_fee(mce_gen_curr)[year]);

    double ann_ded = dblize(Loads_->annual_policy_fee(mce_gen_curr)[year]);

    if(yare_input_.WaiverOfPremiumBenefit)
        {
        double const r = MortalityRates_->WpRates()[year];
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                mly_ded += r * std::min(specamt, WpLimit);
                }
                break;
            case oe_waiver_times_deductions:
                {
                mly_ded *= 1.0 + r;
                ann_ded *= 1.0 + r;
                }
                break;
            }
        }

    double const load = Loads_->target_premium_load_maximum_premium_tax()[year];
    mly_ded /= 1.0 - load;
    ann_ded /= 1.0 - load;

    return std::make_pair(ann_ded, mly_ded);
}

/// Determine approximate monthly deductions split between ee and er.
///
/// This function is similar to approx_mly_ded(), but splits monthly
/// deductions between employee and employer in a plausible way. It is
/// intended for use with group UL products for which the ee and er
/// typically pay these approximate monthly deductions; it and the
/// modal-premium function implemented in terms of it are of narrow
/// applicability and generally not useful with other products.
///
/// Any once-a-year monthly deduction is deliberately ignored for
/// simplicity--if any annual charge is not zero, this function simply
/// sets the deductions it calculates to zero. Once-a-year deductions
/// (e.g., a fee to offset underwriting costs for private placements)
/// are extraordinary, and occur only on products for which a split
/// between ee and er would not be wanted.
///
/// Returns a pair of deliberately unrounded doubles.

std::pair<double,double> BasicValues::approx_mly_ded_ex
    (int      year
    ,currency specamt
    ,currency termamt
    ) const
{
    if(C0 != Loads_->annual_policy_fee(mce_gen_curr)[year])
        {
        return {0.0, 0.0};
        }

    double ee_ded = termamt * DBDiscountRate[year];
    double er_ded = specamt * DBDiscountRate[year];

    ee_ded *= GetCurrentTermRates()[year];
    er_ded *= GetBandedCoiRates(mce_gen_curr, specamt)[year];

    // Paid by er.
    if(yare_input_.AccidentalDeathBenefit)
        {
        double const r = MortalityRates_->AdbRates()[year];
        er_ded += r * std::min(specamt, AdbLimit);
        }

    // Paid by ee.
    if(yare_input_.SpouseRider)
        {
        double const r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[year];
        ee_ded += r * yare_input_.SpouseRiderAmount;
        }

    // Paid by ee.
    if(yare_input_.ChildRider)
        {
        double const r = MortalityRates_->ChildRiderRates()[year];
        ee_ded += r * yare_input_.ChildRiderAmount;
        }

    // Paid by er.
    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double const r = Loads_->specified_amount_load(mce_gen_curr)[year];
        er_ded += r * std::min(specamt, SpecAmtLoadLimit);
        }

    // Paid by er.
    er_ded += dblize(Loads_->monthly_policy_fee(mce_gen_curr)[year]);

    if(yare_input_.WaiverOfPremiumBenefit)
        {
        double const r = MortalityRates_->WpRates()[year];
        switch(WaiverChargeMethod)
            {
            case oe_waiver_times_specamt:
                {
                // Paid by er. (In this case, WP excludes term.)
                er_ded += r * std::min(specamt, WpLimit);
                }
                break;
            case oe_waiver_times_deductions:
                {
                // Paid by ee and er both.
                ee_ded *= 1.0 + r;
                er_ded *= 1.0 + r;
                }
                break;
            }
        }

    double const load = Loads_->target_premium_load_maximum_premium_tax()[year];
    ee_ded /= 1.0 - load;
    er_ded /= 1.0 - load;

    return std::make_pair(ee_ded, er_ded);
}

/// Determine an approximate "pay as you go" modal premium.

currency BasicValues::GetModalPremMlyDed
    (int         year
    ,mcenum_mode mode
    ,currency    specamt
    ) const
{
    auto const deductions = approx_mly_ded(year, specamt);
    double const ann_ded = deductions.first;
    double const mly_ded = deductions.second;
    double const v12 = mly_ded_discount_factor(year, mode);
    double const annuity = (1.0 - std::pow(v12, 12.0 / mode)) / (1.0 - v12);
    return round_min_premium().c(ann_ded + mly_ded * annuity);
}

/// Determine approximate ee and er "pay as you go" modal premiums.

std::pair<currency,currency> BasicValues::GetModalPremMlyDedEx
    (int         year
    ,mcenum_mode mode
    ,currency    specamt
    ,currency    termamt
    ) const
{
    auto const deductions = approx_mly_ded_ex(year, specamt, termamt);
    double const ee_ded = deductions.first;
    double const er_ded = deductions.second;
    double const v12 = DBDiscountRate[year];
    double const annuity = (1.0 - std::pow(v12, 12.0 / mode)) / (1.0 - v12);
    return std::make_pair
        (round_min_premium().c(ee_ded * annuity)
        ,round_min_premium().c(er_ded * annuity)
        );
}

/// Possibly off-anniversary premium to be shown on list bill.
///
/// Implemented in terms of list_bill_premium(), q.v.
///
/// Ascertain deductions at the current age, and then again at the
/// next age iff that is less than the maturity age, otherwise
/// assuming that deductions are zero after maturity. Return their
/// present value, discounted at an interest rate determined as of
/// the list-bill date.
///
/// Any once-a-year monthly deduction is deliberately ignored for
/// simplicity--if any annual charge is not zero, this function simply
/// sets the premiums it calculates to zero. Once-a-year deductions
/// (e.g., a fee to offset underwriting costs for private placements)
/// are extraordinary, and occur only on products for which list bills
/// would not be wanted.

currency BasicValues::GetListBillPremMlyDed
    (int         year
    ,mcenum_mode mode
    ,currency    specamt
    ) const
{
    double const p0 = approx_mly_ded(year, specamt).second;
    int const next_year = 1 + year;
    double const p1 =
        (next_year < GetLength())
        ? approx_mly_ded(next_year, specamt).second
        : 0.0
        ;
    double const z = list_bill_premium
        (p0
        ,p1
        ,mode
        ,yare_input_.EffectiveDate
        ,yare_input_.ListBillDate
        ,mly_ded_discount_factor(year, mode)
        );
    return round_min_premium().c(z);
}

std::pair<currency,currency> BasicValues::GetListBillPremMlyDedEx
    (int         year
    ,mcenum_mode mode
    ,currency    specamt
    ,currency    termamt
    ) const
{
    auto const p0 = approx_mly_ded_ex(year, specamt, termamt);
    int const next_year = 1 + year;
    auto const p1 =
        (next_year < GetLength())
        ? approx_mly_ded_ex(next_year, specamt, termamt)
        : decltype(p0)()
        ;
    double const ee_prem = list_bill_premium
        (p0.first
        ,p1.first
        ,mode
        ,yare_input_.EffectiveDate
        ,yare_input_.ListBillDate
        ,DBDiscountRate[year]
        );
    double const er_prem = list_bill_premium
        (p0.second
        ,p1.second
        ,mode
        ,yare_input_.EffectiveDate
        ,yare_input_.ListBillDate
        ,DBDiscountRate[year]
        );
    return std::make_pair
        (round_min_premium().c(ee_prem)
        ,round_min_premium().c(er_prem)
        );
}

currency BasicValues::GetModalSpecAmtMax(currency annualized_pmt) const
{
    switch(MinPremType)
        {
        case oe_monthly_deduction:
            return GetModalSpecAmtMlyDed(annualized_pmt, mce_annual);
        case oe_modal_nonmec:
            return GetModalSpecAmtMinNonMec(annualized_pmt);
        case oe_modal_table:
            return round_specamt().c
                (
                    annualized_pmt
                /   MortalityRates_->MinimumPremiumRates()[0]
                );
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Argument 'annualized_pmt' is a scalar, intended to represent an
/// initial premium; reason: it's generally inappropriate for a
/// specified-amount strategy to produce a result that varies by
/// duration. It's taken to include 'TgtPremMonthlyPolFee', to make
/// this function the inverse of GetModalPremTgtFromTable(), q.v.

currency BasicValues::GetModalSpecAmtTgt(currency annualized_pmt) const
{
    switch(TgtPremType)
        {
        case oe_monthly_deduction:
            return GetModalSpecAmtMlyDed(annualized_pmt, mce_annual);
        case oe_modal_nonmec:
            return GetModalSpecAmtMinNonMec(annualized_pmt);
        case oe_modal_table:
            return round_specamt().c
                (
                    (annualized_pmt - TgtPremMonthlyPolFee * 12)
                /   MortalityRates_->TargetPremiumRates()[0]
                );
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Calculate specified amount using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount.

currency BasicValues::GetModalSpecAmtMinNonMec(currency annualized_pmt) const
{
    // TAXATION !! No table available if 7PP calculated from first principles.
    return round_min_specamt().c(annualized_pmt / MortalityRates_->SevenPayRates()[0]);
}

//============================================================================
currency BasicValues::GetModalSpecAmtGLP(currency annualized_pmt) const
{
    mcenum_dbopt_7702 const z = effective_dbopt_7702(DeathBfts_->dbopt()[0], Effective7702DboRop);
    return gpt_specamt::CalculateGLPSpecAmt(*this, 0, dblize(annualized_pmt), z);
}

//============================================================================
currency BasicValues::GetModalSpecAmtGSP(currency annualized_pmt) const
{
    return gpt_specamt::CalculateGSPSpecAmt(*this, 0, dblize(annualized_pmt));
}

/// Calculate specified amount using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial premium and mode.
///
/// Corridor factors are stored as double-precision values, but in
/// practice they usually represent whole-number percentages, which
/// are typically printed for each year on policy schedule pages.
/// For GPT, they're defined that way by statute; for CVAT, by
/// convention. Accordingly, round_corridor_factor() rounds to the
/// nearest two decimals for all known products, although it would
/// not be inconceivable to round to three, for example.
///
/// Extra care must be taken in this function, because the specified
/// amount is often rounded to whole dollars (a finer granularity such
/// as cents being used for all other values). Consider this case:
///   $100000 initial payment
///   490% corridor
/// Using 'gnumeric' to view lmi's monthly detail, 4.9 is stored as:
///   4.9000000000000003552713679
///   0 0000000011111111
///   1 2345678901234567 <-- differs in seventeenth significant digit
/// which is presumably the closest representable value. Multiplying
/// that value by 100000 and rounding up to the next dollar gives
/// 490001, but 490000 is desired. To that end, the corridor factor is
/// first scaled by the appropriate power of ten (by one hundred in
/// typical practice) and rounded to integer to recapture the intended
/// value.
///
/// This cannot be implemented as a single call to round_to() because
/// it rounds twice: once to convert a floating-point corridor factor
/// to an exact integer (4.9 --> 490 in the example above), and again
/// after multiplying that integral factor by premium.

currency BasicValues::GetModalSpecAmtCorridor(currency annualized_pmt) const
{
    int const k = round_corridor_factor().decimals();
    double const s = nonstd::power(10, k);
    std::fesetround(FE_TONEAREST);
    double const z = std::nearbyint(s * GetCorridorFactor()[0]);
    return round_min_specamt().c((z * annualized_pmt) / s);
}

/// Calculate specified amount based on salary.
///
/// The result of a salary-based strategy is constrained to be
/// nonnegative, because if 'SalarySpecifiedAmountOffset' is
/// sufficiently large, then specamt would be negative, which cannot
/// make any sense.

currency BasicValues::GetModalSpecAmtSalary(int a_year) const
{
    double z =
          yare_input_.ProjectedSalary[a_year]
        * yare_input_.SalarySpecifiedAmountFactor
        ;
    if(0.0 != yare_input_.SalarySpecifiedAmountCap)
        {
        z = std::min(z, yare_input_.SalarySpecifiedAmountCap);
        }
    z -= yare_input_.SalarySpecifiedAmountOffset;
    return round_specamt().c(std::max(0.0, z));
}

/// In general, strategies linking specamt and premium commute. The
/// "pay deductions" strategy, however, doesn't have a useful analog
/// for determining specamt as a function of initial premium: the
/// contract would almost certainly lapse after one year. Therefore,
/// calling this function elicits an error message. SOMEDAY !! It
/// would be better to disable this strategy in the GUI.

currency BasicValues::GetModalSpecAmtMlyDed(currency, mcenum_mode) const
{
    alarum()
        << "No maximum specified amount is defined for this product."
        << LMI_FLUSH
        ;
    return C0;
}

/// 'Unusual' banding is one particular approach we needed to model.
/// Simpler than the banding method generally used in the industry, it
/// determines a single COI rate from the total specified amount and
/// applies that single rate to the entire NAAR. No layers of coverage
/// are distinguished.

std::vector<double> const& BasicValues::GetBandedCoiRates
    (mcenum_gen_basis rate_basis
    ,currency         a_specamt
    ) const
{
    if(UseUnusualCOIBanding && mce_gen_guar != rate_basis)
        {
        if(CurrCoiTable0Limit <= a_specamt && a_specamt < CurrCoiTable1Limit)
            {
            return MortalityRates_->MonthlyCoiRatesBand1(rate_basis);
            }
        else if(CurrCoiTable1Limit <= a_specamt)
            {
            return MortalityRates_->MonthlyCoiRatesBand2(rate_basis);
            }
        else
            {
            return MortalityRates_->MonthlyCoiRatesBand0(rate_basis);
            }
        }
    else
        {
        return MortalityRates_->MonthlyCoiRatesBand0(rate_basis);
        }
}

/// This forwarding function prevents the 'actuarial_table' module
/// from needing to know about calendar dates and the database.
///
/// At present, exotic lookup methods apply only to current COI rates.
/// An argument could be made for applying them to term rider rates as
/// well.

std::vector<double> BasicValues::GetActuarialTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,int                TableNumber
    ) const
{
    if(DB_CurrCoiTable == TableID && e_reenter_never != CoiInforceReentry)
        {
        return actuarial_table_rates_elaborated
            (TableFile
            ,TableNumber
            ,GetIssueAge()
            ,GetLength()
            ,CoiInforceReentry
            ,yare_input_.InforceYear
            ,duration_ceiling(yare_input_.EffectiveDate, yare_input_.LastCoiReentryDate)
            );
        }
    else
        {
        return actuarial_table_rates
            (TableFile
            ,TableNumber
            ,GetIssueAge()
            ,GetLength()
            );
        }
}

//============================================================================
std::vector<double> BasicValues::GetUnblendedTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ) const
{
    return GetActuarialTable
        (TableFile
        ,TableID
        ,bourn_cast<int>(database().query<int>(TableID))
        );
}

//============================================================================
std::vector<double> BasicValues::GetUnblendedTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,mcenum_gender      gender
    ,mcenum_smoking     smoking
    ) const
{
    database_index index = database().index().gender(gender).smoking(smoking);
    return GetActuarialTable
        (TableFile
        ,TableID
        ,bourn_cast<int>(database().query(TableID, index))
        );
}

//============================================================================
// TODO ?? Better to pass vector as arg rather than return it ?

// This function automatically performs blending by gender and smoking if
// called for. The CanBlend argument tells whether blending is to be
// suppressed for a particular table; its default is to suppress blending.
// For instance, guaranteed COIs might use 80CSO table D for all blends,
// while current COIs reflect the actual blending percentages.
//
// Blending is performed only as called for by the values of the members
// BlendMortSmoking and BlendMortGender of class InputParms and by the
// corresponding arguments.
//
// There are four cases to handle, as can best be seen in a table:
//             female  male  unisex
// smoker         1      1      3
// nonsmoker      1      1      3
// unismoke       2      2      4
//
// The order of blending in the unisex unismoke case makes no difference.
std::vector<double> BasicValues::GetTable
    (std::string const& TableFile
    ,e_database_key     TableID
    ,bool               IsTableValid
    ,EBlend             CanBlendSmoking
    ,EBlend             CanBlendGender
    ) const
{
    if(!IsTableValid)
        {
        return std::vector<double>(GetLength());
        }

    std::string const file_name = AddDataDir(TableFile);

    // To blend by either smoking or gender, both the input must allow
    // it (yare_input_), and the table must allow it (arg: CanBlend);
    // or it must be required by (arg: MustBlend).
    bool BlendSmoking = false;
    switch(CanBlendSmoking)
        {
        case CannotBlend:
            {
            BlendSmoking = false;
            }
            break;
        case CanBlend:
            {
            BlendSmoking = yare_input_.BlendSmoking;
            }
            break;
        case MustBlend:
            {
            BlendSmoking = true;
            }
            break;
        }

    bool BlendGender = false;
    switch(CanBlendGender)
        {
        case CannotBlend:
            {
            BlendGender = false;
            }
            break;
        case CanBlend:
            {
            BlendGender = yare_input_.BlendGender;
            }
            break;
        case MustBlend:
            {
            BlendGender = true;
            }
            break;
        }

    // Case 1: blending is not allowed or not requested--return unblended table
    if
        (
                !BlendSmoking
            &&  !BlendGender

/*
            !CanBlend
        ||  (
                !yare_input_.BlendSmoking
            &&  !yare_input_.BlendGender
            )
*/
        )
        {
        return GetUnblendedTable(file_name, TableID);
        }

    // Any other case needs this
    std::vector<double> BlendedTable;

    // Case 2: blend by smoking only
    // no else because above if returned
    if
        (
            BlendSmoking
        &&  !BlendGender
        )
        {
        std::vector<double> S = GetUnblendedTable
            (file_name
            ,TableID
            ,yare_input_.Gender
            ,mce_smoker
            );
        std::vector<double> N = GetUnblendedTable
            (file_name
            ,TableID
            ,yare_input_.Gender
            ,mce_nonsmoker
            );
        double n = yare_input_.NonsmokerProportion;
        double s = 1.0 - n;
        for(int j = 0; j < GetLength(); ++j)
            {
            BlendedTable.push_back(s * S[j] + n * N[j]);
            }
        }

    // Case 3: blend by gender only
    else if
        (
            !BlendSmoking
        &&  BlendGender
        )
        {
        std::vector<double> F = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,yare_input_.Smoking
            );
        std::vector<double> M = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,yare_input_.Smoking
            );
        double m = yare_input_.MaleProportion;
        double f = 1.0 - m;

/*
    This approach is better actuarial science, but some products'
    specifications do not do this.
        double f_tpx = 1.0;
        double m_tpx = 1.0;
        double tpx;
        double tpx_prev = 1.0;
        for(int j = 0; j < GetLength(); ++j)
            {
            f_tpx *= (1 - F[j]);
            m_tpx *= (1 - M[j]);
            tpx = (f * f_tpx + m * m_tpx);
            BlendedTable.push_back(1.0 - tpx / tpx_prev);
            tpx_prev = tpx;
            }
*/

///*
        for(int j = 0; j < GetLength(); ++j)
            {
            BlendedTable.push_back(f * F[j] + m * M[j]);
            }
//*/
        }

    // Case 4: blend by both smoking and gender
    else if
        (
            BlendSmoking
        &&  BlendGender
        )
        {
        std::vector<double> FS = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,mce_smoker
            );
        std::vector<double> FN = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_female
            ,mce_nonsmoker
            );
        std::vector<double> MS = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,mce_smoker
            );
        std::vector<double> MN = GetUnblendedTable
            (file_name
            ,TableID
            ,mce_male
            ,mce_nonsmoker
            );
        double n = yare_input_.NonsmokerProportion;
        double s = 1.0 - n;
        double m = yare_input_.MaleProportion;
        double f = 1.0 - m;
        for(int j = 0; j < GetLength(); ++j)
            {
            BlendedTable.push_back
                (   f * (s * FS[j] + n * FN[j])
                +   m * (s * MS[j] + n * MN[j])
                );
/* Equivalently we could do this:
                (   s * (f * FS[j] + m * MS[j])
                +   n * (f * FN[j] + m * MN[j])
                );
*/
            }
        }
    else
        {
        alarum() << "Invalid mortality blending." << LMI_FLUSH;
        }

    return BlendedTable;
}

//============================================================================
// TAXATION !! Resolve these issues:
// TODO ?? This might be reworked to go through class Irc7702 all the time;
// at least it shouldn't refer to the input class.
// TODO ?? The profusion of similar names should be trimmed.
std::vector<double> const& BasicValues::GetCorridorFactor() const
{
    switch(yare_input_.DefinitionOfLifeInsurance)
        {
        case mce_cvat:
            {
            return MortalityRates_->CvatCorridorFactors();
            }
        case mce_gpt:
            {
            return Irc7702_->Corridor();
            }
        case mce_noncompliant:
            {
            Non7702CompliantCorridor = std::vector<double>(GetLength(), 1.0);
            return Non7702CompliantCorridor;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

// potential inlines

std::vector<double> const& BasicValues::GetMlyDcvqc() const
{
    return MlyDcvqc;
}

std::vector<double> BasicValues::GetCvatCorridorFactors() const
{
    return cvat_corridor_factors
        (product()
        ,database()
        ,GetIssueAge()
        ,GetLength()
        );
}

// Only current (hence midpoint) COI and term rates are blended.

std::vector<double> BasicValues::GetCurrCOIRates0() const
{
    return GetTable
        (product().datum("CurrCOIFilename")
        ,DB_CurrCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetCurrCOIRates1() const
{
    static constexpr double dbl_inf = std::numeric_limits<double>::infinity();
    static const currency inf = from_cents(dbl_inf);
    return GetTable
        (product().datum("CurrCOIFilename")
        ,DB_CurrCoiTable1
        ,CurrCoiTable0Limit < inf
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetCurrCOIRates2() const
{
    static constexpr double dbl_inf = std::numeric_limits<double>::infinity();
    static const currency inf = from_cents(dbl_inf);
    return GetTable
        (product().datum("CurrCOIFilename")
        ,DB_CurrCoiTable2
        ,CurrCoiTable1Limit < inf
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGuarCOIRates() const
{
    return GetTable
        (product().datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        );
}

std::vector<double> BasicValues::GetSmokerBlendedGuarCOIRates() const
{
    return GetTable
        (product().datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetWpRates() const
{
    return GetTable
        (product().datum("WPFilename")
        ,DB_WpTable
        ,database().query<bool>(DB_AllowWp)
        );
}

std::vector<double> BasicValues::GetAdbRates() const
{
    return GetTable
        (product().datum("ADDFilename")
        ,DB_AdbTable
        ,database().query<bool>(DB_AllowAdb)
        );
}

std::vector<double> BasicValues::GetChildRiderRates() const
{
    return GetTable
        (product().datum("ChildRiderFilename")
        ,DB_ChildRiderTable
        ,database().query<bool>(DB_AllowChildRider)
        );
}

std::vector<double> BasicValues::GetCurrentSpouseRiderRates() const
{
    if(!database().query<bool>(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(product().datum("CurrSpouseRiderFilename"))
        ,database().query<int>(DB_SpouseRiderTable)
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}

std::vector<double> BasicValues::GetGuaranteedSpouseRiderRates() const
{
    if(!database().query<bool>(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(product().datum("GuarSpouseRiderFilename"))
        ,database().query<int>(DB_SpouseRiderGuarTable)
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}

std::vector<double> BasicValues::GetCurrentTermRates() const
{
    return GetTable
        (product().datum("CurrTermFilename")
        ,DB_TermTable
        ,database().query<bool>(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGuaranteedTermRates() const
{
    return GetTable
        (product().datum("GuarTermFilename")
        ,DB_GuarTermTable
        ,database().query<bool>(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetGroupProxyRates() const
{
    return GetTable
        (product().datum("GroupProxyFilename")
        ,DB_GroupProxyRateTable
        );
}

std::vector<double> BasicValues::GetSevenPayRates() const
{
    return irc_7702A_7pp
        (product()
        ,database()
        ,GetIssueAge()
        ,GetLength()
        );
}

std::vector<double> BasicValues::GetMinPremRates() const
{
    return GetTable
        (product().datum("MinPremFilename")
        ,DB_MinPremTable
        ,oe_modal_table == MinPremType
        );
}

std::vector<double> BasicValues::GetTgtPremRates() const
{
    return GetTable
        (product().datum("TgtPremFilename")
        ,DB_TgtPremTable
        ,oe_modal_table == TgtPremType
        );
}

std::vector<double> BasicValues::GetIrc7702QRates() const
{
    return irc_7702_q
        (product()
        ,database()
        ,GetIssueAge()
        ,GetLength()
        );
}

std::vector<double> BasicValues::GetPartialMortalityRates() const
{
    return GetTable
        (product().datum("PartialMortalityFilename")
        ,DB_PartialMortTable
        ,true
        ,CannotBlend
        ,CanBlend
        );
}

std::vector<double> BasicValues::GetSubstdTblMultTable() const
{
    if(0 == database().query<int>(DB_SubstdTableMultTable))
        {
        return std::vector<double>(GetLength(), 1.0);
        }

    return GetTable
        (product().datum("SubstdTblMultFilename")
        ,DB_SubstdTableMultTable
        );
}

std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const
{
    return GetTable
        (product().datum("CurrSpecAmtLoadFilename")
        ,DB_CurrSpecAmtLoadTable
        ,0 != database().query<int>(DB_CurrSpecAmtLoadTable)
        );
}

std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const
{
    return GetTable
        (product().datum("GuarSpecAmtLoadFilename")
        ,DB_GuarSpecAmtLoadTable
        ,0 != database().query<int>(DB_GuarSpecAmtLoadTable)
        );
}
