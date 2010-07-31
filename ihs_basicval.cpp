// Basic values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "basic_values.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "death_benefits.hpp"
#include "et_vector.hpp"
#include "fund_data.hpp"
#include "global_settings.hpp"
#include "ieee754.hpp"           // ldbl_eps_plus_one()
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_x_type.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "math_functors.hpp"
#include "mc_enum_types_aux.hpp" // mc_str()
#include "miscellany.hpp"        // ios_out_trunc_binary()
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "product_data.hpp"
#include "rounding_rules.hpp"
#include "stratified_charges.hpp"
#include "surrchg_rates.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <cmath>                 // std::pow()
#include <cstring>               // std::strlen(), std::strncmp()
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

//============================================================================
BasicValues::BasicValues(Input const& input)
    :Input_              (new Input(input))
    ,yare_input_         (input)
    ,DefnLifeIns_        (mce_cvat)
    ,DefnMaterialChange_ (mce_unnecessary_premium)
    ,Equiv7702DBO3       (mce_option1_for_7702)
    ,MaxWDDed_           (mce_twelve_times_last)
    ,MaxLoanDed_         (mce_twelve_times_last)
    ,StateOfJurisdiction_(mce_s_CT)
    ,StateOfDomicile_    (mce_s_CT)
    ,PremiumTaxState_    (mce_s_CT)
{
    Init();
}

//============================================================================
// TODO ?? Not for general use--use for GPT server only. This is bad design.
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
    :Input_              (new Input)
    ,yare_input_         (*Input_)
    ,DefnLifeIns_        (mce_cvat)
    ,DefnMaterialChange_ (mce_unnecessary_premium)
    ,Equiv7702DBO3       (a_DBOptFor7702)
    ,MaxWDDed_           (mce_twelve_times_last)
    ,MaxLoanDed_         (mce_twelve_times_last)
    ,StateOfJurisdiction_(a_StateOfJurisdiction)
    ,StateOfDomicile_    (a_StateOfJurisdiction)
    ,PremiumTaxState_    (a_StateOfJurisdiction)
    ,InitialTargetPremium(a_TargetPremium)
{
    Input* kludge_input = new Input;

    (*kludge_input)["IssueAge"         ] = value_cast<std::string>(a_IssueAge)         ;
    (*kludge_input)["RetirementAge"    ] = value_cast<std::string>(a_IssueAge)         ;
    (*kludge_input)["Gender"           ] = value_cast<std::string>(a_Gender)           ;
    (*kludge_input)["Smoking"          ] = value_cast<std::string>(a_Smoker)           ;
    (*kludge_input)["UnderwritingClass"] = value_cast<std::string>(a_UnderwritingClass);
    if(a_AdbInForce)
        {
        (*kludge_input)["Status[0].HasADD"] = "Yes";
        }
    else
        {
        (*kludge_input)["Status[0].HasADD"] = "No";
        }
    (*kludge_input)["GroupUnderwritingType"     ] = value_cast<std::string>(a_UnderwritingBasis);
    (*kludge_input)["ProductName"               ] = a_ProductName;
    (*kludge_input)["PremiumTaxState"           ] = mc_str(a_StateOfJurisdiction);
    (*kludge_input)["DefinitionOfLifeInsurance" ] = "GPT";
    (*kludge_input)["DefinitionOfMaterialChange"] = "GPT adjustment event";

    (*kludge_input)["SpecifiedAmount"   ] = value_cast<std::string>(a_FaceAmount);

    mce_dbopt const z
        (mce_option1_for_7702 == a_DBOptFor7702 ? mce_option1
        :mce_option2_for_7702 == a_DBOptFor7702 ? mce_option2
        :throw std::runtime_error("Unexpected DB option.")
        );
    (*kludge_input)["DeathBenefitOption"] = mce_dbopt(z).str();

    // TODO ?? EGREGIOUS_DEFECT Redesign this function instead.
    const_cast<Input&>(*Input_) = *kludge_input;

    GPTServerInit();
}

//============================================================================
BasicValues::~BasicValues()
{
}

//============================================================================
void BasicValues::Init()
{
    ProductData_.reset(new product_data(yare_input_.ProductName));
    // bind to policy form
    //      one filename that brings in all the rest incl database?
    // controls as ctor arg?
    // validate input in context of this policy form

    // TRICKY !! We need the database to look up whether ALB or ANB should
    // be used, in case we need to determine issue age from DOB. But issue
    // age is a database lookup key, so it can change what we looked up in
    // the database. To resolve this circularity, we first set the database
    // assuming that the age is correct, then ascertain whether ALB or ANB
    // is used, then reset the database, then recalculate the age. If any
    // circularity
    // remains, it will be detected and an error message given when we look
    // up the ALB/ANB switch using product_database::Query(int), which
    // restricts looked-up values to scalars that vary across no database axis.

    Database_.reset(new product_database(yare_input_));

    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    if
        (   !Database_->Query(DB_StateApproved)
        &&  !global_settings::instance().ash_nazg()
        &&  !global_settings::instance().regression_testing()
        )
        {
        fatal_error()
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
    HOPEFULLY(IssueAge < 100);
    HOPEFULLY(RetAge <= 100);
    HOPEFULLY(yare_input_.RetireesCanEnroll || IssueAge <= RetAge);

    // The database class constrains maturity age to be scalar.
    EndtAge = static_cast<int>(Database_->Query(DB_MaturityAge));
    Length = EndtAge - IssueAge;

    LedgerType_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (Database_->Query(DB_LedgerType))
            )
        ;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(GetLedgerType());

    if(IssueAge < Database_->Query(DB_MinIssAge))
        {
        throw x_product_rule_violated
            (
            std::string("Issue age less than minimum")
            );
        }
    if(Database_->Query(DB_MaxIssAge) < IssueAge)
        {
        throw x_product_rule_violated
            (
            std::string("Issue age greater than maximum")
            );
        }
    FundData_.reset(new FundData(AddDataDir(ProductData_->datum("FundFilename"))));
    RoundingRules_.reset
        (new rounding_rules(AddDataDir(ProductData_->datum("RoundingFilename")))
        );
    SetRoundingFunctors();
    StratifiedCharges_.reset
        (new stratified_charges(AddDataDir(ProductData_->datum("TierFilename")))
        );
    SpreadFor7702_.assign
        (Length
        ,StratifiedCharges_->minimum_tiered_spread_for_7702()
        );

    // Multilife contracts will need a vector of mortality-rate objects.

    // Mortality and interest rates require database and rounding.
    // Interest rates require tiered data and 7702 spread.
    MortalityRates_.reset(new MortalityRates (*this));
    InterestRates_ .reset(new InterestRates  (*this));
    // Surrender-charge rates will eventually require mortality rates.
    SurrChgRates_  .reset(new SurrChgRates   (*Database_));
    DeathBfts_     .reset(new death_benefits (GetLength(), yare_input_));
    // Outlay requires only input; it might someday use interest rates.
    Outlay_        .reset(new modal_outlay   (yare_input_));
    SetPremiumTaxParameters();
    Loads_         .reset(new Loads          (*this));

    // The target premium can't be ascertained yet if specamt is
    // determined by a strategy.
    InitialTargetPremium = 0.0;

    SetPermanentInvariants();

    Init7702();
    Init7702A();
}

