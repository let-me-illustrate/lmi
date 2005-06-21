// Basic values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
// Portions marked JLM Copyright (C) 2001 (modal target), 2002 (non-7702 corridor) Gregory W. Chicares and Joseph L. Murdzek.
// Author is GWC except where specifically noted otherwise.
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

// $Id: ihs_basicval.cpp,v 1.13 2005-06-21 05:27:48 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "basic_values.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "global_settings.hpp"
#include "ihs_dbdict.hpp"
#include "ihs_funddata.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_proddata.hpp"
#include "ihs_rnddata.hpp"
#include "ihs_x_type.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "mortality_rates.hpp"
#include "outlay.hpp"
#include "surrchg_rates.hpp"
#include "tiered_charges.hpp"

#include <algorithm>
#include <cmath>        // std::pow()
#include <cstring>      // std::strlen()
#include <functional>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace
{
    // TODO ?? This is a kludge.
    //
    // Floating-point numbers that represent integers scaled by
    // negative powers of ten are inexact. For example, a premium
    // rate of $2.40 per $1000 is be stored as 0.0024, but to the
    // hardware it may look like
    //   0.0023999999999999998 [0x3ff69d495182a9930800]
    // Multiplying that number by a million dollars and rounding
    // down to cents yields 2399.99, where 2400.00 is wanted.
    //
    // The best way to handle this is to store integers. Until we
    // have time to consider that, multiplying by 1 + LDBL_EPSILON
    // avoids this embarrassment while introducing an error that
    // shouldn't matter.
    long double const epsilon_plus_one =
        1.0L + std::numeric_limits<long double>::epsilon()
        ;
} // Unnamed namespace.

//============================================================================
BasicValues::BasicValues()
    :Input_(new InputParms)
{
    Init();
}

//============================================================================
BasicValues::BasicValues(InputParms const* input)
    :Input_(new InputParms(*input))
{
    Init();
}

//============================================================================
// TODO ?? Not for general use--use for GPT server only. This is bad design.
BasicValues::BasicValues
    (std::string  const& a_ProductName
    ,e_gender     const& a_Gender
    ,e_class      const& a_UnderwritingClass
    ,e_smoking    const& a_Smoker
    ,int                 a_IssueAge
    ,e_uw_basis   const& a_UnderwritingBasis
    ,e_state      const& a_StateOfJurisdiction
    ,double              a_FaceAmount
    ,e_dbopt_7702 const& a_DBOptFor7702
    ,bool                a_ADDInForce
    ,double              a_TargetPremium
    // TODO ?? Need loan rate type here?
    )
    :InitialTargetPremium(a_TargetPremium)
{
    InputParms* kludge_input = new InputParms;

    kludge_input->NumberOfLives       = 1                        ;

    kludge_input->Status[0].IssueAge  = a_IssueAge               ;
    kludge_input->Status[0].RetAge    = a_IssueAge               ;
    kludge_input->Status[0].Gender    = a_Gender                 ;
    kludge_input->Status[0].Smoking   = a_Smoker                 ;
    kludge_input->Status[0].Class     = a_UnderwritingClass      ;
//    kludge_input->Status[0].HasADD    = static_cast<enum_yes_or_no>(a_ADDInForce.operator const bool());
// TODO ?? reinterpret_cast can't be right...
//    kludge_input->Status[0].HasADD    = reinterpret_cast<enum_yes_or_no>(a_ADDInForce);
//    kludge_input->Status[0].HasADD    = a_ADDInForce;
if(a_ADDInForce)
    {
    kludge_input->Status[0].HasADD = "Yes";
    }
else
    {
    kludge_input->Status[0].HasADD = "No";
    }
    kludge_input->GroupUWType         = a_UnderwritingBasis      ;
    kludge_input->ProductName         = a_ProductName            ;
    kludge_input->InsdState           = a_StateOfJurisdiction    ;
    kludge_input->SponsorState        = a_StateOfJurisdiction    ;
    kludge_input->DefnLifeIns         = e_defn_life_ins(e_gpt)   ;
    kludge_input->DefnMaterialChange  = e_defn_material_change(e_adjustment_event);

    kludge_input->EnforceConsistency();

    std::fill_n
        (kludge_input->SpecAmt.begin()
        ,kludge_input->YearsToMaturity()
        ,a_FaceAmount
        );

    std::fill_n
        (kludge_input->DBOpt.begin()
        ,kludge_input->YearsToMaturity()
        ,Get7702EffectiveDBOpt(a_DBOptFor7702)
        );

    // TODO ?? EGREGIOUS_DEFECT Redesign this function instead.
    const_cast<InputParms&>(*Input_) = *kludge_input;

    GPTServerInit();
}

//============================================================================
BasicValues::~BasicValues()
{
}

//============================================================================
void BasicValues::Init()
{
    ProductData_.reset(new TProductData(Input_->ProductName));
    // bind to policy form
    //      one filename that brings in all the rest incl database?
    // controls as ctor arg?
    // validate input in context of this policy form

    InputStatus const& S = Input_->Status[0]; // TODO ?? Database based on first life only.

    // TRICKY !! We need the database to look up whether ALB or ANB should
    // be used, in case we need to determine issue age from DOB. But issue
    // age is a database lookup key, so it can change what we looked up in
    // the database. To resolve this circularity, we first assume ALB, then
    // create the database, then recalculate the age. If any circularity
    // remains, it will be detected and an error message given when we look
    // up the ALB/ANB switch using TDatabase::Query(int), which restricts
    // looked-up values to scalars that vary across no database axis.

    // TODO ?? Does this even belong here?
    S.MakeAgesAndDatesConsistent(Input_->EffDate, 0);
    Database_.reset(new TDatabase(*Input_));
    bool use_anb = Database_->Query(DB_AgeLastOrNearest);
    S.MakeAgesAndDatesConsistent(Input_->EffDate, use_anb);
    // Now that we have the right issue age, we need to reinitialize
    // the database with that age.
    Database_.reset(new TDatabase(*Input_));
    StateOfJurisdiction = Database_->GetStateOfJurisdiction();

    if
        (   !Database_->Query(DB_StateApproved)
        &&  !global_settings::instance().ash_nazg()
        &&  !global_settings::instance().regression_testing()
        )
        {
        throw std::runtime_error
            (    Input_->ProductName
            +    " not approved in state "
            +    GetStateOfJurisdiction().str()
            );
        }

    IssueAge = S.IssueAge;
    RetAge = S.RetAge;
    LMI_ASSERT(IssueAge < 100);
    LMI_ASSERT(RetAge <= 100);
    LMI_ASSERT(Input_->RetireesCanEnroll || IssueAge <= RetAge);

    int endt_age = static_cast<int>(Database_->Query(DB_EndtAge));
    Length = endt_age - IssueAge;

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
    FundData_.reset(new FundData(AddDataDir(ProductData_->GetFundFilename())));
    RoundingRules_.reset
        (new rounding_rules
            (StreamableRoundingRules
                (AddDataDir(ProductData_->GetRoundingFilename())
                ).get_rounding_rules()
            )
        );
    TieredCharges_.reset
        (new tiered_charges(AddDataDir(ProductData_->GetTierFilename()))
        );
    SpreadFor7702_.assign(Length, TieredCharges_->minimum_tiered_spread_for_7702());

    // Multilife contracts will need a vector of mortality-rate objects.

    // Mortality and interest rates require database.
    // Interest rates require tiered data and 7702 spread.
    MortalityRates_.reset(new MortalityRates (*this));
    InterestRates_ .reset(new InterestRates  (*this));
    // Surrender-charge rates will eventually require mortality rates.
    SurrChgRates_  .reset(new SurrChgRates   (*Database_));
    DeathBfts_     .reset(new death_benefits (*this));
    // Outlay requires interest rates.
    Outlay_        .reset(new Outlay         (*this));
    SetLowestPremTaxRate();
    Loads_         .reset(new Loads          (*this));

    // The target premium can't be ascertained yet if specamt is
    // determined by a strategy.
    InitialTargetPremium = 0.0;

    PremiumTaxLoadIsTieredInStateOfDomicile     = false;
    PremiumTaxLoadIsTieredInStateOfJurisdiction = false;

    SetPermanentInvariants();

    Init7702();
    Init7702A();
}

