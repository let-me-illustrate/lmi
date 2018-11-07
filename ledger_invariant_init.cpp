// Ledger data that do not vary by basis--initialization.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ledger_invariant.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "fund_data.hpp"
#include "interest_rates.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "loads.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "miscellany.hpp"               // each_equal()
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "product_data.hpp"
#include "ssize_lmi.hpp"

#include <algorithm>                    // max()
#include <stdexcept>

void LedgerInvariant::Init(BasicValues const* b)
{
    // Zero-initialize almost everything.
    Init();

    irr_precision_ = b->round_irr().decimals();

// EePmt and ErPmt are *input* values, used only as a kludge, e.g. in
// premium-strategy calculations. Use E[er]GrossPmt for illustrations:
// they're *output* values that result from transaction processing.

    EePmt           = b->Outlay_->ee_modal_premiums();
    ErPmt           = b->Outlay_->er_modal_premiums();
//  TgtPrem         =
//  GrossPmt        =
//  EeGrossPmt      =
//  ErGrossPmt      =
    // These must be set dynamically because they may be changed,
    // e.g. to respect guideline limits.
//    External1035Amount;
//    Internal1035Amount;
//    Dumpin               =

    InforceUnloanedAV =
          b->yare_input_.InforceGeneralAccountValue
        + b->yare_input_.InforceSeparateAccountValue
        ;
    InforceTaxBasis      = b->yare_input_.InforceTaxBasis           ;

    // Certain data members, including but almost certainly not
    // limited to these, should not be initialized to any non-zero
    // value here. Actual values are inserted in account-value
    // processing, subject to various restrictions that often cause
    // them to differ from input values. Notably, values need to be
    // zero after lapse.
//    NetWD                 =
//    NewCashLoan           =
//    GptForceout           =
//    NaarForceout          =
//    ModalMinimumPremium   =
//    EeModalMinimumPremium =
//    ErModalMinimumPremium =

    HasSupplSpecAmt = false;
    if(b->yare_input_.TermRider)
        {
        TermSpecAmt     .assign(Length, b->yare_input_.TermRiderAmount);
        }
    else if(b->database().query<bool>(DB_TermIsNotRider))
        {
        TermSpecAmt      = b->DeathBfts_->supplamt();
        if(!each_equal(TermSpecAmt, 0.0))
            {
            HasSupplSpecAmt = true;
            }
        }
    else
        {
        TermSpecAmt     .assign(Length, 0.0);
        }
    SpecAmt         = b->DeathBfts_->specamt();
    for(int j = 0; j < Length; ++j)
        {
        EeMode[j] = b->Outlay_->ee_premium_modes()[j];
        ErMode[j] = b->Outlay_->er_premium_modes()[j];
        DBOpt [j] = b->DeathBfts_->dbopt()[j];
        }

    IndvTaxBracket       = b->yare_input_.TaxBracket                ;
    CorpTaxBracket       = b->yare_input_.CorporationTaxBracket     ;
    Salary               = b->yare_input_.ProjectedSalary           ;
    AnnualFlatExtra      = b->yare_input_.FlatExtra                 ;
    HoneymoonValueSpread = b->yare_input_.HoneymoonValueSpread      ;
    AddonMonthlyFee      = b->yare_input_.ExtraMonthlyCustodialFee  ;
    AddonCompOnAssets    = b->yare_input_.ExtraCompensationOnAssets ;
    AddonCompOnPremium   = b->yare_input_.ExtraCompensationOnPremium;
    CorridorFactor       = b->GetCorridorFactor();
    AnnLoanDueRate       = b->InterestRates_->RegLnDueRate
        (mce_gen_curr
        ,mce_annual_rate
        );
    InitAnnLoanDueRate   = AnnLoanDueRate[0];
    CurrMandE            = b->InterestRates_->MAndERate(mce_gen_curr);
    TotalIMF             = b->InterestRates_->InvestmentManagementFee();
    RefundableSalesLoad  = b->Loads_->refundable_sales_load_proportion();

    std::vector<double> coimult;
    b->database().query_into(DB_CurrCoiMultiplier, coimult);
    CurrentCoiMultiplier =
          coimult                            [b->yare_input_.InforceYear]
        * b->yare_input_.CurrentCoiMultiplier[b->yare_input_.InforceYear]
        * b->yare_input_.CountryCoiMultiplier
        ;

    CountryIso3166Abbrev = mc_str(b->yare_input_.Country);
    Comments             = b->yare_input_.Comments;

    FundNumbers           .resize(0);
    FundNames             .resize(0);
    FundAllocs            .resize(0);
    FundAllocations       .resize(0);

    // The antediluvian branch doesn't meaningfully initialize class FundData.
    int number_of_funds(0);
    if(!is_antediluvian_fork())
        {
        number_of_funds = b->FundData_->GetNumberOfFunds();
        }

//    enum{NumberOfFunds = 30}; // DEPRECATED
    int const NumberOfFunds = 30; // DEPRECATED
    int expected_number_of_funds = std::max(number_of_funds, NumberOfFunds);
    std::vector<double> v(b->yare_input_.FundAllocations);
    if(lmi::ssize(v) < expected_number_of_funds)
        {
        v.insert(v.end(), expected_number_of_funds - v.size(), 0.0);
        }

    for(int j = 0; j < number_of_funds; ++j)
        {
        FundNumbers.push_back(j);
        FundNames.push_back(b->FundData_->GetFundInfo(j).LongName());

        // TODO ?? InputParms::NumberOfFunds is defectively hardcocded
        // as thirty as this is written, but we need to support a product
        // with more than thirty funds. The input routines respect that
        // hardcoded limit and are difficult to change, so funds after
        // the thirtieth cannot be selected individually; but if the
        // rule 'equal initial fund allocations' is chosen instead of
        // specifying individual allocations, then the average fund fee
        // is calculated reflecting all funds, even past the thirtieth:
        // thus, calculations are correct for any input, and the defect
        // in the program itself is just that some legitimate inputs are
        // not allowed, though the output spreadsheet has its own
        // hardcoded limit (due to space), which is a separate defect.
        // Here we pass a zero allocation to the output spreadsheet for
        // all funds past the thirtieth, which is correct because no
        // nonzero allocation can be selected. That's correct even if
        // the 'equal initial allocations' rule is chosen, in which
        // case the allocations are not explicitly shown, but are instead
        // stated in words to be equal--because the spreadsheet layout
        // otherwise shows allocations as integer percentages, and
        // something like '.3333333...' would overflow the space available.
        //
        // As of 2008, most of the foregoing is no longer applicable,
        // except for the hardcoded limit, which is copied above.
        FundAllocs     .push_back(static_cast<int>(v[j]));
        FundAllocations.push_back(0.01 * v[j]);
        }

    GenAcctAllocation = 1.0 - premium_allocation_to_sepacct(b->yare_input_);

    SplitFundAllocation =
            (0.0 != GenAcctAllocation && 1.0 != GenAcctAllocation)
        ||
            (  0.0 != b->yare_input_.InforceGeneralAccountValue
            && 0.0 != b->yare_input_.InforceSeparateAccountValue
            )
        ;

    b->database().query_into(DB_NoLapseAlwaysActive, NoLapseAlwaysActive);
    b->database().query_into(DB_NoLapseMinDur      , NoLapseMinDur);
    b->database().query_into(DB_NoLapseMinAge      , NoLapseMinAge);
    b->database().query_into(DB_Has1035ExchCharge  , Has1035ExchCharge);

    // SOMEDAY !! Things indexed with '[0]' should probably use inforce year instead.
    InitBaseSpecAmt         = b->DeathBfts_->specamt()[0];
    InitTermSpecAmt         = TermSpecAmt[0];
    ChildRiderAmount        = b->yare_input_.ChildRiderAmount;
    SpouseRiderAmount       = b->yare_input_.SpouseRiderAmount;

//  InitPrem                = 0;
//  GuarPrem                = 0;
//  InitSevenPayPrem        =
//  InitTgtPrem             =
//  InitMinPrem             =
//  ListBillPremium         =
//  EeListBillPremium       =
//  ErListBillPremium       =
//  ModalMinimumDumpin      =

    MaleProportion          = b->yare_input_.MaleProportion;
    NonsmokerProportion     = b->yare_input_.NonsmokerProportion;
    PartMortTableMult       = b->yare_input_.PartialMortalityMultiplier;

    // Assert this because the illustration currently prints a scalar
    // guaranteed max, assuming that it's the same for all years.
    std::vector<double> const& guar_m_and_e_rate = b->InterestRates_->MAndERate
        (mce_gen_guar
        );
    LMI_ASSERT(each_equal(guar_m_and_e_rate, guar_m_and_e_rate.front()));
    GuarMaxMandE            = guar_m_and_e_rate[0];
    InitDacTaxRate          = b->Loads_->dac_tax_load()[b->yare_input_.InforceYear];
    InitPremTaxRate         = b->PremiumTax_->maximum_load_rate();
//  GenderDistinct          = 0;
    GenderBlended           = b->yare_input_.BlendGender;
//  SmokerDistinct          = 0;
    SmokerBlended           = b->yare_input_.BlendSmoking;

    SubstdTable             = b->yare_input_.SubstandardTable;

    Age                     = b->yare_input_.IssueAge;
    RetAge                  = b->yare_input_.RetirementAge;
    EndtAge                 = b->yare_input_.IssueAge + b->GetLength();
    b->database().query_into(DB_GroupIndivSelection, GroupIndivSelection);
    UseExperienceRating     = b->yare_input_.UseExperienceRating;
    UsePartialMort          = b->yare_input_.UsePartialMortality;
    AvgFund                 = b->yare_input_.UseAverageOfAllFunds;
    CustomFund              = b->yare_input_.OverrideFundManagementFee;

    HasWP                   = b->yare_input_.WaiverOfPremiumBenefit;
    HasADD                  = b->yare_input_.AccidentalDeathBenefit;
    HasTerm                 = b->yare_input_.TermRider;
    HasChildRider           = b->yare_input_.ChildRider;
    HasSpouseRider          = b->yare_input_.SpouseRider;
    SpouseIssueAge          = b->yare_input_.SpouseIssueAge;

    HasHoneymoon            = b->yare_input_.HoneymoonEndorsement;
    PostHoneymoonSpread     = b->yare_input_.PostHoneymoonSpread;
    b->database().query_into(DB_SplitMinPrem       , SplitMinPrem);
    b->database().query_into(DB_AllowDbo3          , AllowDbo3);

    // These are reassigned below based on product data if available.
    std::string dbo_name_option1 = mc_str(mce_option1);
    std::string dbo_name_option2 = mc_str(mce_option2);
    std::string dbo_name_rop     = mc_str(mce_rop    );
    std::string dbo_name_mdb     = mc_str(mce_mdb    );

    // The antediluvian branch doesn't meaningfully initialize class product_data.
    if(!is_antediluvian_fork())
        {
        product_data const& p = b->product();
        // Accommodate one alternative policy-form name.
        // DATABASE !! It would be much better, of course, to let all
        // strings in class product_data vary across the same axes as
        // database_entity objects.
        bool alt_form = b->database().query<bool>(DB_UsePolicyFormAlt);
        dbo_name_option1               = p.datum("DboNameLevel"                   );
        dbo_name_option2               = p.datum("DboNameIncreasing"              );
        dbo_name_rop                   = p.datum("DboNameReturnOfPremium"         );
        dbo_name_mdb                   = p.datum("DboNameMinDeathBenefit"         );
        PolicyForm = p.datum(alt_form ? "PolicyFormAlternative" : "PolicyForm");
        PolicyMktgName                 = p.datum("PolicyMktgName"                 );
        PolicyLegalName                = p.datum("PolicyLegalName"                );
        InsCoShortName                 = p.datum("InsCoShortName"                 );
        InsCoName                      = p.datum("InsCoName"                      );
        InsCoAddr                      = p.datum("InsCoAddr"                      );
        InsCoStreet                    = p.datum("InsCoStreet"                    );
        InsCoPhone                     = p.datum("InsCoPhone"                     );
        MainUnderwriter                = p.datum("MainUnderwriter"                );
        MainUnderwriterAddress         = p.datum("MainUnderwriterAddress"         );
        CoUnderwriter                  = p.datum("CoUnderwriter"                  );
        CoUnderwriterAddress           = p.datum("CoUnderwriterAddress"           );

        AvName                         = p.datum("AvName"                         );
        CsvName                        = p.datum("CsvName"                        );
        CsvHeaderName                  = p.datum("CsvHeaderName"                  );
        NoLapseProvisionName           = p.datum("NoLapseProvisionName"           );
        ContractName                   = p.datum("ContractName"                   );

        AccountValueFootnote           = p.datum("AccountValueFootnote"           );
        AttainedAgeFootnote            = p.datum("AttainedAgeFootnote"            );
        CashSurrValueFootnote          = p.datum("CashSurrValueFootnote"          );
        DeathBenefitFootnote           = p.datum("DeathBenefitFootnote"           );
        InitialPremiumFootnote         = p.datum("InitialPremiumFootnote"         );
        NetPremiumFootnote             = p.datum("NetPremiumFootnote"             );
        GrossPremiumFootnote           = p.datum("GrossPremiumFootnote"           );
        OutlayFootnote                 = p.datum("OutlayFootnote"                 );
        PolicyYearFootnote             = p.datum("PolicyYearFootnote"             );

        ADDTerseName                   = p.datum("ADDTerseName"                   );
        InsurabilityTerseName          = p.datum("InsurabilityTerseName"          );
        ChildTerseName                 = p.datum("ChildTerseName"                 );
        SpouseTerseName                = p.datum("SpouseTerseName"                );
        TermTerseName                  = p.datum("TermTerseName"                  );
        WaiverTerseName                = p.datum("WaiverTerseName"                );
        AccelBftRiderTerseName         = p.datum("AccelBftRiderTerseName"         );
        OverloanRiderTerseName         = p.datum("OverloanRiderTerseName"         );

        ADDFootnote                    = p.datum("ADDFootnote"                    );
        ChildFootnote                  = p.datum("ChildFootnote"                  );
        SpouseFootnote                 = p.datum("SpouseFootnote"                 );
        TermFootnote                   = p.datum("TermFootnote"                   );
        WaiverFootnote                 = p.datum("WaiverFootnote"                 );
        AccelBftRiderFootnote          = p.datum("AccelBftRiderFootnote"          );
        OverloanRiderFootnote          = p.datum("OverloanRiderFootnote"          );

        GroupQuoteShortProductName     = p.datum("GroupQuoteShortProductName"     );
        GroupQuoteIsNotAnOffer         = p.datum("GroupQuoteIsNotAnOffer"         );
        GroupQuoteRidersFooter         = p.datum("GroupQuoteRidersFooter"         );
        GroupQuotePolicyFormId         = p.datum("GroupQuotePolicyFormId"         );
        GroupQuoteStateVariations      = p.datum("GroupQuoteStateVariations"      );
        GroupQuoteProspectus           = p.datum("GroupQuoteProspectus"           );
        GroupQuoteUnderwriter          = p.datum("GroupQuoteUnderwriter"          );
        GroupQuoteBrokerDealer         = p.datum("GroupQuoteBrokerDealer"         );
        GroupQuoteRubricMandatory      = p.datum("GroupQuoteRubricMandatory"      );
        GroupQuoteRubricVoluntary      = p.datum("GroupQuoteRubricVoluntary"      );
        GroupQuoteRubricFusion         = p.datum("GroupQuoteRubricFusion"         );
        GroupQuoteFooterMandatory      = p.datum("GroupQuoteFooterMandatory"      );
        GroupQuoteFooterVoluntary      = p.datum("GroupQuoteFooterVoluntary"      );
        GroupQuoteFooterFusion         = p.datum("GroupQuoteFooterFusion"         );

        MinimumPremiumFootnote         = p.datum("MinimumPremiumFootnote"         );
        PremAllocationFootnote         = p.datum("PremAllocationFootnote"         );

        InterestDisclaimer             = p.datum("InterestDisclaimer"             );
        GuarMortalityFootnote          = p.datum("GuarMortalityFootnote"          );
        ProductDescription             = p.datum("ProductDescription"             );
        StableValueFootnote            = p.datum("StableValueFootnote"            );
        NoVanishPremiumFootnote        = p.datum("NoVanishPremiumFootnote"        );
        RejectPremiumFootnote          = p.datum("RejectPremiumFootnote"          );
        ExpRatingFootnote              = p.datum("ExpRatingFootnote"              );
        MortalityBlendFootnote         = p.datum("MortalityBlendFootnote"         );
        HypotheticalRatesFootnote      = p.datum("HypotheticalRatesFootnote"      );
        SalesLoadRefundFootnote        = p.datum("SalesLoadRefundFootnote"        );
        NoLapseFootnote                = p.datum("NoLapseFootnote"                );
        MarketValueAdjFootnote         = p.datum("MarketValueAdjFootnote"         );
        ExchangeChargeFootnote0        = p.datum("ExchangeChargeFootnote0"        );
        CurrentValuesFootnote          = p.datum("CurrentValuesFootnote"          );
        DBOption1Footnote              = p.datum("DBOption1Footnote"              );
        DBOption2Footnote              = p.datum("DBOption2Footnote"              );
        ExpRatRiskChargeFootnote       = p.datum("ExpRatRiskChargeFootnote"       );
        ExchangeChargeFootnote1        = p.datum("ExchangeChargeFootnote1"        );
        FlexiblePremiumFootnote        = p.datum("FlexiblePremiumFootnote"        );
        GuaranteedValuesFootnote       = p.datum("GuaranteedValuesFootnote"       );
        CreditingRateFootnote          = p.datum("CreditingRateFootnote"          );
        GrossRateFootnote              = p.datum("GrossRateFootnote"              );
        NetRateFootnote                = p.datum("NetRateFootnote"                );
        MecFootnote                    = p.datum("MecFootnote"                    );
        GptFootnote                    = p.datum("GptFootnote"                    );
        MidpointValuesFootnote         = p.datum("MidpointValuesFootnote"         );
        SinglePremiumFootnote          = p.datum("SinglePremiumFootnote"          );
        MonthlyChargesFootnote         = p.datum("MonthlyChargesFootnote"         );
        UltCreditingRateFootnote       = p.datum("UltCreditingRateFootnote"       );
        MaxNaarFootnote                = p.datum("MaxNaarFootnote"                );
        PremTaxSurrChgFootnote         = p.datum("PremTaxSurrChgFootnote"         );
        PolicyFeeFootnote              = p.datum("PolicyFeeFootnote"              );
        AssetChargeFootnote            = p.datum("AssetChargeFootnote"            );
        InvestmentIncomeFootnote       = p.datum("InvestmentIncomeFootnote"       );
        IrrDbFootnote                  = p.datum("IrrDbFootnote"                  );
        IrrCsvFootnote                 = p.datum("IrrCsvFootnote"                 );
        MortalityChargesFootnote       = p.datum("MortalityChargesFootnote"       );
        LoanAndWithdrawalFootnote      = p.datum("LoanAndWithdrawalFootnote"      );
        LoanFootnote                   = p.datum("LoanFootnote"                   );
        ImprimaturPresale              = p.datum("ImprimaturPresale"              );
        ImprimaturPresaleComposite     = p.datum("ImprimaturPresaleComposite"     );
        ImprimaturInforce              = p.datum("ImprimaturInforce"              );
        ImprimaturInforceComposite     = p.datum("ImprimaturInforceComposite"     );
        StateMarketingImprimatur       = p.datum("StateMarketingImprimatur"       );
        InforceNonGuaranteedFootnote0  = p.datum("InforceNonGuaranteedFootnote0"  );
        InforceNonGuaranteedFootnote1  = p.datum("InforceNonGuaranteedFootnote1"  );
        InforceNonGuaranteedFootnote2  = p.datum("InforceNonGuaranteedFootnote2"  );
        InforceNonGuaranteedFootnote3  = p.datum("InforceNonGuaranteedFootnote3"  );
        NonGuaranteedFootnote          = p.datum("NonGuaranteedFootnote"          );
        MonthlyChargesPaymentFootnote  = p.datum("MonthlyChargesPaymentFootnote"  );
        SurrenderFootnote              = p.datum("SurrenderFootnote"              );
        PortabilityFootnote            = p.datum("PortabilityFootnote"            );
        FundRateFootnote               = p.datum("FundRateFootnote"               );
        FundRateFootnote0              = p.datum("FundRateFootnote0"              );
        FundRateFootnote1              = p.datum("FundRateFootnote1"              );
        IssuingCompanyFootnote         = p.datum("IssuingCompanyFootnote"         );
        SubsidiaryFootnote             = p.datum("SubsidiaryFootnote"             );
        PlacementAgentFootnote         = p.datum("PlacementAgentFootnote"         );
        MarketingNameFootnote          = p.datum("MarketingNameFootnote"          );
        }

    ProductName             = b->yare_input_.ProductName;
    ProducerName            = b->yare_input_.AgentName;

    std::string agent_city     = b->yare_input_.AgentCity;
    std::string agent_state    = mc_str(b->yare_input_.AgentState);
    std::string agent_zip_code = b->yare_input_.AgentZipCode;
    std::string agent_city_etc(agent_city + ", " + agent_state);
    if(!agent_zip_code.empty())
        {
        agent_city_etc += " ";
        }
    agent_city_etc += agent_zip_code;

    ProducerStreet          = b->yare_input_.AgentAddress;
    ProducerCity            = agent_city_etc;
    CorpName                = b->yare_input_.CorporationName;

    MasterContractNumber    = b->yare_input_.MasterContractNumber;
    ContractNumber          = b->yare_input_.ContractNumber;

    Insured1                = b->yare_input_.InsuredName;
    Gender                  = mc_str(b->yare_input_.Gender);
    UWType                  = mc_str(b->yare_input_.GroupUnderwritingType);

    // This could be factored out if it ever needs to be reused.
    //
    // DATABASE !! It would make sense to handle it in the product
    // database if class product_data is rewritten to encompass
    // variation across axes (as class DBDictionary does).
    //
    auto const smoke_or_tobacco = b->database().query<oenum_smoking_or_tobacco>(DB_SmokeOrTobacco);
    if(oe_tobacco_nontobacco == smoke_or_tobacco)
        {
        switch(b->yare_input_.Smoking)
            {
            case mce_smoker:    Smoker =    "Tobacco"; break;
            case mce_nonsmoker: Smoker = "Nontobacco"; break;
            case mce_unismoke:  Smoker = "Unitobacco"; break;
            }
        }
    else if(oe_smoker_nonsmoker == smoke_or_tobacco)
        {
        Smoker = mc_str(b->yare_input_.Smoking);
        }
    else
        {
        throw std::logic_error("Unknown oe_smoker_nonsmoker convention.");
        }

    UWClass                 = mc_str(b->yare_input_.UnderwritingClass);
    SubstandardTable        = mc_str(b->yare_input_.SubstandardTable);

    EffDate                 = calendar_date(b->yare_input_.EffectiveDate  ).str();
    EffDateJdn              = calendar_date(b->yare_input_.EffectiveDate  ).julian_day_number();
    DateOfBirth             = calendar_date(b->yare_input_.DateOfBirth    ).str();
    DateOfBirthJdn          = calendar_date(b->yare_input_.DateOfBirth    ).julian_day_number();
    ListBillDate            = calendar_date(b->yare_input_.ListBillDate   ).str();
    ListBillDateJdn         = calendar_date(b->yare_input_.ListBillDate   ).julian_day_number();
    InforceAsOfDate         = calendar_date(b->yare_input_.InforceAsOfDate).str();
    InforceAsOfDateJdn      = calendar_date(b->yare_input_.InforceAsOfDate).julian_day_number();
    InitErMode              = mc_str(b->Outlay_->er_premium_modes()[0]);

    mcenum_dbopt const init_dbo = b->DeathBfts_->dbopt()[0];
    InitDBOpt =
         (mce_option1 == init_dbo) ? dbo_name_option1
        :(mce_option2 == init_dbo) ? dbo_name_option2
        :(mce_rop     == init_dbo) ? dbo_name_rop
        :(mce_mdb     == init_dbo) ? dbo_name_mdb
        :throw std::logic_error("Unrecognized initial death benefit option.")
        ;

    DefnLifeIns             = mc_str(b->yare_input_.DefinitionOfLifeInsurance);
    DefnMaterialChange      = mc_str(b->yare_input_.DefinitionOfMaterialChange);
    AvoidMec                = mc_str(b->yare_input_.AvoidMecMethod);
    PartMortTableName       = "1983 GAM"; // TODO ?? Hardcoded.
    StatePostalAbbrev       = mc_str(b->GetStateOfJurisdiction());
    PremiumTaxState         = mc_str(b->GetPremiumTaxState());

    IsInforce = b->yare_input_.EffectiveDate != b->yare_input_.InforceAsOfDate;

    // This test is probably redundant because it is already performed
    // in class Input. But it's difficult to prove that it is actually
    // redundant and will always remain so, while repeating it here
    // costs little and gives a stronger guarantee that illustrations
    // that would violate this rule cannot be produced.
    if(IsInforce && (0 == b->yare_input_.InforceYear && 0 == b->yare_input_.InforceMonth))
        {
        alarum()
            << "Inforce illustrations not permitted during month of issue."
            << LMI_FLUSH
            ;
        }

    WriteTsvFile = contains(b->yare_input_.Comments, "idiosyncrasyY");

    SupplementalReport         = b->yare_input_.CreateSupplementalReport;
    SupplementalReportColumn00 = mc_str(b->yare_input_.SupplementalReportColumn00);
    SupplementalReportColumn01 = mc_str(b->yare_input_.SupplementalReportColumn01);
    SupplementalReportColumn02 = mc_str(b->yare_input_.SupplementalReportColumn02);
    SupplementalReportColumn03 = mc_str(b->yare_input_.SupplementalReportColumn03);
    SupplementalReportColumn04 = mc_str(b->yare_input_.SupplementalReportColumn04);
    SupplementalReportColumn05 = mc_str(b->yare_input_.SupplementalReportColumn05);
    SupplementalReportColumn06 = mc_str(b->yare_input_.SupplementalReportColumn06);
    SupplementalReportColumn07 = mc_str(b->yare_input_.SupplementalReportColumn07);
    SupplementalReportColumn08 = mc_str(b->yare_input_.SupplementalReportColumn08);
    SupplementalReportColumn09 = mc_str(b->yare_input_.SupplementalReportColumn09);
    SupplementalReportColumn10 = mc_str(b->yare_input_.SupplementalReportColumn10);
    SupplementalReportColumn11 = mc_str(b->yare_input_.SupplementalReportColumn11);

    // irr_initialized_ is deliberately not set here: it's not
    // encompassed by 'FullyInitialized'.
    FullyInitialized = true;
}