//============================================================================
// TODO ??  Not for general use--use for GPT server only, for now--refactor later
void BasicValues::GPTServerInit()
{
    ProductData_.reset(new product_data(yare_input_.ProductName));
    Database_.reset(new product_database(yare_input_));

    IssueAge = yare_input_.IssueAge;
    RetAge   = yare_input_.RetirementAge;
    HOPEFULLY(IssueAge < 100);
    HOPEFULLY(RetAge <= 100);
    HOPEFULLY(yare_input_.RetireesCanEnroll || IssueAge <= RetAge);

    StateOfJurisdiction_ = yare_input_.StateOfJurisdiction;
    PremiumTaxState_     = yare_input_.PremiumTaxState    ;

    // The database class constrains maturity age to be scalar.
    EndtAge = static_cast<int>(Database_->Query(DB_MaturityAge));
    Length = EndtAge - IssueAge;

    LedgerType_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (Database_->Query(DB_LedgerType))
            )
        ;
    IsSubjectToIllustrationReg_ = is_subject_to_ill_reg(GetLedgerType());

    if(IssueAge < Database_->Query(DB_MinIssAge))
        {
        throw x_product_rule_violated
            (
            std::string("Issue age less than minimum")
            );
        }
    if(Database_->Query(DB_MaxIssAge) < IssueAge)
        {
        throw x_product_rule_violated
            (
            std::string("Issue age greater than maximum")
            );
        }
//  FundData_       = new FundData
//      (AddDataDir(ProductData_->datum("FundFilename"))
//      );
    RoundingRules_.reset
        (new rounding_rules(AddDataDir(ProductData_->datum("RoundingFilename")))
        );
    SetRoundingFunctors();
    StratifiedCharges_.reset
        (new stratified_charges(AddDataDir(ProductData_->datum("TierFilename")))
        );

    // Requires database.
//  MortalityRates_.reset(new MortalityRates (*this));
//  InterestRates_ .reset(new InterestRates  (*this));
    // Will require mortality rates eventually.
//  SurrChgRates_  .reset(new SurrChgRates   (Database_));
//  DeathBfts_     .reset(new death_benefits (GetLength(), yare_input_));
    // Outlay requires only input; it might someday use interest rates.
//  Outlay_        .reset(new modal_outlay   (yare_input_));
    SetPremiumTaxParameters();
    Loads_         .reset(new Loads          (*this));

    SetPermanentInvariants();

    Init7702();
}