//============================================================================
// TODO ??  Not for general use--use for GPT server only, for now--refactor later
void BasicValues::GPTServerInit()
{
    ProductData_.reset(new TProductData(Input_->ProductName));

    InputStatus const& S = Input_->Status[0]; // TODO ?? Database based on first life only.
    bool use_anb = Database_->Query(DB_AgeLastOrNearest);
    // TODO ?? Does this even belong here?
    Input_->Status[0].MakeAgesAndDatesConsistent(Input_->EffDate, use_anb);
    IssueAge = S.IssueAge;
    RetAge = S.RetAge;
    LMI_ASSERT(IssueAge < 100);
    LMI_ASSERT(RetAge <= 100);
    LMI_ASSERT(Input_->RetireesCanEnroll || IssueAge <= RetAge);

    Database_.reset(new TDatabase(*Input_));
    StateOfJurisdiction = Database_->GetStateOfJurisdiction();

    int endt_age = static_cast<int>(Database_->Query(DB_EndtAge));
    Length = endt_age - IssueAge;

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
//      (AddDataDir(ProductData_->GetFundFilename())
//      );
    RoundingRules_.reset
        (new rounding_rules
            (StreamableRoundingRules
                (AddDataDir(ProductData_->GetRoundingFilename())
                ).get_rounding_rules()
            )
        );
    TieredCharges_.reset
        (new tiered_charges(AddDataDir(ProductData_->GetTierFilename()))
        );

    // Requires database.
//  MortalityRates_.reset(new MortalityRates (*this));
//  InterestRates_ .reset(new InterestRates  (*this));
    // Will require mortality rates eventually.
//  SurrChgRates_  .reset(new SurrChgRates   (Database_));
//  DeathBfts_     .reset(new death_benefits (*this));
    // Requires interest rates.
//  Outlay_        .reset(new Outlay         (*this));
    SetLowestPremTaxRate();
    Loads_         .reset(new Loads          (*this));

    SetPermanentInvariants();

    Init7702();
}

//============================================================================
// TODO ?? Does this belong in the funds class?
double BasicValues::InvestmentManagementFee() const
{
    if(!Database_->Query(DB_AllowSepAcct))
        {
        return 0.0;
        }

    if(Input_->OverrideFundMgmtFee)
        {
        return Input_->InputFundMgmtFee / 10000.0L;
        }

    double z = 0.0;
    double TotalSepAcctAllocations = 0.0;
    FundData const& Funds = *FundData_;

    for(int j = 0; j < Funds.GetNumberOfFunds(); j++)
        {
        double weight;
        // If average of all funds, then use equal weights, but
        // disregard "custom" funds--that is, set their weights to
        // zero. Custom funds are those whose name begins with "Custom".
        // Reason: "average" means average of the normally-available
        // funds only.
        if(Input_->AvgFund)
            {
            char const s[] = "Custom";
            std::size_t n = std::strlen(s);
            bool ignore = 0 == strncmp
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
            // We allow Input_->NumberOfFunds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the *.fnd file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of Input_->FundAllocs, use a weight of zero.
            bool legal_storage = j < Input_->FundAllocs.size();
            double fund_alloc = Input_->FundAllocs[j];
            weight = legal_storage ? fund_alloc : 0.0;
            }
*/
        else if(j < static_cast<int>(Input_->FundAllocs.size()))
            {
            // TODO ?? Can this be correct? Shouldn't we be looking to
            // the .fnd file rather than the input class?
            //
            // We allow Input_->NumberOfFunds < Funds.GetNumberOfFunds
            // so an accurate fund average can be calculated (based
            // on the funds in the *.fnd file), even though the inputs
            // class may not accommodate that many funds. If j falls
            // outside the range of Input_->FundAllocs, use a weight of zero.
            weight = Input_->FundAllocs[j];
            }
        else
            {
            weight = 0.0;
            }

        if(0.0 != weight)
            {
            z += weight * Funds.GetFundInfo(j).ScalarIMF();
            TotalSepAcctAllocations += weight;
            }
        }

    // Spread over separate account funds only
    if(0.0 != TotalSepAcctAllocations)
        {
        // Convert from basis points
        z /= 10000.0 * TotalSepAcctAllocations;
        }

    return z;
}

//============================================================================
void BasicValues::Init7702()
{
//  Mly7702qc = MortalityRates_->GetaCoi7702(); // TODO ?? Should this function be eliminated?
    Mly7702qc = GetIRC7702Rates(); // TODO ?? This could use a better name.
    std::transform
        (Mly7702qc.begin()
        ,Mly7702qc.end()
        ,Mly7702qc.begin()
        ,std::bind2nd
            (coi_rate_from_q<double>()
            ,Database_->Query(DB_MaxMonthlyCoiRate)
            )
        );

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
    switch(Input_.LoanRateType)
        {
        case e_fixed_loan_rate:
        // APL: guar_int gets guar_int max gross_loan_rate - guar_loan_spread
            {
            std::vector<double> gross_loan_rate;
            Database_->Query(gross_loan_rate, DB_FixedLoanRate);
            std::vector<double> guar_loan_spread;
            Database_->Query(guar_loan_spread, DB_GuarRegLoanSpread);
            std::vector<double> guar_loan_rate(Length);
            std::transform
                (gross_loan_rate.begin()
                ,gross_loan_rate.end()
                ,guar_loan_spread.begin()
                ,guar_loan_rate.begin()
                ,std::minus<double>()
                );
            std::transform
                (guar_int.begin()
                ,guar_int.end()
                ,guar_loan_spread.begin()
                ,guar_int.begin()
                ,greater_of<double>()
                );
            }
            break;
        case e_variable_loan_rate:
            {
            // do nothing
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_.LoanRateType
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
*/

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

    Mly7702ig = DBDiscountRate;
    std::transform(Mly7702ig.begin(), Mly7702ig.end(), Mly7702ig.begin(),
          std::bind1st(std::divides<double>(), 1.0)
          );
    std::transform(Mly7702ig.begin(), Mly7702ig.end(), Mly7702ig.begin(),
          std::bind2nd(std::minus<double>(), 1.0)
          );

    // TODO ?? We should avoid reading the rate file again; but
    // the GPT server doesn't initialize a MortalityRates object
    // that would hold those rates.
    std::vector<double> local_mly_charge_add(Length, 0.0);
    if(Input_->Status[0].HasADD)
        {
        local_mly_charge_add = GetADDRates();
        }

    Irc7702_.reset
        (new Irc7702
            (*this
            ,Input_->DefnLifeIns
            ,Input_->Status[0].IssueAge
            ,EndtAge
            ,Mly7702qc  // MortalityRates_->GetaCoi7702() // TODO ?? This is monthly?
            ,Mly7702iGlp
            ,Mly7702iGsp
            ,Mly7702ig
            ,SpreadFor7702_
            ,Input_->SpecAmt[0] + Input_->Status[0].TermAmt
            ,Input_->SpecAmt[0] + Input_->Status[0].TermAmt
            ,Get7702EffectiveDBOpt(Input_->DBOpt[0])
            // TODO ?? Using the guaranteed basis for all the following should
            // be an optional behavior.
            ,Loads_->annual_policy_fee(e_basis(e_currbasis))
            ,Loads_->monthly_policy_fee(e_basis(e_currbasis))
            ,Loads_->specified_amount_load(e_basis(e_currbasis))
            ,SpecAmtLoadLimit
            ,local_mly_charge_add
            ,ADDLimit
            ,Loads_->target_premium_load_7702_excluding_premium_tax()
            ,Loads_->excess_premium_load_7702_excluding_premium_tax()
            ,InitialTargetPremium
            ,round_min_premium
            ,round_max_premium
            ,round_min_specamt
            ,round_max_specamt
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
            ,DefnLifeIns.value()
            ,DefnMaterialChange.value()
            ,false // TODO ?? Joint life: hardcoded for now.
            ,Input_->AvoidMec.value()
            ,true  // TODO ?? Use table for 7pp: hardcoded for now.
            ,true  // TODO ?? Use table for NSP: hardcoded for now.
            ,MortalityRates_->SevenPayRates()
            ,MortalityRates_->CvatNspRates()
            ,round_max_premium
            )
        );
}

//============================================================================
// Needed for guideline premium.
// TODO ?? dbopt is ignored for now, but some product designs will need it.
double BasicValues::GetTgtPrem
    (int            Year
    ,double         SpecAmt
    ,e_dbopt const& // Unused for now.
    ,e_mode  const& Mode
    ) const
{
    if(Database_->Query(DB_TgtPmFixedAtIssue))
        {
        if(0 == Year)
            {
            InitialTargetPremium = GetModalTgtPrem
                (Year
                ,Mode
                ,SpecAmt
                );
            }
            return InitialTargetPremium;
        }
    else
        {
        return GetModalTgtPrem
            (Year
            ,Mode
            ,SpecAmt
            );
        }
}

//============================================================================
void BasicValues::SetPermanentInvariants()
{
    // TODO ?? It would be better not to constrain so many things
    // not to vary by duration by using Query(enumerator).
    StateOfDomicile     = e_state(ProductData_->GetInsCoDomicile());
    EndtAge             = static_cast<int>(Database_->Query(DB_EndtAge));

    // TODO ?? Perhaps we want the premium-tax load instead of the
    // premium-tax rate here; or maybe we want neither as a member
    // variable, since the premium-tax load is in the loads class.
    PremTaxRate         = Database_->Query(DB_PremTaxRate);

    {
    InputParms IP(*Input_);
    IP.InsdState    = GetStateOfDomicile();
    IP.SponsorState = GetStateOfDomicile();
    TDatabase TempDatabase(IP);
    DomiciliaryPremTaxRate = 0.0;
    if(!Input_->AmortizePremLoad)
        {
        DomiciliaryPremTaxRate = TempDatabase.Query(DB_PremTaxLoad);
        }
    }
    TestPremiumTaxLoadConsistency();

    MinRenlBaseFace     = Database_->Query(DB_MinRenlBaseFace      );
    MinRenlFace         = Database_->Query(DB_MinRenlFace          );
    NoLapseOpt1Only     = Database_->Query(DB_NoLapseOpt1Only      );
    NoLapseUnratedOnly  = Database_->Query(DB_NoLapseUnratedOnly   );
    OptChgCanIncrSA     = Database_->Query(DB_OptChgCanIncrSA      );
    OptChgCanDecrSA     = Database_->Query(DB_OptChgCanDecrSA      );
    WDCanDecrSADBO1     = Database_->Query(DB_WDCanDecrSADBO1      );
    WDCanDecrSADBO2     = Database_->Query(DB_WDCanDecrSADBO2      );
    WDCanDecrSADBO3     = Database_->Query(DB_WDCanDecrSADBO3      );
    MaxIncrAge          = static_cast<int>(Database_->Query(DB_MaxIncrAge));
    WaivePmTxInt1035    = Database_->Query(DB_WaivePmTxInt1035     );
    AllowTerm           = Database_->Query(DB_AllowTerm            );
    ExpPerKLimit        = Database_->Query(DB_ExpPerKLimit         );
    MaxWDDed            = static_cast<enum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxWDDed)));
    MaxWDAVMult         = Database_->Query(DB_MaxWDAVMult          );
    MaxLoanDed          = static_cast<enum_anticipated_deduction>(static_cast<int>(Database_->Query(DB_MaxLoanDed)));
    MaxLoanAVMult       = Database_->Query(DB_MaxLoanAVMult        );
    NoLapseMinDur       = static_cast<int>(Database_->Query(DB_NoLapseMinDur));
    NoLapseMinAge       = static_cast<int>(Database_->Query(DB_NoLapseMinAge));
    MinSpecAmt          = Database_->Query(DB_MinSpecAmt           );
    ADDLimit            = Database_->Query(DB_ADDLimit             );
    WPLimit             = Database_->Query(DB_WPMax                );
    SpecAmtLoadLimit    = Database_->Query(DB_SpecAmtLoadLimit     );
    MinWD               = Database_->Query(DB_MinWD                );
    WDFee               = Database_->Query(DB_WDFee                );
    WDFeeRate           = Database_->Query(DB_WDFeeRate            );
    AllowChangeToDBO2   = Database_->Query(DB_AllowChangeToDBO2    );
    AllowSAIncr         = Database_->Query(DB_AllowSAIncr          );
    NoLapseAlwaysActive = Database_->Query(DB_NoLapseAlwaysActive  );
    WaiverChargeMethod  = static_cast<e_waiver_charge_method>(static_cast<int>(Database_->Query(DB_WPChargeMethod)));
    LapseIgnoresSurrChg = Database_->Query(DB_LapseIgnoresSurrChg  );
    SurrChgOnIncr       = Database_->Query(DB_SurrChgOnIncr        );
    SurrChgOnDecr       = Database_->Query(DB_SurrChgOnDecr        );
    LMI_ASSERT(!SurrChgOnDecr); // Surrchg change on decrease not supported.

    Database_->Query(FreeWDProportion, DB_FreeWDProportion);

    Database_->Query(DBDiscountRate, DB_NAARDiscount);

    Database_->Query(AssetComp , DB_AssetComp);
    Database_->Query(CompTarget, DB_CompTarget);
    Database_->Query(CompExcess, DB_CompExcess);

    LedgerType = Input_->LedgerType();

    FirstYearPremiumRetaliationLimit = Database_->Query(DB_PremTaxRetalLimit);

    COIIsDynamic        = Database_->Query(DB_DynamicCOI             );
    MandEIsDynamic      = Database_->Query(DB_DynamicMandE           );
    SepAcctLoadIsDynamic= Database_->Query(DB_DynamicSepAcctLoad     );

    UseUnusualCOIBanding= Database_->Query(DB_UnusualCOIBanding      );

    // 'Unusual' COI banding accommodates a particular idiosyncratic
    // product which has no term rider and doesn't permit experience
    // rating, so we assert those preconditions and write simple code
    // for 'unusual' COI banding that ignores those features.
    LMI_ASSERT(!(UseUnusualCOIBanding && COIIsDynamic));
    LMI_ASSERT(!(UseUnusualCOIBanding && AllowTerm));

    // Table ratings can arise only from medical underwriting.
    // However, flat extras can be used even with guaranteed issue,
    // e.g. for aviation, occupation, avocation, or foreign travel.
    if
        (   e_table_none != Input_->Status[0].SubstdTable
        &&  e_medical    != Input_->GroupUWType
        )
        {
        fatal_error()
            << "Substandard table ratings require medical underwriting."
            << LMI_FLUSH
            ;
        }

    // Spouse and child riders are not similarly tested because
    // their rates shouldn't depend on the main insured's health.
    if(Input_->Status[0].IsPolicyRated() && Input_->Status[0].HasWP)
        {
        fatal_error()
            << "Substandard waiver of premium not supported."
            << LMI_FLUSH
            ;
        }
    if(Input_->Status[0].IsPolicyRated() && Input_->Status[0].HasADD)
        {
        fatal_error()
            << "Substandard accidental death rider not supported."
            << LMI_FLUSH
            ;
        }

    DefnLifeIns         = Input_->DefnLifeIns;
    DefnMaterialChange  = Input_->DefnMaterialChange;
    Equiv7702DBO3       = static_cast<enum_dbopt_7702>(static_cast<int>(Database_->Query(DB_Equiv7702DBO3)));
    MaxNAAR             = Input_->MaxNAAR;

    Database_->Query(MinPremIntSpread_, DB_MinPremIntSpread);

    round_specamt            = RoundingRules_->round_specamt           ();
    round_death_benefit      = RoundingRules_->round_death_benefit     ();
    round_naar               = RoundingRules_->round_naar              ();
    round_coi_rate           = RoundingRules_->round_coi_rate          ();
    round_coi_charge         = RoundingRules_->round_coi_charge        ();
    round_gross_premium      = RoundingRules_->round_gross_premium     ();
    round_net_premium        = RoundingRules_->round_net_premium       ();
    round_interest_rate      = RoundingRules_->round_interest_rate     ();
    round_interest_credit    = RoundingRules_->round_interest_credit   ();
    round_withdrawal         = RoundingRules_->round_withdrawal        ();
    round_loan               = RoundingRules_->round_loan              ();
    round_corridor_factor    = RoundingRules_->round_corridor_factor   ();
    round_surrender_charge   = RoundingRules_->round_surrender_charge  ();
    round_irr                = RoundingRules_->round_irr               ();
    round_min_specamt        = RoundingRules_->round_min_specamt       ();
    round_max_specamt        = RoundingRules_->round_max_specamt       ();
    round_min_premium        = RoundingRules_->round_min_premium       ();
    round_max_premium        = RoundingRules_->round_max_premium       ();
    round_interest_rate_7702 = RoundingRules_->round_interest_rate_7702();

    SetMaxSurvivalDur();
}