//============================================================================
// TODO ?? Does this belong in the funds class? Consider merging it
// with code in AccountValue::SetInitialValues().
double BasicValues::InvestmentManagementFee() const
{
    if(!Database_->Query(DB_AllowSepAcct))
        {
        return 0.0;
        }

    if(yare_input_.OverrideFundManagementFee)
        {
        return yare_input_.InputFundManagementFee / 10000.0L;
        }

    double z = 0.0;
    double total_sepacct_allocations = 0.0;
    FundData const& Funds = *FundData_;

    for(int j = 0; j < Funds.GetNumberOfFunds(); j++)
        {
        double weight;
        // If average of all funds, then use equal weights, but
        // disregard "custom" funds--that is, set their weights to
        // zero. Custom funds are those whose name begins with "Custom".
        // Reason: "average" means average of the normally-available
        // funds only.
        if(yare_input_.UseAverageOfAllFunds)
            {
            char const s[] = "Custom";
            std::size_t n = std::strlen(s);
            bool ignore = 0 == std::strncmp
                (s
                ,Funds.GetFundInfo(j).ShortName().c_str()
                ,n
                );
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

/// Initialize 7702 object.
///
/// This function is called unconditionally, even for CVAT cases that
/// read CVAT corridor factors from a table, for two reasons:
///   - GLP and GSP premium and specamt strategies are always offered;
///   - at least one known product uses GLP as a handy proxy for a
///     minimum no-lapse premium, even when the GPT is not elected.
///
/// To conform to the practices of certain admin systems, DCV COI
/// rates are stored in a rounded table, but calculations from first
/// principles (GLP, GSP, 7PP, e.g.) use unrounded monthly rates;
/// thus, necessary premium uses both. But this is immaterial.

void BasicValues::Init7702()
{
    Mly7702qc = GetIRC7702Rates();
    double max_coi_rate = Database_->Query(DB_MaxMonthlyCoiRate);
    LMI_ASSERT(0.0 != max_coi_rate);
    max_coi_rate = 1.0 / max_coi_rate;
    assign(Mly7702qc, apply_binary(coi_rate_from_q<double>(), Mly7702qc, max_coi_rate));

    MlyDcvqc = Mly7702qc;
    std::transform
        (MlyDcvqc.begin()
        ,MlyDcvqc.end()
        ,MlyDcvqc.begin()
        ,round_coi_rate()
        );
    if(contains(yare_input_.Comments, "idiosyncrasy_dcvq"))
        {
        std::ostringstream oss;
        oss
            << yare_input_.ProductName
            << '_'
            << mc_str(yare_input_.Gender)
            << '_'
            << mc_str(yare_input_.Smoking)
            << ".dcvq"
            << configurable_settings::instance().spreadsheet_file_extension()
            ;
        std::ofstream os(oss.str().c_str(), ios_out_trunc_binary());
        int const minimum_age  = static_cast<int>(Database_->Query(DB_MinIssAge  ));
        int const maturity_age = static_cast<int>(Database_->Query(DB_MaturityAge));
        if(minimum_age != yare_input_.IssueAge)
            {
            warning()
                << "Issue age is "
                << yare_input_.IssueAge
                << ", but the minimum is "
                << minimum_age
                << ". Use the minimum instead."
                << LMI_FLUSH
                ;
            }
        for(int j = 0; j < maturity_age - minimum_age; ++j)
            {
            std::string s = value_cast<std::string>(MlyDcvqc[j]);
            os << j + minimum_age << '\t' << s << '\n';
            }
        }

    // Monthly guar net int for 7702, with 4 or 6% min, is
    //   greater of {4%, 6%} and annual guar int rate
    //   less 7702 spread
    // TODO ?? We need to subtract other things too, e.g. comp (sometimes)...
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> guar_int;
    Database_->Query(guar_int, DB_GuarInt);
/*
    switch(yare_input_.LoanRateType)
        {
        case mce_fixed_loan_rate:
            {
            std::vector<double> gross_loan_rate;
            Database_->Query(gross_loan_rate, DB_FixedLoanRate);
            std::vector<double> guar_loan_spread;
            Database_->Query(guar_loan_spread, DB_GuarRegLoanSpread);
            // ET !! std::vector<double> guar_loan_rate = gross_loan_rate - guar_loan_spread;
            std::vector<double> guar_loan_rate(Length);
            std::transform
                (gross_loan_rate.begin()
                ,gross_loan_rate.end()
                ,guar_loan_spread.begin()
                ,guar_loan_rate.begin()
                ,std::minus<double>()
                );
            // ET !! guar_int = max(guar_int, guar_loan_spread);
            // TODO ?? But that looks incorrect when written clearly!
            // Perhaps this old comment:
            //   APL: guar_int gets guar_int max gross_loan_rate - guar_loan_spread
            // suggests the actual intention.
            std::transform
                (guar_int.begin()
                ,guar_int.end()
                ,guar_loan_spread.begin()
                ,guar_int.begin()
                ,greater_of<double>()
                );
            }
            break;
        case mce_variable_loan_rate:
            {
            // do nothing
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << yare_input_.LoanRateType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
*/

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(.04, guar_int) - SpreadFor7702_);
    Mly7702iGlp.assign(Length, 0.04);
    std::transform
        (guar_int.begin()
        ,guar_int.end()
        ,Mly7702iGlp.begin()
        ,Mly7702iGlp.begin()
        ,greater_of<double>()
        );
    std::transform
        (Mly7702iGlp.begin()
        ,Mly7702iGlp.end()
        ,SpreadFor7702_.begin()
        ,Mly7702iGlp.begin()
        ,std::minus<double>()
        );
    std::transform
        (Mly7702iGlp.begin()
        ,Mly7702iGlp.end()
        ,Mly7702iGlp.begin()
        ,i_upper_12_over_12_from_i<double>()
        );

    // ET !! Mly7702iGsp = i_upper_12_over_12_from_i(max(.06, guar_int) - SpreadFor7702_);
    Mly7702iGsp.assign(Length, 0.06);
    std::transform
        (guar_int.begin()
        ,guar_int.end()
        ,Mly7702iGsp.begin()
        ,Mly7702iGsp.begin()
        ,greater_of<double>()
        );
    std::transform
        (Mly7702iGsp.begin()
        ,Mly7702iGsp.end()
        ,SpreadFor7702_.begin()
        ,Mly7702iGsp.begin()
        ,std::minus<double>()
        );
    std::transform
        (Mly7702iGsp.begin()
        ,Mly7702iGsp.end()
        ,Mly7702iGsp.begin()
        ,i_upper_12_over_12_from_i<double>()
        );

    Database_->Query(Mly7702ig, DB_NaarDiscount);

    // TODO ?? We should avoid reading the rate file again; but
    // the GPT server doesn't initialize a MortalityRates object
    // that would hold those rates.
    std::vector<double> local_mly_charge_add(Length, 0.0);
    if(yare_input_.AccidentalDeathBenefit)
        {
        local_mly_charge_add = GetAdbRates();
        }

    Irc7702_.reset
        (new Irc7702
            (*this
            ,yare_input_.DefinitionOfLifeInsurance
            ,yare_input_.IssueAge
            ,EndtAge
            ,Mly7702qc
            ,Mly7702iGlp
            ,Mly7702iGsp
            ,Mly7702ig
            ,SpreadFor7702_
            ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
            ,yare_input_.SpecifiedAmount[0] + yare_input_.TermRiderAmount
            ,effective_dbopt_7702(yare_input_.DeathBenefitOption[0], Equiv7702DBO3)
            // TODO ?? Using the guaranteed basis for all the following should
            // be an optional behavior.
            ,Loads_->annual_policy_fee    (mce_gen_curr)
            ,Loads_->monthly_policy_fee   (mce_gen_curr)
            ,Loads_->specified_amount_load(mce_gen_curr)
            ,SpecAmtLoadLimit
            ,local_mly_charge_add
            ,AdbLimit
            ,Loads_->target_premium_load_7702_excluding_premium_tax()
            ,Loads_->excess_premium_load_7702_excluding_premium_tax()
            ,InitialTargetPremium
            ,round_min_premium()
            ,round_max_premium()
            ,round_min_specamt()
            ,round_max_specamt()
            )
        );
}

//============================================================================
void BasicValues::Init7702A()
{
    int magic = 0; // TODO ?? A kludge.
    Irc7702A_.reset
        (new Irc7702A
            (magic
            ,DefnLifeIns_
            ,DefnMaterialChange_
            ,false // TODO ?? Joint life: hardcoded for now.
            ,yare_input_.AvoidMecMethod
            ,true  // TODO ?? Use table for 7pp: hardcoded for now.
            ,true  // TODO ?? Use table for NSP: hardcoded for now.
            ,MortalityRates_->SevenPayRates()
            ,MortalityRates_->CvatNspRates()
            ,round_max_premium()
            )
        );
}

//============================================================================
// Needed for guideline premium.
// TODO ?? a_dbopt is ignored for now, but some product designs will need it.
double BasicValues::GetTgtPrem
    (int          a_year
    ,double       a_specamt
    ,mcenum_dbopt // a_dbopt Unused for now.
    ,mcenum_mode  a_mode
    ) const
{
    if(Database_->Query(DB_TgtPremFixedAtIssue))
        {
        if(0 == a_year)
            {
            InitialTargetPremium = GetModalTgtPrem
                (a_year
                ,a_mode
                ,a_specamt
                );
            }
            return InitialTargetPremium;
        }
    else
        {
        return GetModalTgtPrem
            (a_year
            ,a_mode
            ,a_specamt
            );
        }
}

//============================================================================
void BasicValues::SetPermanentInvariants()
{
    MinRenlBaseFace     = Database_->Query(DB_MinRenlBaseSpecAmt   );
    MinRenlFace         = Database_->Query(DB_MinRenlSpecAmt       );
    NoLapseOpt1Only     = Database_->Query(DB_NoLapseDbo1Only      );
    NoLapseUnratedOnly  = Database_->Query(DB_NoLapseUnratedOnly   );
    OptChgCanIncrSA     = Database_->Query(DB_DboChgCanIncrSpecAmt );
    OptChgCanDecrSA     = Database_->Query(DB_DboChgCanDecrSpecAmt );
    WDCanDecrSADBO1     = Database_->Query(DB_WdCanDecrSpecAmtDbo1 );
    WDCanDecrSADBO2     = Database_->Query(DB_WdCanDecrSpecAmtDbo2 );
    WDCanDecrSADBO3     = Database_->Query(DB_WdCanDecrSpecAmtDbo3 );
    MaxIncrAge          = static_cast<int>(Database_->Query(DB_MaxIncrAge));
    WaivePmTxInt1035    = Database_->Query(DB_WaivePremTaxInt1035  );
    AllowTerm           = Database_->Query(DB_AllowTerm            );
    ExpPerKLimit        = Database_->Query(DB_ExpSpecAmtLimit      );
    MaxWDDed_           = static_cast<mcenum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxWdDed)));
    MaxWDAVMult         = Database_->Query(DB_MaxWdAcctValMult     );
    MaxLoanDed_         = static_cast<mcenum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxLoanDed)));
    MaxLoanAVMult       = Database_->Query(DB_MaxLoanAcctValMult   );
    NoLapseMinDur       = static_cast<int>(Database_->Query(DB_NoLapseMinDur));
    NoLapseMinAge       = static_cast<int>(Database_->Query(DB_NoLapseMinAge));
    MinSpecAmt          = Database_->Query(DB_MinSpecAmt           );
    AdbLimit            = Database_->Query(DB_AdbLimit             );
    WpLimit             = Database_->Query(DB_WpMax                );
    SpecAmtLoadLimit    = Database_->Query(DB_SpecAmtLoadLimit     );
    MinWD               = Database_->Query(DB_MinWd                );
    WDFee               = Database_->Query(DB_WdFee                );
    WDFeeRate           = Database_->Query(DB_WdFeeRate            );
    AllowChangeToDBO2   = Database_->Query(DB_AllowChangeToDbo2    );
    AllowSAIncr         = Database_->Query(DB_AllowSpecAmtIncr     );
    NoLapseAlwaysActive = Database_->Query(DB_NoLapseAlwaysActive  );
    WaiverChargeMethod  = static_cast<oenum_waiver_charge_method>(static_cast<int>(Database_->Query(DB_WpChargeMethod)));
    LapseIgnoresSurrChg = Database_->Query(DB_LapseIgnoresSurrChg  );
    SurrChgOnIncr       = Database_->Query(DB_SurrChgOnIncr        );
    SurrChgOnDecr       = Database_->Query(DB_SurrChgOnDecr        );
    HOPEFULLY(!SurrChgOnDecr); // Surrchg change on decrease not supported.

    Database_->Query(FreeWDProportion, DB_FreeWdProportion);

    Database_->Query(DBDiscountRate, DB_NaarDiscount);
    LMI_ASSERT(!contains(DBDiscountRate, -1.0));
// This would be more natural:
//    assign(DBDiscountRate, 1.0 / (1.0 + DBDiscountRate));
// but we avoid it for the nonce because it causes slight regression errors.
    assign(DBDiscountRate, 1.0 + DBDiscountRate);
    assign(DBDiscountRate, 1.0 / DBDiscountRate);

    Database_->Query(AssetComp , DB_AssetComp);
    Database_->Query(CompTarget, DB_CompTarget);
    Database_->Query(CompExcess, DB_CompExcess);

    MandEIsDynamic      = Database_->Query(DB_DynamicMandE         );
    SepAcctLoadIsDynamic= Database_->Query(DB_DynamicSepAcctLoad   );

    UseUnusualCOIBanding= Database_->Query(DB_UnusualCoiBanding    );

    // 'Unusual' COI banding accommodates a particular idiosyncratic
    // product which has no term rider and doesn't permit experience
    // rating, so we assert those preconditions and write simple code
    // for 'unusual' COI banding that ignores those features.
    HOPEFULLY(!(UseUnusualCOIBanding && yare_input_.UseExperienceRating));
    HOPEFULLY(!(UseUnusualCOIBanding && AllowTerm));

    // Table ratings can arise only from medical underwriting.
    // However, flat extras can be used even with guaranteed issue,
    // e.g. for aviation, occupation, avocation, or foreign travel.
    if
        (   mce_table_none != yare_input_.SubstandardTable
        &&  mce_medical    != yare_input_.GroupUnderwritingType
        )
        {
        fatal_error()
            << "Substandard table ratings require medical underwriting."
            << LMI_FLUSH
            ;
        }

    // Spouse and child riders are not similarly tested because
    // their rates shouldn't depend on the main insured's health.
    if(is_policy_rated(yare_input_) && yare_input_.WaiverOfPremiumBenefit)
        {
        fatal_error()
            << "Substandard waiver of premium not supported."
            << LMI_FLUSH
            ;
        }
    if(is_policy_rated(yare_input_) && yare_input_.AccidentalDeathBenefit)
        {
        fatal_error()
            << "Substandard accidental death rider not supported."
            << LMI_FLUSH
            ;
        }

    DefnLifeIns_        = yare_input_.DefinitionOfLifeInsurance;
    DefnMaterialChange_ = yare_input_.DefinitionOfMaterialChange;
    Equiv7702DBO3       = static_cast<mcenum_dbopt_7702>(static_cast<int>(Database_->Query(DB_Equiv7702Dbo3)));
    MaxNAAR             = yare_input_.MaximumNaar;

    Database_->Query(MinPremIntSpread_, DB_MinPremIntSpread);

    SetMaxSurvivalDur();
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
    set_rounding_rule(round_specamt_           , RoundingRules_->datum("RoundSpecAmt"    ));
    set_rounding_rule(round_death_benefit_     , RoundingRules_->datum("RoundDeathBft"   ));
    set_rounding_rule(round_naar_              , RoundingRules_->datum("RoundNaar"       ));
    set_rounding_rule(round_coi_rate_          , RoundingRules_->datum("RoundCoiRate"    ));
    set_rounding_rule(round_coi_charge_        , RoundingRules_->datum("RoundCoiCharge"  ));
    set_rounding_rule(round_gross_premium_     , RoundingRules_->datum("RoundGrossPrem"  ));
    set_rounding_rule(round_net_premium_       , RoundingRules_->datum("RoundNetPrem"    ));
    set_rounding_rule(round_interest_rate_     , RoundingRules_->datum("RoundIntRate"    ));
    set_rounding_rule(round_interest_credit_   , RoundingRules_->datum("RoundIntCredit"  ));
    set_rounding_rule(round_withdrawal_        , RoundingRules_->datum("RoundWithdrawal" ));
    set_rounding_rule(round_loan_              , RoundingRules_->datum("RoundLoan"       ));
    set_rounding_rule(round_corridor_factor_   , RoundingRules_->datum("RoundCorrFactor" ));
    set_rounding_rule(round_surrender_charge_  , RoundingRules_->datum("RoundSurrCharge" ));
    set_rounding_rule(round_irr_               , RoundingRules_->datum("RoundIrr"        ));
    set_rounding_rule(round_min_specamt_       , RoundingRules_->datum("RoundMinSpecamt" ));
    set_rounding_rule(round_max_specamt_       , RoundingRules_->datum("RoundMaxSpecamt" ));
    set_rounding_rule(round_min_premium_       , RoundingRules_->datum("RoundMinPrem"    ));
    set_rounding_rule(round_max_premium_       , RoundingRules_->datum("RoundMaxPrem"    ));
    set_rounding_rule(round_interest_rate_7702_, RoundingRules_->datum("RoundIntRate7702"));
}

/// Set all parameters that depend on premium-tax state.
///
/// These database entities should be looked up by tax state:
///  - DB_PremTaxLoad
///  - DB_PremTaxRate
///  - DB_PremTaxRetalLimit
/// These probably (for inchoate amortization) shouldn't:
///  - DB_PremTaxAmortPeriod
///  - DB_PremTaxAmortIntRate
/// This definitely shouldn't be:
///  - DB_PremTaxState
/// These aren't used anywhere yet:
///  - DB_PremTaxFundCharge
///  - DB_PremTaxTable
///  - DB_PremTaxTierGroup
///  - DB_PremTaxTierPeriod
///  - DB_PremTaxTierNonDecr

void BasicValues::SetPremiumTaxParameters()
{
    PremiumTaxLoadIsTieredInStateOfDomicile_ = StratifiedCharges_->premium_tax_is_tiered(GetStateOfDomicile());
    PremiumTaxLoadIsTieredInPremiumTaxState_ = StratifiedCharges_->premium_tax_is_tiered(GetPremiumTaxState());

    yare_input yi_premtax(*Input_);
    yi_premtax.StateOfJurisdiction = GetPremiumTaxState();
    product_database db_premtax(yi_premtax);

    LowestPremiumTaxLoad_ = lowest_premium_tax_load
        (db_premtax
        ,*StratifiedCharges_
        ,GetPremiumTaxState()
        ,yare_input_.AmortizePremiumLoad
        );

    // TODO ?? It would be better not to constrain so many things
    // not to vary by duration by using Query(enumerator).

    PremiumTaxRate_ = db_premtax.Query(DB_PremTaxRate);
    PremiumTaxLoad_ = db_premtax.Query(DB_PremTaxLoad);

    StateOfDomicile_ = mc_state_from_string(ProductData_->datum("InsCoDomicile"));
    {
    yare_input yi_domicile(*Input_);
    yi_domicile.StateOfJurisdiction = GetStateOfDomicile();
    product_database db_domicile(yi_domicile);
    DomiciliaryPremiumTaxLoad_ = 0.0;
    if(!yare_input_.AmortizePremiumLoad)
        {
        DomiciliaryPremiumTaxLoad_ = db_domicile.Query(DB_PremTaxLoad);
        }
    }

    TestPremiumTaxLoadConsistency();

    FirstYearPremiumRetaliationLimit_ = db_premtax.Query(DB_PremTaxRetalLimit);
}