//============================================================================
void BasicValues::SetLowestPremTaxRate()
{
    // TRICKY !! Here, we use 'DB_PremTaxLoad', not 'DB_PremTaxRate',
    // to set the value of 'LowestPremTaxRate'. Premium-tax loads
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

    LowestPremTaxRate = 0.0;
    if(Input_->AmortizePremLoad)
        {
        return;
        }

    LowestPremTaxRate = Database_->Query(DB_PremTaxLoad);

    TDBValue const& premium_tax_loads = Database_->GetEntry(DB_PremTaxLoad);
    if(!TDBValue::VariesByState(premium_tax_loads))
        {
        return;
        }

    // If premium-tax load varies by state, we're assuming that
    // it equals premium-tax rate--i.e. that premium tax is passed
    // through exactly--and that therefore tiered tax rates determine
    // loads where applicable and implemented.
    TDBValue const& premium_tax_rates = Database_->GetEntry(DB_PremTaxRate);
    if(!TDBValue::Equivalent(premium_tax_loads, premium_tax_rates))
        {
        hobsons_choice()
            << "Premium-tax load varies by state, but differs"
            << " from premium-tax rates. Probably the database"
            << " is incorrect.\n"
            << "premium_tax_loads:\n" << premium_tax_loads
            << "premium_tax_rates:\n" << premium_tax_rates
            << LMI_FLUSH
            ;
        }

    if(TieredCharges_->premium_tax_is_tiered(StateOfJurisdiction))
        {
        // TODO ?? TestPremiumTaxLoadConsistency() repeats this test.
        // Probably all the consistency testing should be moved to
        // the database class.
        if(0.0 != LowestPremTaxRate)
            {
            hobsons_choice()
                << "Premium-tax rate is tiered in state "
                << StateOfJurisdiction
                << ", but the product database specifies a scalar rate of "
                << LowestPremTaxRate
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        LowestPremTaxRate = TieredCharges_->minimum_tiered_premium_tax_rate
            (StateOfJurisdiction
            );
        }
}

//============================================================================
void BasicValues::TestPremiumTaxLoadConsistency()
{
    // If premium-tax load doesn't vary by duration or state, then
    // assume that the intention is to override tiering even in
    // states that have tiered premium-tax rates. For instance, if
    // a flat two percent is coded for every state, then it is
    // probably desired to ignore all state variations and nuances.
    TDBValue const& premium_tax_loads = Database_->GetEntry(DB_PremTaxLoad);
    if(!TDBValue::VariesByState(premium_tax_loads))
        {
        return;
        }

    if(TieredCharges_->premium_tax_is_tiered(GetStateOfJurisdiction()))
        {
        PremiumTaxLoadIsTieredInStateOfJurisdiction = true;
        if(0.0 != Database_->Query(DB_PremTaxLoad))
            {
            hobsons_choice()
                << "Premium-tax rate is tiered in state of jurisdiction "
                << GetStateOfJurisdiction()
                << ", but the product database specifies a scalar rate of "
                << Database_->Query(DB_PremTaxLoad)
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        }

    if(TieredCharges_->premium_tax_is_tiered(GetStateOfDomicile()))
        {
        PremiumTaxLoadIsTieredInStateOfDomicile = true;
        if(0.0 != GetDomiciliaryPremTaxRate())
            {
            hobsons_choice()
                << "Premium-tax rate is tiered in state of domicile "
                << GetStateOfDomicile()
                << ", but the product database specifies a scalar rate of "
                << GetDomiciliaryPremTaxRate()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        // TODO ?? Code not tested if state of domicile has tiered rate.
        hobsons_choice()
            << "Premium-tax rate is tiered in state of domicile "
            << GetStateOfDomicile()
            << ", but this program has not been tested for that case."
            << " Please test it carefully before using it."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void BasicValues::SetMaxSurvivalDur()
{
    switch(Input_->SurviveToType)
        {
        case e_no_survival_limit:
            {
            MaxSurvivalDur  = EndtAge;
            }
            break;
        case e_survive_to_age:
            {
            MaxSurvivalDur  = Input_->SurviveToAge - Input_->Status[0].IssueAge;
            }
            break;
        case e_survive_to_year:
            {
            MaxSurvivalDur  = Input_->SurviveToYear;
            }
            break;
        case e_survive_to_ex:
            {
            std::vector<double> z(MortalityRates_->PartialMortalityQ());
            std::transform
                (z.begin()
                ,z.end()
                ,z.begin()
                ,std::bind1st(std::minus<double>(), 1.0)
                );
            std::partial_sum(z.begin(), z.end(), z.begin(), std::multiplies<double>());
            MaxSurvivalDur = std::accumulate(z.begin(), z.end(), 0.0);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->SurviveToType
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    LMI_ASSERT(MaxSurvivalDur <= EndtAge);
}

//============================================================================
// Simply calls the target prem routine for now
double BasicValues::GetModalMinPrem
    (int           Year
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    e_modal_prem_type const PremType =
        static_cast<e_modal_prem_type>(static_cast<int>(Database_->Query(DB_MinPremType)));
    return GetModalPrem(Year, Mode, SpecAmt, PremType);
}

//============================================================================
double BasicValues::GetModalTgtPrem
    (int           Year
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    e_modal_prem_type const PremType =
        static_cast<e_modal_prem_type>(static_cast<int>(Database_->Query(DB_TgtPremType)));
    double modal_prem = GetModalPrem(Year, Mode, SpecAmt, PremType);

#if 0
// Authors of this ifdefed-out block: GWC and JLM.
// TODO ?? JOE--Please see comments
        // Some products define only an annual target premium, with
        // no directions for calculating a non-annual policy fee.
// JOE--do we need to define a modal target premium?
        // The program shall complain
        // if a non-annual taget premium is requested.
// JOE--
// I had asked:
// why compare an enum to an int, and also to a value it cannot assume?
// IOW, given:
//enum EMode
//  {e_annual       = 1
//  ,e_semiannual   = 2
//  ,e_quarterly    = 4
//  ,e_monthly      = 12
//  };
// how can a variable of type Mode have the value zero?
//
// And if you mean to assert that it's annual, why not compare to e_annual?
// Even if you compared it to one rather than zero, how maintainable is that,
// and what happens if we ever change the value of the enumerator e_annual?
// Changing
//      if(0 == Mode)
// to
//      LMI_ASSERT(0 == Mode);
// does not address this, and actually makes correct code incorrect (why?),
// so the proposed replacement of if...else below with
//      LMI_ASSERT(0 == Mode);
//      modal_prem += POLICYFEE;
// seems like something we shouldn't do.
//
// That is not to say that
//      LMI_ASSERT(0 == Mode);
// or even
//      LMI_ASSERT(e_annual == Mode);
// would be good: that would allow a non-annual mode to be entered, but
// give a runtime error. Either non-annual modes are OK and should be
// supported, or they are not OK. What harm is done by the change below?

/* Changed from:
        if(0 == Mode)
            {
            modal_prem += POLICYFEE;
            }
        else
            {
            // Not a precise method for modalizing the tgt prem's policy fee?
            // What should we do if a product defines only an annual target
            // premium, but a modal one is wanted?
            modal_prem += POLICYFEE / Mode;
            }
*/
// Changed to:
//      modal_prem += POLICYFEE / Mode;
// TODO ?? WE MUST DEFINE POLICYFEE HERE
#endif // 0

    return modal_prem;
}

//============================================================================
double BasicValues::GetModalPrem
    (int                      Year
    ,e_mode            const& Mode
    ,double                   SpecAmt
    ,e_modal_prem_type const& PremType
    ) const
{
    if(e_monthly_deduction == PremType)
        {
        return GetModalPremMlyDed(Year, Mode, SpecAmt);
        }
    else if(e_modal_nonmec == PremType)
        {
        return GetModalPremMaxNonMec(Year, Mode, SpecAmt);
        }
    else if(e_modal_table == PremType)
        {
        // The fn s/b generalized to allow an input premium file and an input
        // policy fee. If e_modal_table is ever used for other than tgt prem,
        // it will be wrong. TODO ?? Fix this.
        return GetModalPremTgtFromTable(Year, Mode, SpecAmt);
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << PremType << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

//============================================================================
double BasicValues::GetModalPremMaxNonMec
    (int           // Year TODO ?? Unused for now.
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    double temp = MortalityRates_->SevenPayRates()[0];
    // always use initial spec amt and mode--fixed at issue
    // round down--mustn't violate 7702A
    return round_max_premium(temp * epsilon_plus_one * SpecAmt / Mode);
}

//============================================================================
double BasicValues::GetModalPremTgtFromTable
    (int           // Year TODO ?? Unused for now.
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    return round_max_premium
        (
            (   Database_->Query(DB_TgtPremPolFee)
            +       SpecAmt
                *   epsilon_plus_one
                *   MortalityRates_->TargetPremiumRates()[0]
            )
        /   Mode
        );
}

//============================================================================
double BasicValues::GetModalPremCorridor
    (int           // Year TODO ?? Unused for now.
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    double temp = MortalityRates_->CvatCorridorFactors()[0];
    // always use initial spec amt and mode--fixed at issue
    // round down--mustn't violate 7702A
    return round_max_premium((epsilon_plus_one * SpecAmt / temp) / Mode);
}

//============================================================================
double BasicValues::GetModalPremGLP
    (int           a_Duration
    ,e_mode const& a_Mode
    ,double        a_BftAmt
    ,double        a_SpecAmt
    ) const
{
    double z = Irc7702_->CalculateGLP
        (a_Duration
        ,a_BftAmt
        ,a_SpecAmt
        ,Irc7702_->GetLeastBftAmtEver()
        ,Get7702EffectiveDBOpt(DeathBfts_->dbopt()[0])
        );

// TODO ?? PROBLEMS HERE
// what if Year != 0 ?
// term rider, dumpin

    z /= a_Mode;
    return round_max_premium(epsilon_plus_one * z);
}

//============================================================================
double BasicValues::GetModalPremGSP
    (int           a_Duration
    ,e_mode const& a_Mode
    ,double        a_BftAmt
    ,double        a_SpecAmt
    ) const
{
    double z = Irc7702_->CalculateGSP
        (a_Duration
        ,a_BftAmt
        ,a_SpecAmt
        ,Irc7702_->GetLeastBftAmtEver()
        );

// TODO ?? PROBLEMS HERE
// what if Year != 0 ?
// term rider, dumpin

    z /= a_Mode;
    return round_max_premium(epsilon_plus_one * z);
}

//============================================================================
double BasicValues::GetModalPremMlyDed
    (int           Year
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    double z = SpecAmt;
    z /=
        (   1.0
        +   InterestRates_->GenAcctNetRate
                (e_basis(e_guarbasis)
                ,e_rate_period(e_monthly_rate)
                )[Year]
        );
    z *= GetBandedCoiRates(e_basis(e_currbasis), SpecAmt)[Year];
    z += Loads_->monthly_policy_fee(e_basis(e_currbasis))[Year];

    if(Input_->Status[0].HasADD)
        {
        z +=
                MortalityRates_->ADDRates()[Year]
            *   std::min(SpecAmt, ADDLimit)
            ;
        }
    // TODO ?? Other riders should be considered here.

    double annual_charge = Loads_->annual_policy_fee(e_basis(e_currbasis))[Year];

    if(Input_->Status[0].HasWP)
        {
        // TODO ?? For simplicity, ignore Database_->Query(DB_WPMax)
        double r = MortalityRates_->WPRates()[Year];
        z *= 1.0 + r;
        annual_charge *= 1.0 + r;
        }

    // TODO ?? Only *target* load?
    z /= 1.0 - Loads_->target_total_load(e_basis(e_currbasis))[Year];

    z *= GetAnnuityValueMlyDed(Year, Mode);
    z += annual_charge;

    return round_min_premium(z);
}

//============================================================================
double BasicValues::GetModalSpecAmtMax
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    e_modal_prem_type const prem_type = static_cast<e_modal_prem_type>
        (static_cast<int>(Database_->Query(DB_MinPremType))
        );
    return GetModalSpecAmt
            (a_EeMode
            ,a_EePmt
            ,a_ErMode
            ,a_ErPmt
            ,prem_type
            );
}

//============================================================================
double BasicValues::GetModalSpecAmtTgt
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    e_modal_prem_type const prem_type = static_cast<e_modal_prem_type>
        (static_cast<int>(Database_->Query(DB_TgtPremType))
        );
    return GetModalSpecAmt
            (a_EeMode
            ,a_EePmt
            ,a_ErMode
            ,a_ErPmt
            ,prem_type
            );
}

//============================================================================
// TODO ?? No 'Year' argument?
double BasicValues::GetModalSpecAmt
    (e_mode     const& a_EeMode
    ,double            a_EePmt
    ,e_mode     const& a_ErMode
    ,double            a_ErPmt
    ,e_modal_prem_type a_PremType
    ) const
{
    if(e_monthly_deduction == a_PremType)
        {
        return GetModalSpecAmtMlyDed
            (a_EeMode
            ,a_EePmt
            ,a_ErMode
            ,a_ErPmt
            );
        }
    else if(e_modal_nonmec == a_PremType)
        {
        return GetModalSpecAmtMinNonMec
            (a_EeMode
            ,a_EePmt
            ,a_ErMode
            ,a_ErPmt
            );
        }
    else if(e_modal_table == a_PremType)
        {
        // TODO ?? This is dubious. If the table specified is a
        // seven-pay table, then this seems not to give the same
        // result as the seven-pay premium type.
        double annualized_pmt = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
        return round_min_specamt
            (annualized_pmt / GetModalPremTgtFromTable(0, a_EeMode, 1)
            );
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << a_PremType << '.'
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

//============================================================================
double BasicValues::GetModalSpecAmtMinNonMec
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    double annualized_pmt = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
    return round_min_specamt
        (annualized_pmt / MortalityRates_->SevenPayRates()[0]
        );
}

//============================================================================
double BasicValues::GetModalSpecAmtGLP
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    double annualized_pmt = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
// TODO ?? Duration 0 hardcoded here, but prem functions use actual duration.
    return Irc7702_->CalculateGLPSpecAmt
        (0
        ,annualized_pmt
        ,Get7702EffectiveDBOpt(DeathBfts_->dbopt()[0])
        );
// TODO ?? This should already be rounded, and rounding it again should
// only be harmful. Expunge after testing and after reconsidering all
// other code that returns rounded specamts.
//    return round_min_specamt(z);
}

//============================================================================
double BasicValues::GetModalSpecAmtGSP
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    double annualized_pmt = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
// TODO ?? Duration 0 hardcoded here, but prem functions use actual duration.
    return Irc7702_->CalculateGSPSpecAmt
        (0
        ,annualized_pmt
        );
// TODO ?? This should already be rounded, and rounding it again should
// only be harmful. Expunge after testing and after reconsidering all
// other code that returns rounded specamts.
//    return round_min_specamt(z);
}

//============================================================================
double BasicValues::GetModalSpecAmtCorridor
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
    double annualized_pmt = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
    double rate = MortalityRates_->CvatCorridorFactors()[0];
    // always use initial spec amt and mode--fixed at issue
    // round up--mustn't violate 7702A
    return round_min_specamt(annualized_pmt * rate);
}

//============================================================================
double BasicValues::GetModalSpecAmtMlyDed
    (e_mode const& a_EeMode
    ,double        a_EePmt
    ,e_mode const& a_ErMode
    ,double        a_ErPmt
    ) const
{
// TODO ?? This is broken. It is supposed to take mode into account,
// but what should be the behavior if ee and er both pay and their modes differ?
// For now, we just assume that ee mode governs...only a guess...
    e_mode guess_mode = a_EeMode;
    double z = a_EeMode * a_EePmt + a_ErMode * a_ErPmt;
    z /= guess_mode;

    double annual_charge = Loads_->annual_policy_fee(e_basis(e_currbasis))[0];

    double wp_rate = 0.0;
    if(Input_->Status[0].HasWP)
        {
        // For simplicity, ignore Database_->Query(DB_WPMax)
        wp_rate = MortalityRates_->WPRates()[0];
        if(0.0 != 1.0 + wp_rate)
            {
            annual_charge /= (1.0 + wp_rate);
            }
        }

    z -= annual_charge;

    // TODO ?? Use first-year values only--don't want this to vary by year.
    z /= GetAnnuityValueMlyDed(0, guess_mode);
    // TODO ?? only *target* load?
// TODO ?? Looks like none of our test decks exercise this line.
    z *= 1.0 - Loads_->target_total_load(e_basis(e_currbasis))[0];

    // TODO ?? Is this correct now?
    if(Input_->Status[0].HasWP && 0.0 != 1.0 + wp_rate)
        {
        // For simplicity, ignore Database_->Query(DB_WPMax)
        z /= (1.0 + wp_rate);
        }

    if(Input_->Status[0].HasADD)
        {
        // TODO ?? For simplicity, ignore Database_->Query(DB_ADDMax)
        z -= MortalityRates_->ADDRates()[0];
        }
    // TODO ?? Other riders should be considered here.

    z -= Loads_->monthly_policy_fee(e_basis(e_currbasis))[0];
    // TODO ?? Probably we should respect banding. This is a
    // conservtive shortcut.
    z /= MortalityRates_->MonthlyCoiRatesBand0(e_basis(e_currbasis))[0];
    z *= 1.0 + InterestRates_->GenAcctNetRate
        (e_basis(e_guarbasis)
        ,e_rate_period(e_monthly_rate)
        )[0]
        ;

    return round_max_specamt(z);
}

//============================================================================
// 'Unusual' banding is one particular approach we needed to model.
// Simpler than the banding method generally used in the industry, it
// determines a single COI rate from the total specified amount and
// applies that single rate to the entire NAAR. No layers of coverage
// are distinguished.
std::vector<double> const& BasicValues::GetBandedCoiRates
    (e_basis const& rate_basis
    ,double specamt
    ) const
{
    if(UseUnusualCOIBanding && e_guarbasis != rate_basis)
        {
        double band_0_limit = Database_->Query(DB_CurrCOITable0Limit);
        double band_1_limit = Database_->Query(DB_CurrCOITable1Limit);
        LMI_ASSERT(0.0 <= band_0_limit);
        LMI_ASSERT(band_0_limit <= band_1_limit);
        if(band_0_limit <= specamt && specamt < band_1_limit)
            {
            return MortalityRates_->MonthlyCoiRatesBand1(rate_basis);
            }
        else if(band_1_limit <= specamt)
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

//============================================================================
double BasicValues::GetAnnuityValueMlyDed
    (int           Year
    ,e_mode const& Mode
    ) const
{
    LMI_ASSERT(0.0 != Mode);
    double spread = 0.0;
    if(e_monthly != Mode)
        {
        spread = MinPremIntSpread_[Year] * 1.0 / Mode;
        }
    double z = i_upper_12_over_12_from_i<double>()
        (   Input_->GenAcctRate[Year]
        -   spread
        );
// TODO ?? What do we do if SA and GA current rates differ?
    double u = 1.0 + std::max
        (z
        ,InterestRates_->GenAcctNetRate
            (e_basis(e_guarbasis)
            ,e_rate_period(e_monthly_rate)
            )[Year]
        );
    u = 1.0 / u;
    return (1.0 - std::pow(u, 12.0 / Mode)) / (1.0 - u);
}

//============================================================================
std::vector<double> BasicValues::GetInforceAdjustedTable
    (std::string const& TableFile
    ,long int           TableID
    ,long int           TableNumber
    ) const
{
    if(DB_CurrCOITable == TableID && Database_->Query(DB_CoiInforceReentry))
        {
        int inforce_year = Input_->InforceYear;
        std::vector<double> v = actuarial_table
            (TableFile
            ,TableNumber
            ,GetIssueAge() + inforce_year
            ,GetLength()   - inforce_year
            );
        v.insert(v.begin(), inforce_year, 0.0);
        return v;
        }
    else
        {
        return actuarial_table
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
    ,long int           TableID
    ) const
{
    return GetInforceAdjustedTable
        (TableFile
        ,TableID
        ,static_cast<long int>(Database_->Query(TableID))
        );
}

//============================================================================
std::vector<double> BasicValues::GetUnblendedTable
    (std::string const& TableFile
    ,long int           TableID
    ,e_gender    const& gender
    ,e_smoking   const& smoking
    ) const
{
    InputParms IP(*Input_);
    IP.Status[0].Gender = e_gender(gender);
    IP.Status[0].Smoking = e_smoking(smoking);

    TDatabase TempDatabase(IP);

    return GetInforceAdjustedTable
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
    ,long int    const& TableID
    ,EBlend      const& CanBlendSmoking
    ,EBlend      const& CanBlendGender
    ) const
{
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
            BlendSmoking = Input_->BlendMortSmoking;
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
            BlendGender = Input_->BlendMortGender;
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
                !Input_->BlendMortSmoking
            &&  !Input_->BlendMortGender
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
            BlendSmoking // Input_->BlendMortSmoking
        &&  !BlendGender // Input_->BlendMortGender
        )
        {
        std::vector<double> S = GetUnblendedTable
            (file_name
            ,TableID
            ,Input_->Status[0].Gender
            ,e_smoking(e_smoker)
            );
        std::vector<double> N = GetUnblendedTable
            (file_name
            ,TableID
            ,Input_->Status[0].Gender
            ,e_smoking(e_nonsmoker)
            );
        double n = Input_->NonsmokerProportion;
        double s = 1.0 - n;
        for(int j = 0; j < GetLength(); j++)
            {
            BlendedTable.push_back(s * S[j] + n * N[j]);
            }
        }

    // Case 3: blend by gender only
    else if
        (
            !BlendSmoking // Input_->BlendMortSmoking
        &&  BlendGender // Input_->BlendMortGender
        )
        {
        std::vector<double> F = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_female)
            ,Input_->Status[0].Smoking
            );
        std::vector<double> M = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_male)
            ,Input_->Status[0].Smoking
            );
        double m = Input_->MaleProportion;
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
            BlendSmoking // Input_->BlendMortSmoking
        &&  BlendGender // Input_->BlendMortGender
        )
        {
        std::vector<double> FS = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_female)
            ,e_smoking(e_smoker)
            );
        std::vector<double> FN = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_female)
            ,e_smoking(e_nonsmoker)
            );
        std::vector<double> MS = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_male)
            ,e_smoking(e_smoker)
            );
        std::vector<double> MN = GetUnblendedTable
            (file_name
            ,TableID
            ,e_gender(e_male)
            ,e_smoking(e_nonsmoker)
            );
        double n = Input_->NonsmokerProportion;
        double s = 1.0 - n;
        double m = Input_->MaleProportion;
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
// TODO ?? This might be reworked to go through class Irc7702 all the time.
// TODO ?? The profusion of similar names should be trimmed.
std::vector<double> const& BasicValues::GetCorridorFactor() const
{
    switch(Input_->DefnLifeIns)
        {
        case e_cvat:
            {
            return MortalityRates_->CvatCorridorFactors();
            }
            // break;
        case e_gpt:
            {
            return Irc7702_->Corridor();
            }
            // break;
        case e_noncompliant:
            {
            // TODO ?? Why not have this function return the vector it creates?
            CalculateNon7702CompliantCorridor();
            return Non7702CompliantCorridor;
            }
            // break;
        default:
            {
            fatal_error()
                << "Case '"
                << Input_->DefnLifeIns
                << "' not found."
                << LMI_FLUSH
                ;
            }
            break;
        }

    static std::vector<double> z;
    return z;
}