/// Lowest premium-tax load, for 7702 and 7702A purposes.

double lowest_premium_tax_load
    (product_database   const& db
    ,stratified_charges const& stratified
    ,mcenum_state              premium_tax_state
    ,bool                      amortize_premium_load
    )
{
    // TRICKY !! Here, we use 'DB_PremTaxLoad', not 'DB_PremTaxRate',
    // to determine the lowest premium-tax load. Premium-tax loads
    // (charged by the insurer to the contract) and rates (charged by
    // the state to the insurer) really shouldn't be mixed. The
    // intention is to support products that pass actual premium tax
    // through as a load, taking into account retaliation and tiered
    // premium-tax rates.
    //
    // While a more complicated model would be more aesthetically
    // satisfying, this gives the right answer in practice for the
    // two cases we believe will arise in practice. In the first case,
    // premium-tax load doesn't vary by state--perhaps a flat load
    // such as two percent might be used, or maybe zero percent with
    // premium-tax expense covered elsewhere in pricing--and tiering
    // is ignored, so this implementation just returns the flat load.
    // In the second case, the exact premium tax is passed through,
    // so the tax rate equals the tax load.

    double z = 0.0;
    if(amortize_premium_load)
        {
        return z;
        }

    z = db.Query(DB_PremTaxLoad);

    if(!db.varies_by_state(DB_PremTaxLoad))
        {
        return z;
        }

    // If premium-tax load varies by state, we're assuming that
    // it equals premium-tax rate--i.e. that premium tax is passed
    // through exactly--and that therefore tiered tax rates determine
    // loads where applicable and implemented.
    if(!db.are_equivalent(DB_PremTaxLoad, DB_PremTaxRate))
        {
        fatal_error()
            << "Premium-tax load varies by state, but differs"
            << " from premium-tax rates. Probably the database"
            << " is incorrect.\n"
            << LMI_FLUSH
            ;
        }

    if(stratified.premium_tax_is_tiered(premium_tax_state))
        {
        if(0.0 != z)
            {
            fatal_error()
                << "Premium-tax load is tiered in state "
                << mc_str(premium_tax_state)
                << ", but the product database specifies a scalar load of "
                << z
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        z = stratified.minimum_tiered_premium_tax_rate(premium_tax_state);
        }

    return z;
}

/// Test consistency of premium-tax loads.
///
/// In particular, if the tiered premium-tax load isn't zero, then the
/// corresponding non-tiered load must be zero.
///
/// Premium-tax pass-through for AK, DE, and SD insurers is not
/// supported. If the state of domicile has a tiered rate, then most
/// likely the premium-tax state does not, and retaliation would often
/// override the tiering. When those two states are the same, then no
/// retaliation occurs, and calculations would presumably be correct.
/// When both states have tiered rates, but they are different states,
/// then the calculation could be complicated; but DE tiering is not
/// supported at all yet, and AK (SD) companies probably write few
/// contracts in SD (AK), so these exotic cases haven't commanded any
/// attention. If premium tax is not passed through as a load, then
/// there's no problem at all.

void BasicValues::TestPremiumTaxLoadConsistency() const
{
    if(PremiumTaxLoadIsTieredInPremiumTaxState_)
        {
        if(0.0 != PremiumTaxLoad())
            {
            fatal_error()
                << "Premium-tax load is tiered in premium-tax state "
                << mc_str(GetPremiumTaxState())
                << ", but the product database specifies a scalar load of "
                << PremiumTaxLoad()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        }

    if(PremiumTaxLoadIsTieredInStateOfDomicile_)
        {
        if(0.0 != DomiciliaryPremiumTaxLoad())
            {
            fatal_error()
                << "Premium-tax load is tiered in state of domicile "
                << mc_str(GetStateOfDomicile())
                << ", but the product database specifies a scalar load of "
                << DomiciliaryPremiumTaxLoad()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        fatal_error()
            << "Premium-tax load is tiered in state of domicile "
            << mc_str(GetStateOfDomicile())
            << ", but that case is not supported."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
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
            std::vector<double> z(MortalityRates_->PartialMortalityQ());
            // ET !! z = 1.0 - z;
            std::transform
                (z.begin()
                ,z.end()
                ,z.begin()
                ,std::bind1st(std::minus<double>(), 1.0)
                );
            // ET !! In APL, this would be [writing multiplication as '*']
            //   +/*\1-z
            // It would be nice to have a concise representation for that.
            std::partial_sum(z.begin(), z.end(), z.begin(), std::multiplies<double>());
            MaxSurvivalDur = std::accumulate(z.begin(), z.end(), 0.0);
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << yare_input_.SurviveToType
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
    HOPEFULLY(MaxSurvivalDur <= EndtAge);
}

//============================================================================
// For now at least, calls the same subroutine as GetModalTgtPrem().
double BasicValues::GetModalMinPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    oenum_modal_prem_type const PremType =
        static_cast<oenum_modal_prem_type>(static_cast<int>(Database_->Query(DB_MinPremType)));
    return GetModalPrem(a_year, a_mode, a_specamt, PremType);
}

//============================================================================
double BasicValues::GetModalTgtPrem
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    oenum_modal_prem_type const PremType =
        static_cast<oenum_modal_prem_type>(static_cast<int>(Database_->Query(DB_TgtPremType)));
    double modal_prem = GetModalPrem(a_year, a_mode, a_specamt, PremType);

    // TODO ?? Probably this should reflect policy fee. Some products
    // define only an annual target premium, and don't specify how to
    // modalize it.
//      modal_prem += POLICYFEE / a_mode;

    return modal_prem;
}

//============================================================================
double BasicValues::GetModalPrem
    (int                   a_year
    ,mcenum_mode           a_mode
    ,double                a_specamt
    ,oenum_modal_prem_type a_prem_type
    ) const
{
    if(oe_monthly_deduction == a_prem_type)
        {
        return GetModalPremMlyDed(a_year, a_mode, a_specamt);
        }
    else if(oe_modal_nonmec == a_prem_type)
        {
        return GetModalPremMaxNonMec(a_year, a_mode, a_specamt);
        }
    else if(oe_modal_table == a_prem_type)
        {
        // The fn s/b generalized to allow an input premium file and an input
        // policy fee. If oe_modal_table is ever used for other than tgt prem,
        // it will be wrong. TODO ?? Fix this.
        return GetModalPremTgtFromTable(a_year, a_mode, a_specamt);
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << a_prem_type << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

/// Calculate premium using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount. Thus, arguments should represent initial
/// premium and mode.

double BasicValues::GetModalPremMaxNonMec
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    double temp = MortalityRates_->SevenPayRates()[0];
    return round_max_premium()(temp * ldbl_eps_plus_one() * a_specamt / a_mode);
}

/// Calculate premium using a target-premium ratio.
///
/// Only the initial target-premium rate is used here, because that's
/// generally fixed at issue. However, this calculation remains naive
/// in that the initial specified amount may also be fixed at issue.

double BasicValues::GetModalPremTgtFromTable
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    return round_max_premium()
        (
            (   Database_->Query(DB_TgtPremMonthlyPolFee)
            +       a_specamt
                *   ldbl_eps_plus_one()
                *   MortalityRates_->TargetPremiumRates()[0]
            )
        /   a_mode
        );
}

/// Calculate premium using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial specified amount and mode.

double BasicValues::GetModalPremCorridor
    (int      // a_year // Unused.
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    double temp = GetCorridorFactor()[0];
    return round_max_premium()((ldbl_eps_plus_one() * a_specamt / temp) / a_mode);
}

//============================================================================
double BasicValues::GetModalPremGLP
    (int         a_duration
    ,mcenum_mode a_mode
    ,double      a_bft_amt
    ,double      a_specamt
    ) const
{
    double z = Irc7702_->CalculateGLP
        (a_duration
        ,a_bft_amt
        ,a_specamt
        ,Irc7702_->GetLeastBftAmtEver()
        ,effective_dbopt_7702(DeathBfts_->dbopt()[0], Equiv7702DBO3)
        );

// TODO ?? PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium()(ldbl_eps_plus_one() * z);
}

//============================================================================
double BasicValues::GetModalPremGSP
    (int         a_duration
    ,mcenum_mode a_mode
    ,double      a_bft_amt
    ,double      a_specamt
    ) const
{
    double z = Irc7702_->CalculateGSP
        (a_duration
        ,a_bft_amt
        ,a_specamt
        ,Irc7702_->GetLeastBftAmtEver()
        );

// TODO ?? PROBLEMS HERE
// what if a_year != 0 ?
// term rider, dumpin

    z /= a_mode;
    return round_max_premium()(ldbl_eps_plus_one() * z);
}

/// Determine an approximate "pay as you go" modal premium.
///
/// This more or less represents actual monthly deductions, at least
/// for monthly mode on an option B contract, generally favoring
/// sufficiency over minimality, but simplicity most of all.
///
/// For simplicity, certain details are disregarded:
///   - waiver benefits are generally subject to a maximum
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
/// TODO ?? What should be the behavior if ee and er both pay and their
/// modes differ?

double BasicValues::GetModalPremMlyDed
    (int         a_year
    ,mcenum_mode a_mode
    ,double      a_specamt
    ) const
{
    double z = a_specamt * DBDiscountRate[a_year];
    z *= GetBandedCoiRates(mce_gen_curr, a_specamt)[a_year];

    if(yare_input_.AccidentalDeathBenefit)
        {
        double r = MortalityRates_->AdbRates()[a_year];
        z += r * std::min(a_specamt, AdbLimit);
        }

    if(yare_input_.SpouseRider)
        {
        double r = MortalityRates_->SpouseRiderRates(mce_gen_curr)[a_year];
        z += r * yare_input_.SpouseRiderAmount;
        }

    if(yare_input_.ChildRider)
        {
        double r = MortalityRates_->ChildRiderRates()[a_year];
        z += r * yare_input_.ChildRiderAmount;
        }

    if(true) // Written thus for parallelism and to keep 'r' local.
        {
        double r = Loads_->specified_amount_load(mce_gen_curr)[a_year];
        z += r * std::min(a_specamt, SpecAmtLoadLimit);
        }

    z += Loads_->monthly_policy_fee(mce_gen_curr)[a_year];

    double annual_charge = Loads_->annual_policy_fee(mce_gen_curr)[a_year];

    if(yare_input_.WaiverOfPremiumBenefit)
        {
        double r = MortalityRates_->WpRates()[a_year];
        z *= 1.0 + r;
        annual_charge *= 1.0 + r;
        }

    z /= 1.0 - Loads_->target_total_load(mce_gen_curr)[a_year];

    z *= GetAnnuityValueMlyDed(a_year, a_mode);
    z += annual_charge;

    return round_min_premium()(z);
}

//============================================================================
double BasicValues::GetModalSpecAmtMax
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    oenum_modal_prem_type const prem_type = static_cast<oenum_modal_prem_type>
        (static_cast<int>(Database_->Query(DB_MinPremType))
        );
    return GetModalSpecAmt
            (a_ee_mode
            ,a_ee_pmt
            ,a_er_mode
            ,a_er_pmt
            ,prem_type
            );
}

//============================================================================
double BasicValues::GetModalSpecAmtTgt
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    oenum_modal_prem_type const prem_type = static_cast<oenum_modal_prem_type>
        (static_cast<int>(Database_->Query(DB_TgtPremType))
        );
    return GetModalSpecAmt
            (a_ee_mode
            ,a_ee_pmt
            ,a_er_mode
            ,a_er_pmt
            ,prem_type
            );
}

/// Calculate specified amount as a simple function of premium.
///
/// Only scalar premiums and modes are used here. They're intended to
/// represent initial values. Reason: it's generally inappropriate for
/// a specified-amount strategy to produce a result that varies by
/// duration.

double BasicValues::GetModalSpecAmt
    (mcenum_mode           a_ee_mode
    ,double                a_ee_pmt
    ,mcenum_mode           a_er_mode
    ,double                a_er_pmt
    ,oenum_modal_prem_type a_prem_type
    ) const
{
    if(oe_monthly_deduction == a_prem_type)
        {
        return GetModalSpecAmtMlyDed
            (a_ee_mode
            ,a_ee_pmt
            ,a_er_mode
            ,a_er_pmt
            );
        }
    else if(oe_modal_nonmec == a_prem_type)
        {
        return GetModalSpecAmtMinNonMec
            (a_ee_mode
            ,a_ee_pmt
            ,a_er_mode
            ,a_er_pmt
            );
        }
    else if(oe_modal_table == a_prem_type)
        {
        // TODO ?? This is dubious. If the table specified is a
        // seven-pay table, then this seems not to give the same
        // result as the seven-pay premium type.
        double annualized_pmt = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
        return round_min_specamt()
            (annualized_pmt / GetModalPremTgtFromTable(0, a_ee_mode, 1)
            );
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << a_prem_type << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

/// Calculate specified amount using a seven-pay ratio.
///
/// Only the initial seven-pay premium rate is used here. Material
/// changes dramatically complicate the relationship between premium
/// and specified amount.

double BasicValues::GetModalSpecAmtMinNonMec
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    double annualized_pmt = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
    return round_min_specamt()
        (annualized_pmt / MortalityRates_->SevenPayRates()[0]
        );
}

//============================================================================
double BasicValues::GetModalSpecAmtGLP
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    double annualized_pmt = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
    return Irc7702_->CalculateGLPSpecAmt
        (0
        ,annualized_pmt
        ,effective_dbopt_7702(DeathBfts_->dbopt()[0], Equiv7702DBO3)
        );
}