//============================================================================
// Authors: GWC and JLM.
// TODO ?? Why not have this function return the vector it creates?
void BasicValues::CalculateNon7702CompliantCorridor() const
// TODO ?? JOE--What does a comment such as this add?
// non 7702 compliant corridor
{
    // TODO ?? JOE--Please do clean this up. How many style rules
    // are violated here? Do you understand the rationale for
    // those rules?

    // Interpolate the input coridor factor from the pivot age to 1.0 by
    // age 95. This notion is based on GPT corridor factors.
    Non7702CompliantCorridor.resize(Length);

    double pivot_age = Database_->Query(DB_NonUSCorridorPivot);

    LMI_ASSERT( (pivot_age >= 0.0)
            &&(pivot_age < 95.0)
            );

    double non_us_corr = Input_->NonUSCorridor;
    double increment = (non_us_corr - 1.0)
                     / (95.0 - pivot_age)
                     ;

    for(int index=0; index < Length; index++)
        {
        if(index + IssueAge <= pivot_age)
            {
            Non7702CompliantCorridor[index]=non_us_corr;
            }
        else if(index + IssueAge > 95)
            {
            Non7702CompliantCorridor[index]=1.0;
            }
        else
            {
            double interp_corr = non_us_corr
                               - (index + IssueAge - pivot_age)
                               * increment
                               ;
            Non7702CompliantCorridor[index]=interp_corr;
            }
        Non7702CompliantCorridor[index] = round_corridor_factor
            (Non7702CompliantCorridor[index]
            );
        }
}