//============================================================================
double BasicValues::GetModalSpecAmtGSP
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    double annualized_pmt = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
    return Irc7702_->CalculateGSPSpecAmt
        (0
        ,annualized_pmt
        );
}

/// Calculate specified amount using a corridor ratio.
///
/// Only the initial corridor factor is used here, because this
/// strategy makes sense only at issue. Thus, arguments should
/// represent initial premium and mode.

double BasicValues::GetModalSpecAmtCorridor
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    double annualized_pmt = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
    double rate = GetCorridorFactor()[0];
    return round_min_specamt()(annualized_pmt * rate);
}

/// In general, strategies linking specamt and premium commute. The
/// "pay deductions" strategy, however, doesn't have a useful analog
/// for determining specamt as a function of initial premium: the
/// contract would almost certainly lapse after one year. Therefore,
/// calling this function elicits an error message.

double BasicValues::GetModalSpecAmtMlyDed
    (mcenum_mode a_ee_mode
    ,double      a_ee_pmt
    ,mcenum_mode a_er_mode
    ,double      a_er_pmt
    ) const
{
    if(!global_settings::instance().regression_testing())
        {
        fatal_error()
            << "No maximum specified amount is defined for this product."
            << LMI_FLUSH
            ;
        }

    // Soon this ancient implementation will be expunged. Original
    // 'todo' defect markers have been replaced with the word "DEFECT"
    // in capital letters: they no longer count toward the global
    // total because they're already unreachable for end users.

    // For now, we just assume that ee mode governs...only a guess...
    mcenum_mode guess_mode = a_ee_mode;
    double z = a_ee_mode * a_ee_pmt + a_er_mode * a_er_pmt;
    z /= guess_mode;

    double annual_charge = Loads_->annual_policy_fee(mce_gen_curr)[0];

    double wp_rate = 0.0;
    if(yare_input_.WaiverOfPremiumBenefit)
        {
        // For simplicity, ignore Database_->Query(DB_WpMax)
        wp_rate = MortalityRates_->WpRates()[0];
        if(0.0 != 1.0 + wp_rate)
            {
            annual_charge /= (1.0 + wp_rate);
            }
        }

    z -= annual_charge;

    // DEFECT Use first-year values only--don't want this to vary by year.
    z /= GetAnnuityValueMlyDed(0, guess_mode);
    // DEFECT only *target* load?
// DEFECT Looks like none of our test decks exercise this line.
    z *= 1.0 - Loads_->target_total_load(mce_gen_curr)[0];

    // DEFECT Is this correct now?
    if(yare_input_.WaiverOfPremiumBenefit && 0.0 != 1.0 + wp_rate)
        {
        // For simplicity, ignore Database_->Query(DB_WpMax)
        z /= (1.0 + wp_rate);
        }

    if(yare_input_.AccidentalDeathBenefit)
        {
        // DEFECT For simplicity, ignore Database_->Query(DB_AdbLimit)
        z -= MortalityRates_->AdbRates()[0];
        }
    // DEFECT Other riders should be considered here.

    z -= Loads_->monthly_policy_fee(mce_gen_curr)[0];
    // DEFECT Probably we should respect banding. This is a
    // conservative shortcut.
    z /= MortalityRates_->MonthlyCoiRatesBand0(mce_gen_curr)[0];
    z *= 1.0 + InterestRates_->GenAcctNetRate
        (mce_gen_guar
        ,mce_monthly_rate
        )[0]
        ;

    return round_max_specamt()(z);
}

/// 'Unusual' banding is one particular approach we needed to model.
/// Simpler than the banding method generally used in the industry, it
/// determines a single COI rate from the total specified amount and
/// applies that single rate to the entire NAAR. No layers of coverage
/// are distinguished.

std::vector<double> const& BasicValues::GetBandedCoiRates
    (mcenum_gen_basis rate_basis
    ,double           a_specamt
    ) const
{
    if(UseUnusualCOIBanding && mce_gen_guar != rate_basis)
        {
        double band_0_limit = Database_->Query(DB_CurrCoiTable0Limit);
        double band_1_limit = Database_->Query(DB_CurrCoiTable1Limit);
        LMI_ASSERT(0.0 <= band_0_limit);
        LMI_ASSERT(band_0_limit <= band_1_limit);
        if(band_0_limit <= a_specamt && a_specamt < band_1_limit)
            {
            return MortalityRates_->MonthlyCoiRatesBand1(rate_basis);
            }
        else if(band_1_limit <= a_specamt)
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

/// Calculate a special modal factor on the fly.
///
/// This factor depends on the general-account rate, which is always
/// specified, even for separate-account-only products.
///
/// This concept is at the same time overelaborate and inadequate.
/// If the crediting rate changes during a policy year, it results in
/// a "pay-deductions" premium that varies between anniversaries, yet
/// may not prevent the contract from lapsing; both those outcomes are
/// likely to frustrate customers.

double BasicValues::GetAnnuityValueMlyDed
    (int         a_year
    ,mcenum_mode a_mode
    ) const
{
    LMI_ASSERT(0.0 != a_mode);
    double spread = 0.0;
    if(mce_monthly != a_mode)
        {
        spread = MinPremIntSpread_[a_year] * 1.0 / a_mode;
        }
    double z = i_upper_12_over_12_from_i<double>()
        (   yare_input_.GeneralAccountRate[a_year]
        -   spread
        );
    double u = 1.0 + std::max
        (z
        ,InterestRates_->GenAcctNetRate
            (mce_gen_guar
            ,mce_monthly_rate
            )[a_year]
        );
    u = 1.0 / u;
    return (1.0 - std::pow(u, 12.0 / a_mode)) / (1.0 - u);
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
    ,long int           TableNumber
    ) const
{
    e_actuarial_table_method const method =
        static_cast<e_actuarial_table_method>
            (static_cast<int>(Database_->Query(DB_CoiInforceReentry))
            );

    if(DB_CurrCoiTable == TableID && e_reenter_never != method)
        {
        return actuarial_table_rates_elaborated
            (TableFile
            ,TableNumber
            ,GetIssueAge()
            ,GetLength()
            ,method
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
        ,static_cast<long int>(Database_->Query(TableID))
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
    yare_input YI(*Input_);
    YI.Gender  = gender ;
    YI.Smoking = smoking;

    product_database TempDatabase(YI);

    return GetActuarialTable
        (TableFile
        ,TableID
        ,static_cast<long int>(TempDatabase.Query(TableID))
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
    ,EBlend      const& CanBlendSmoking
    ,EBlend      const& CanBlendGender
    ) const
{
    if(!IsTableValid)
        {
        return std::vector<double>(GetLength());
        }

    std::string const file_name = AddDataDir(TableFile);

    // To blend by either smoking or gender, both the input must allow
    // it (Input_ member), and the table must allow it (arg: CanBlend);
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
        default:
            {
            fatal_error()
                << "Case '"
                << CanBlendSmoking
                << "' not found."
                << LMI_FLUSH
                ;
            }
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
        default:
            {
            fatal_error()
                << "Case '"
                << CanBlendGender
                << "' not found."
                << LMI_FLUSH
                ;
            }
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
        for(int j = 0; j < GetLength(); j++)
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
        for(int j = 0; j < GetLength(); j++)
            {
            f_tpx *= (1 - F[j]);
            m_tpx *= (1 - M[j]);
            tpx = (f * f_tpx + m * m_tpx);
            BlendedTable.push_back(1.0 - tpx / tpx_prev);
            tpx_prev = tpx;
            }
*/

///*
        for(int j = 0; j < GetLength(); j++)
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
        for(int j = 0; j < GetLength(); j++)
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
        fatal_error() << "Invalid mortality blending." << LMI_FLUSH;
        }

    return BlendedTable;
}

//============================================================================
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
            // break;
        case mce_gpt:
            {
            return Irc7702_->Corridor();
            }
            // break;
        case mce_noncompliant:
            {
            Non7702CompliantCorridor = std::vector<double>(Length, 1.0);
            return Non7702CompliantCorridor;
            }
            // break;
        default:
            {
            fatal_error()
                << "Case "
                << yare_input_.DefinitionOfLifeInsurance
                << " not found."
                << LMI_FLUSH
                ;
            }
            break;
        }

    static std::vector<double> z;
    return z;
}

// potential inlines

std::vector<double> const& BasicValues::SpreadFor7702() const
{
    return SpreadFor7702_;
}

std::vector<double> const& BasicValues::GetMly7702iGlp() const
{
    return Mly7702iGlp;
}

std::vector<double> const& BasicValues::GetMly7702qc() const
{
    return Mly7702qc;
}

std::vector<double> const& BasicValues::GetMlyDcvqc() const
{
    return MlyDcvqc;
}

// Only current (hence midpoint) COI and term rates are blended

std::vector<double> BasicValues::GetCvatCorridorFactors() const
{
    return GetTable
        (ProductData_->datum("CorridorFilename")
        ,DB_CorridorTable
        );
}
std::vector<double> BasicValues::GetCurrCOIRates0() const
{
    return GetTable
        (ProductData_->datum("CurrCOIFilename")
        ,DB_CurrCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}
std::vector<double> BasicValues::GetCurrCOIRates1() const
{
    if
        ( Database_->Query(DB_CurrCoiTable0Limit)
        < std::numeric_limits<double>::max()
        )
        {
        return GetTable
            (ProductData_->datum("CurrCOIFilename")
            ,DB_CurrCoiTable1
            ,true
            ,CanBlend
            ,CanBlend
            );
        }
    else
        {
        return std::vector<double>(Length);
        }
}
std::vector<double> BasicValues::GetCurrCOIRates2() const
{
    if
        ( Database_->Query(DB_CurrCoiTable1Limit)
        < std::numeric_limits<double>::max()
        )
        {
        return GetTable
            (ProductData_->datum("CurrCOIFilename")
            ,DB_CurrCoiTable2
            ,true
            ,CanBlend
            ,CanBlend
            );
        }
    else
        {
        return std::vector<double>(Length);
        }
}
std::vector<double> BasicValues::GetGuarCOIRates() const
{
    return GetTable
        (ProductData_->datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        );
}
std::vector<double> BasicValues::GetSmokerBlendedGuarCOIRates() const
{
    return GetTable
        (ProductData_->datum("GuarCOIFilename")
        ,DB_GuarCoiTable
        ,true
        ,CanBlend
        ,CanBlend
        );
}
std::vector<double> BasicValues::GetWpRates() const
{
    return GetTable
        (ProductData_->datum("WPFilename")
        ,DB_WpTable
        ,Database_->Query(DB_AllowWp)
        );
}
std::vector<double> BasicValues::GetAdbRates() const
{
    return GetTable
        (ProductData_->datum("ADDFilename")
        ,DB_AdbTable
        ,Database_->Query(DB_AllowAdb)
        );
}
std::vector<double> BasicValues::GetChildRiderRates() const
{
    return GetTable
        (ProductData_->datum("ChildRiderFilename")
        ,DB_ChildRiderTable
        ,Database_->Query(DB_AllowChildRider)
        );
}
std::vector<double> BasicValues::GetCurrentSpouseRiderRates() const
{
    if(!Database_->Query(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(ProductData_->datum("CurrSpouseRiderFilename"))
        ,static_cast<long int>(Database_->Query(DB_SpouseRiderTable))
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}
std::vector<double> BasicValues::GetGuaranteedSpouseRiderRates() const
{
    if(!Database_->Query(DB_AllowSpouseRider))
        {
        return std::vector<double>(GetLength());
        }

    std::vector<double> z = actuarial_table_rates
        (AddDataDir(ProductData_->datum("GuarSpouseRiderFilename"))
        ,static_cast<long int>(Database_->Query(DB_SpouseRiderGuarTable))
        ,yare_input_.SpouseIssueAge
        ,EndtAge - yare_input_.SpouseIssueAge
        );
    z.resize(Length);
    return z;
}
std::vector<double> BasicValues::GetCurrentTermRates() const
{
    return GetTable
        (ProductData_->datum("CurrTermFilename")
        ,DB_TermTable
        ,Database_->Query(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}
std::vector<double> BasicValues::GetGuaranteedTermRates() const
{
    return GetTable
        (ProductData_->datum("GuarTermFilename")
        ,DB_GuarTermTable
        ,Database_->Query(DB_AllowTerm)
        ,CanBlend
        ,CanBlend
        );
}
std::vector<double> BasicValues::GetTableYRates() const
{
    return GetTable
        (ProductData_->datum("TableYFilename")
        ,DB_TableYTable
        );
}
std::vector<double> BasicValues::GetTAMRA7PayRates() const
{
    return GetTable
        (ProductData_->datum("TAMRA7PayFilename")
        ,DB_SevenPayTable
        );
}
std::vector<double> BasicValues::GetTgtPremRates() const
{
    return GetTable
        (ProductData_->datum("TgtPremFilename")
        ,DB_TgtPremTable
        ,oe_modal_table == Database_->Query(DB_TgtPremType)
        );
}
std::vector<double> BasicValues::GetIRC7702Rates() const
{
    return GetTable
        (ProductData_->datum("IRC7702Filename")
        ,DB_Irc7702QTable
        );
}
std::vector<double> BasicValues::Get83GamRates() const
{
    return GetTable
        (ProductData_->datum("Gam83Filename")
        ,DB_Gam83Table
        ,true
        ,CannotBlend
        ,CanBlend
        );
}
std::vector<double> BasicValues::GetSubstdTblMultTable() const
{
    if(0 == Database_->Query(DB_SubstdTableMultTable))
        {
        return std::vector<double>(GetLength(), 1.0);
        }

    return GetTable
        (ProductData_->datum("SubstdTblMultFilename")
        ,DB_SubstdTableMultTable
        );
}
std::vector<double> BasicValues::GetCurrSpecAmtLoadTable() const
{
    if(0 == Database_->Query(DB_CurrSpecAmtLoadTable))
        {
        return std::vector<double>(GetLength());
        }

    return GetTable
        (ProductData_->datum("CurrSpecAmtLoadFilename")
        ,DB_CurrSpecAmtLoadTable
        );
}
std::vector<double> BasicValues::GetGuarSpecAmtLoadTable() const
{
    if(0 == Database_->Query(DB_GuarSpecAmtLoadTable))
        {
        return std::vector<double>(GetLength());
        }

    return GetTable
        (ProductData_->datum("GuarSpecAmtLoadFilename")
        ,DB_GuarSpecAmtLoadTable
        );
}