// 7702 recognizes death benefit options 1 and 2 only. A contract
// might have a death benefit option other than the usual two, but
// for 7702 (and 7702A) purposes it's treated as either option 1
// or option 2.
//
//============================================================================
e_dbopt_7702 const BasicValues::Get7702EffectiveDBOpt
    (e_dbopt const& a_DBOpt
    ) const
{
    e_dbopt_7702 z(e_option1_for_7702);
    switch(a_DBOpt)
        {
        case e_option1:
            {
            z = e_option1_for_7702;
            }
            break;
        case e_option2:
            {
            z = e_option2_for_7702;
            }
            break;
        case e_rop:
            {
            z = Equiv7702DBO3;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << a_DBOpt
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    return z;
}

//============================================================================
e_dbopt const BasicValues::Get7702EffectiveDBOpt
    (e_dbopt_7702 const& a_DBOpt
    ) const
{
    e_dbopt z(e_option1);
    switch(a_DBOpt)
        {
        case e_option1_for_7702:
            {
            z = e_option1;
            }
            break;
        case e_option2_for_7702:
            {
            z = e_option2;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << a_DBOpt
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
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

// Only current (hence midpoint) COI and term rates are blended

std::vector<double> BasicValues::GetCvatCorridorFactors() const
{
    return GetTable
        (ProductData_->GetCorridorFilename()
        ,DB_CorridorTable
        );
}
std::vector<double> BasicValues::GetCurrCOIRates0() const
{
    return GetTable
        (ProductData_->GetCurrCOIFilename()
        ,DB_CurrCOITable, CanBlend, CanBlend
        );
}
std::vector<double> BasicValues::GetCurrCOIRates1() const
{
    if
        ( Database_->Query(DB_CurrCOITable0Limit)
        < std::numeric_limits<double>::max()
        )
        {
        return GetTable
            (ProductData_->GetCurrCOIFilename()
            ,DB_CurrCOITable1, CanBlend, CanBlend
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
        ( Database_->Query(DB_CurrCOITable1Limit)
        < std::numeric_limits<double>::max()
        )
        {
        return GetTable
            (ProductData_->GetCurrCOIFilename()
            ,DB_CurrCOITable2, CanBlend, CanBlend
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
        (ProductData_->GetGuarCOIFilename()
        ,DB_GuarCOITable
        );
}
std::vector<double> BasicValues::GetSmokerBlendedGuarCOIRates() const
{
    return GetTable
        (ProductData_->GetGuarCOIFilename()
        ,DB_GuarCOITable, CanBlend, CanBlend
        );
}
std::vector<double> BasicValues::GetWPRates() const
{
    return GetTable
        (ProductData_->GetWPFilename()
        ,DB_WPTable
        );
}
std::vector<double> BasicValues::GetADDRates() const
{
    return GetTable
        (ProductData_->GetADDFilename()
        ,DB_ADDTable
        );
}
std::vector<double> BasicValues::GetChildRiderRates() const
{
    return GetTable
        (ProductData_->GetChildRiderFilename()
        ,DB_ChildRiderTable
        );
}
std::vector<double> BasicValues::GetCurrentSpouseRiderRates() const
{
    std::vector<double> z = actuarial_table
        (AddDataDir(ProductData_->GetCurrSpouseRiderFilename())
        ,static_cast<long int>(Database_->Query(DB_SpouseRiderTable))
        ,Input_->SpouseIssueAge
        ,static_cast<int>(Database_->Query(DB_EndtAge)) - Input_->SpouseIssueAge
        );
    z.resize(Length);
    return z;
}
std::vector<double> BasicValues::GetGuaranteedSpouseRiderRates() const
{
    std::vector<double> z = actuarial_table
        (AddDataDir(ProductData_->GetGuarSpouseRiderFilename())
        ,static_cast<long int>(Database_->Query(DB_SpousRiderGuarTable))
        ,Input_->SpouseIssueAge
        ,static_cast<int>(Database_->Query(DB_EndtAge)) - Input_->SpouseIssueAge
        );
    z.resize(Length);
    return z;
}
std::vector<double> BasicValues::GetCurrentTermRates() const
{
    return GetTable
        (ProductData_->GetCurrTermFilename()
        ,DB_TermTable, CanBlend, CanBlend
        );
}
std::vector<double> BasicValues::GetGuaranteedTermRates() const
{
    return GetTable
        (ProductData_->GetGuarTermFilename()
        ,DB_GuarTermTable, CanBlend, CanBlend
        );
}
std::vector<double> BasicValues::GetTableYRates() const
{
    return GetTable
        (ProductData_->GetTableYFilename()
        ,DB_TableYTable
        );
}
std::vector<double> BasicValues::GetTAMRA7PayRates() const
{
    return GetTable
        (ProductData_->GetTAMRA7PayFilename()
        ,DB_TAMRA7PayTable
        );
}
std::vector<double> BasicValues::GetTgtPremRates() const
{
    return GetTable
        (ProductData_->GetTgtPremFilename()
        ,DB_TgtPremTable
        );
}
std::vector<double> BasicValues::GetIRC7702Rates() const
{
    return GetTable
        (ProductData_->GetIRC7702Filename()
        ,DB_IRC7702QTable
        );
}
std::vector<double> BasicValues::Get83GamRates() const
{
    return GetTable
        (ProductData_->GetGam83Filename()
        ,DB_83GamTable, CannotBlend, CanBlend
        );
}

