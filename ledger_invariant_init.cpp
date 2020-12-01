// Ledger data that do not vary by basis--initialization.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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
#include "lingo.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "loads.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "miscellany.hpp"               // each_equal()
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "product_data.hpp"
#include "ssize_lmi.hpp"

#include <algorithm>                    // max(), max_element()
#include <stdexcept>

/// Initialize with values determined by BasicValues construction.
///
/// This class's own ctor initializes all its data members, generally
/// to zero unless it can determine a better default. Many members
/// have static values that are determined only when a BasicValues
/// object is constructed; those are set here. This function is
/// defined in its own TU in order to keep the rest of this class
/// separated from BasicValues and its dependencies.
///
/// This function mentions every member, generally in declaration
/// order, to ensure that nothing is overlooked. However, some
/// members are mentioned only as comment lines, because their values
/// may be changed during monthiversary processing; this class's ctor
/// already initialized them, so there's no point in reinitializing
/// them here to the same values.
///
/// It might seem that a member representing payments, e.g., should be
/// initialized here with input values that are known to BasicValues.
/// However, suppose the input payment is $10000 annually, but the
/// contract lapses early: then the original initialization to zero
/// was correct after the lapse year, and overriding it with a nonzero
/// value made it incorrect. In such a case, it is better to leave all
/// elements as zero, and assign nonzero values only as they emerge.
/// Such members are indicated with the string "DYNAMIC".

void LedgerInvariant::Init(BasicValues const* b)
{
    // Zero-initialize almost everything.
    Init();

    InforceLives = b->partial_mortality_lx();

    irr_precision_ = b->round_irr().decimals();

    // BOY vectors.

//  GrossPmt                   = DYNAMIC
//  EeGrossPmt                 = DYNAMIC
//  ErGrossPmt                 = DYNAMIC
//  NetWD                      = DYNAMIC
//  NewCashLoan                = DYNAMIC
//  Outlay                     = DYNAMIC
//  GptForceout                = DYNAMIC
//  NaarForceout               = DYNAMIC Not yet implemented.
//  ModalMinimumPremium        = DYNAMIC
//  EeModalMinimumPremium      = DYNAMIC
//  ErModalMinimumPremium      = DYNAMIC

    AddonMonthlyFee            = b->yare_input_.ExtraMonthlyCustodialFee  ;

    // EOY vectors.

    HasSupplSpecAmt            = false;
    if(b->yare_input_.TermRider)
        {
        TermSpecAmt            .assign(Length, b->yare_input_.TermRiderAmount);
        }
    else if(b->database().query<bool>(DB_TermIsNotRider))
        {
        TermSpecAmt            = b->DeathBfts_->supplamt();
        if(!each_equal(TermSpecAmt, 0.0))
            {
            HasSupplSpecAmt    = true;
            }
        }
    else
        {
        TermSpecAmt            .assign(Length, 0.0);
        }
    SpecAmt                    = b->DeathBfts_->specamt();

    // Forborne vectors.

    Salary                     = b->yare_input_.ProjectedSalary           ;

    // Nonscalable vectors.

    IndvTaxBracket             = b->yare_input_.TaxBracket                ;
    CorpTaxBracket             = b->yare_input_.CorporationTaxBracket     ;
    AnnualFlatExtra            = b->yare_input_.FlatExtra                 ;
    HoneymoonValueSpread       = b->yare_input_.HoneymoonValueSpread      ;
    PartMortTableMult          = b->yare_input_.PartialMortalityMultiplier;
    AddonCompOnAssets          = b->yare_input_.ExtraCompensationOnAssets ;
    AddonCompOnPremium         = b->yare_input_.ExtraCompensationOnPremium;
    CorridorFactor             = b->GetCorridorFactor();

    AnnLoanDueRate = b->InterestRates_->RegLnDueRate
        (mce_gen_curr
        ,mce_annual_rate
        );

    CurrMandE                  = b->InterestRates_->MAndERate(mce_gen_curr);
    TotalIMF                   = b->InterestRates_->InvestmentManagementFee();
    RefundableSalesLoad        = b->Loads_->refundable_sales_load_proportion();

    // Scalable scalars.

    // SOMEDAY !! Things indexed with '[0]' should probably use inforce year instead.
    InitBaseSpecAmt            = b->DeathBfts_->specamt()[0];
    InitTermSpecAmt            = TermSpecAmt[0];
    ChildRiderAmount           = b->yare_input_.ChildRiderAmount;
    SpouseRiderAmount          = b->yare_input_.SpouseRiderAmount;

//  InitPrem                   = DYNAMIC
//  GuarPrem                   = DYNAMIC
//  InitSevenPayPrem           = DYNAMIC
//  InitGSP                    = DYNAMIC
//  InitGLP                    = DYNAMIC
//  InitTgtPrem                = DYNAMIC
//  ListBillPremium            = DYNAMIC
//  EeListBillPremium          = DYNAMIC
//  ErListBillPremium          = DYNAMIC
    // These may need to change, to respect guideline limits:
//  Dumpin                     = DYNAMIC
//  External1035Amount         = DYNAMIC
//  Internal1035Amount         = DYNAMIC

    InforceTotalAV =
          b->yare_input_.InforceGeneralAccountValue
        + b->yare_input_.InforceRegularLoanValue
        + b->yare_input_.InforcePreferredLoanValue
        + b->yare_input_.InforceSeparateAccountValue
        ;
    InforceTaxBasis            = b->yare_input_.InforceTaxBasis           ;

    // Nonscalable scalars.

    MaleProportion             = b->yare_input_.MaleProportion;
    NonsmokerProportion        = b->yare_input_.NonsmokerProportion;

    // Assert this because the illustration currently prints a scalar
    // guaranteed max, assuming that it's the same for all years.
    std::vector<double> const& guar_m_and_e_rate = b->InterestRates_->MAndERate
        (mce_gen_guar
        );
    LMI_ASSERT(each_equal(guar_m_and_e_rate, guar_m_and_e_rate.front()));
    GuarMaxMandE               = guar_m_and_e_rate[0];
    InitDacTaxRate             = b->Loads_->dac_tax_load()[b->yare_input_.InforceYear];
    InitPremTaxRate            = b->PremiumTax_->maximum_load_rate();
    GenderBlended              = b->yare_input_.BlendGender;
    SmokerBlended              = b->yare_input_.BlendSmoking;

    Age                        = b->yare_input_.IssueAge;
    RetAge                     = b->yare_input_.RetirementAge;
    EndtAge                    = b->yare_input_.IssueAge + b->GetLength();
    b->database().query_into(DB_GroupIndivSelection, GroupIndivSelection);
    NoLongerIssued             = b->database().query<bool>(DB_NoLongerIssued);
    AllowGroupQuote            = b->database().query<bool>(DB_AllowGroupQuote);
    b->database().query_into(DB_TxCallsGuarUwSubstd, TxCallsGuarUwSubstd);
    AllowExperienceRating      = b->database().query<bool>(DB_AllowExpRating);
    UseExperienceRating        = b->yare_input_.UseExperienceRating;
    UsePartialMort             = b->yare_input_.UsePartialMortality;

    SurviveToExpectancy        = false;
    SurviveToYear              = false;
    SurviveToAge               = false;
    switch(b->yare_input_.SurviveToType)
        {
        case mce_no_survival_limit:     /* do nothing */   ; break;
        case mce_survive_to_age:        SurviveToAge        = true; break;
        case mce_survive_to_year:       SurviveToYear       = true; break;
        case mce_survive_to_expectancy: SurviveToExpectancy = true; break;
        }
    LMI_ASSERT(SurviveToExpectancy + SurviveToYear + SurviveToAge <= 1);

    // This isn't constrained to the cell-specific maturity duration.
    // If a composite has two cells, ages 20 and 80, and the input max
    // duration for the case (and both cells) is 25 years, then the
    // composite max duration really is 25: it's not limited to 20
    // because the 80-year-old matures earlier.
    SurvivalMaxYear            = b->yare_input_.SurviveToYear;
    // However, this one is so constrained:
    SurvivalMaxAge             = b->yare_input_.SurviveToAge;
    SurvivalMaxAge             = std::min(EndtAge, SurvivalMaxAge);

    AvgFund                    = b->yare_input_.UseAverageOfAllFunds;
    CustomFund                 = b->yare_input_.OverrideFundManagementFee;

//  IsMec                      = DYNAMIC
    // INPUT !! This should depend only on 'yare_input_.InforceIsMec',
    // but see its assignment in 'ihs_acctval.cpp'.
//  InforceIsMec               = DYNAMIC
    InforceYear                = b->yare_input_.InforceYear;
    InforceMonth               = b->yare_input_.InforceMonth;
//  MecYear                    = DYNAMIC
//  MecMonth                   = DYNAMIC

    HasWP                      = b->yare_input_.WaiverOfPremiumBenefit;
    HasADD                     = b->yare_input_.AccidentalDeathBenefit;
    HasTerm                    = b->yare_input_.TermRider;
//  HasSupplSpecAmt // Already assigned above.
    HasChildRider              = b->yare_input_.ChildRider;
    HasSpouseRider             = b->yare_input_.SpouseRider;
    SpouseIssueAge             = b->yare_input_.SpouseIssueAge;

    HasHoneymoon               = b->yare_input_.HoneymoonEndorsement;
    PostHoneymoonSpread        = b->yare_input_.PostHoneymoonSpread;
    b->database().query_into(DB_SplitMinPrem        , SplitMinPrem);
    b->database().query_into(DB_ErNotionallyPaysTerm, ErNotionallyPaysTerm);
    b->database().query_into(DB_IsSinglePremium     , IsSinglePremium);

    std::vector<double> z      = b->InterestRates_->RegLoanSpread(mce_gen_guar);
    LMI_ASSERT(!z.empty());              // Ensure *(std::max_element()) works.
    MaxAnnGuarLoanSpread       = *std::max_element(z.begin(), z.end());
    LMI_ASSERT(!AnnLoanDueRate.empty()); // Ensure *(std::max_element()) works.
    MaxAnnCurrLoanDueRate      = *std::max_element
        (AnnLoanDueRate.begin()
        ,AnnLoanDueRate.end()
        );

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

    std::vector<double> coimult;
    b->database().query_into(DB_CurrCoiMultiplier, coimult);
    CurrentCoiMultiplier =
          coimult                            [b->yare_input_.InforceYear]
        * b->yare_input_.CurrentCoiMultiplier[b->yare_input_.InforceYear]
        * b->yare_input_.CountryCoiMultiplier
        ;

    b->database().query_into(DB_NoLapseAlwaysActive, NoLapseAlwaysActive);
    b->database().query_into(DB_NoLapseMinDur      , NoLapseMinDur);
    b->database().query_into(DB_NoLapseMinAge      , NoLapseMinAge);
    b->database().query_into(DB_Has1035ExchCharge  , Has1035ExchCharge);

    EffDateJdn            = calendar_date(b->yare_input_.EffectiveDate     ).julian_day_number();
    DateOfBirthJdn        = calendar_date(b->yare_input_.DateOfBirth       ).julian_day_number();
    LastCoiReentryDateJdn = calendar_date(b->yare_input_.LastCoiReentryDate).julian_day_number();
    ListBillDateJdn       = calendar_date(b->yare_input_.ListBillDate      ).julian_day_number();
    InforceAsOfDateJdn    = calendar_date(b->yare_input_.InforceAsOfDate   ).julian_day_number();

    GenAcctAllocation          = 1.0 - premium_allocation_to_sepacct(b->yare_input_);

    SplitFundAllocation =
            (0.0 != GenAcctAllocation && 1.0 != GenAcctAllocation)
        ||
            (  0.0 != b->yare_input_.InforceGeneralAccountValue
            && 0.0 != b->yare_input_.InforceSeparateAccountValue
            )
        ;

    WriteTsvFile               = contains(b->yare_input_.Comments, "idiosyncrasyY");

    SupplementalReport         = b->yare_input_.CreateSupplementalReport;

    // These are reassigned below based on product data if available.
    std::string dbo_name_option1 = mc_str(mce_option1);
    std::string dbo_name_option2 = mc_str(mce_option2);
    std::string dbo_name_rop     = mc_str(mce_rop    );
    std::string dbo_name_mdb     = mc_str(mce_mdb    );

    // The antediluvian branch doesn't meaningfully initialize class product_data.
    if(!is_antediluvian_fork())
        {
        product_data const& p = b->product();
        // Accommodate one alternative policy-form name. // LINGO !! expunge this block:
        // DATABASE !! It would be much better, of course, to let all
        // strings in class product_data vary across the same axes as
        // database_entity objects.
        bool alt_form = b->database().query<bool>(DB_UsePolicyFormAlt);
        dbo_name_option1           = p.datum("DboNameLevel"                   );
        dbo_name_option2           = p.datum("DboNameIncreasing"              );
        dbo_name_rop               = p.datum("DboNameReturnOfPremium"         );
        dbo_name_mdb               = p.datum("DboNameMinDeathBenefit"         );

        // Strings.

        PolicyForm = p.datum(alt_form ? "PolicyFormAlternative" : "PolicyForm"); // LINGO !! expunge old implementation here

        auto policy_form = b->database().query<int>(DB_PolicyForm);
        bool const policy_form_is_okay =
               b->lingo_->lookup(policy_form) == PolicyForm
            || "{PolicyFormAlternative}" == PolicyForm
            ;
        if(!policy_form_is_okay)
            alarum()
                << b->lingo_->lookup(policy_form) << " b->lingo_->lookup(policy_form)\n"
                << PolicyForm << " PolicyForm\n"
                << LMI_FLUSH
                ;
        PolicyForm = b->lingo_->lookup(policy_form);

        PolicyMktgName             = p.datum("PolicyMktgName"                 );
        PolicyLegalName            = p.datum("PolicyLegalName"                );
        CsoEra     = mc_str(b->database().query<mcenum_cso_era>(DB_CsoEra));
        InsCoShortName             = p.datum("InsCoShortName"                 );
        InsCoName                  = p.datum("InsCoName"                      );
        InsCoAddr                  = p.datum("InsCoAddr"                      );
        InsCoStreet                = p.datum("InsCoStreet"                    );
        InsCoPhone                 = p.datum("InsCoPhone"                     );
        MainUnderwriter            = p.datum("MainUnderwriter"                );
        MainUnderwriterAddress     = p.datum("MainUnderwriterAddress"         );
        CoUnderwriter              = p.datum("CoUnderwriter"                  );
        CoUnderwriterAddress       = p.datum("CoUnderwriterAddress"           );

        // Terms defined in the contract

        AvName                     = p.datum("AvName"                         );
        CsvName                    = p.datum("CsvName"                        );
        CsvHeaderName              = p.datum("CsvHeaderName"                  );
        NoLapseProvisionName       = p.datum("NoLapseProvisionName"           );
        ContractName               = p.datum("ContractName"                   );
        DboName                    = p.datum("DboName"                        );
        // PDF !! It is hoped that these three local variables (which
        // duplicate 'dbo_name_option1' etc. above) can be expunged.
        DboNameLevel               = p.datum("DboNameLevel"                   );
        DboNameIncreasing          = p.datum("DboNameIncreasing"              );
        DboNameMinDeathBenefit     = p.datum("DboNameMinDeathBenefit"         );
        GenAcctName                = p.datum("GenAcctName"                    );
        GenAcctNameElaborated      = p.datum("GenAcctNameElaborated"          );
        SepAcctName                = p.datum("SepAcctName"                    );
        SpecAmtName                = p.datum("SpecAmtName"                    );
        SpecAmtNameElaborated      = p.datum("SpecAmtNameElaborated"          );
        UwBasisMedical             = p.datum("UwBasisMedical"                 );
        UwBasisParamedical         = p.datum("UwBasisParamedical"             );
        UwBasisNonmedical          = p.datum("UwBasisNonmedical"              );
        UwBasisSimplified          = p.datum("UwBasisSimplified"              );
        UwBasisGuaranteed          = p.datum("UwBasisGuaranteed"              );
        UwClassPreferred           = p.datum("UwClassPreferred"               );
        UwClassStandard            = p.datum("UwClassStandard"                );
        UwClassRated               = p.datum("UwClassRated"                   );
        UwClassUltra               = p.datum("UwClassUltra"                   );

        // Ledger column definitions.

        AccountValueFootnote       = p.datum("AccountValueFootnote"           );
        AttainedAgeFootnote        = p.datum("AttainedAgeFootnote"            );
        CashSurrValueFootnote      = p.datum("CashSurrValueFootnote"          );
        DeathBenefitFootnote       = p.datum("DeathBenefitFootnote"           );
        InitialPremiumFootnote     = p.datum("InitialPremiumFootnote"         );
        NetPremiumFootnote         = p.datum("NetPremiumFootnote"             );
        GrossPremiumFootnote       = p.datum("GrossPremiumFootnote"           );
        OutlayFootnote             = p.datum("OutlayFootnote"                 );
        PolicyYearFootnote         = p.datum("PolicyYearFootnote"             );

        // Terse rider names.

        ADDTerseName               = p.datum("ADDTerseName"                   );
        InsurabilityTerseName      = p.datum("InsurabilityTerseName"          );
        ChildTerseName             = p.datum("ChildTerseName"                 );
        SpouseTerseName            = p.datum("SpouseTerseName"                );
        TermTerseName              = p.datum("TermTerseName"                  );
        WaiverTerseName            = p.datum("WaiverTerseName"                );
        AccelBftRiderTerseName     = p.datum("AccelBftRiderTerseName"         );
        OverloanRiderTerseName     = p.datum("OverloanRiderTerseName"         );

        // Rider footnotes.

        ADDFootnote                = p.datum("ADDFootnote"                    );
        ChildFootnote              = p.datum("ChildFootnote"                  );
        SpouseFootnote             = p.datum("SpouseFootnote"                 );
        TermFootnote               = p.datum("TermFootnote"                   );
        WaiverFootnote             = p.datum("WaiverFootnote"                 );
        AccelBftRiderFootnote      = p.datum("AccelBftRiderFootnote"          );
        OverloanRiderFootnote      = p.datum("OverloanRiderFootnote"          );

        // Group quote footnotes.

        GroupQuoteShortProductName = p.datum("GroupQuoteShortProductName"     );
        GroupQuoteIsNotAnOffer     = p.datum("GroupQuoteIsNotAnOffer"         );
        GroupQuoteRidersFooter     = p.datum("GroupQuoteRidersFooter"         );
        GroupQuotePolicyFormId     = p.datum("GroupQuotePolicyFormId"         );
        GroupQuoteStateVariations  = p.datum("GroupQuoteStateVariations"      );
        GroupQuoteProspectus       = p.datum("GroupQuoteProspectus"           );
        GroupQuoteUnderwriter      = p.datum("GroupQuoteUnderwriter"          );
        GroupQuoteBrokerDealer     = p.datum("GroupQuoteBrokerDealer"         );
        GroupQuoteRubricMandatory  = p.datum("GroupQuoteRubricMandatory"      );
        GroupQuoteRubricVoluntary  = p.datum("GroupQuoteRubricVoluntary"      );
        GroupQuoteRubricFusion     = p.datum("GroupQuoteRubricFusion"         );
        GroupQuoteFooterMandatory  = p.datum("GroupQuoteFooterMandatory"      );
        GroupQuoteFooterVoluntary  = p.datum("GroupQuoteFooterVoluntary"      );
        GroupQuoteFooterFusion     = p.datum("GroupQuoteFooterFusion"         );

        // Premium-specific footnotes.

        MinimumPremiumFootnote     = p.datum("MinimumPremiumFootnote"         );
        PremAllocationFootnote     = p.datum("PremAllocationFootnote"         );

        // Miscellaneous other footnotes.

        InterestDisclaimer         = p.datum("InterestDisclaimer"             );
        GuarMortalityFootnote      = p.datum("GuarMortalityFootnote"          );
        ProductDescription         = p.datum("ProductDescription"             );
        StableValueFootnote        = p.datum("StableValueFootnote"            );
        NoVanishPremiumFootnote    = p.datum("NoVanishPremiumFootnote"        );
        RejectPremiumFootnote      = p.datum("RejectPremiumFootnote"          );
        ExpRatingFootnote          = p.datum("ExpRatingFootnote"              );
        MortalityBlendFootnote     = p.datum("MortalityBlendFootnote"         );
        HypotheticalRatesFootnote  = p.datum("HypotheticalRatesFootnote"      );
        SalesLoadRefundFootnote    = p.datum("SalesLoadRefundFootnote"        );
        NoLapseEverFootnote        = p.datum("NoLapseEverFootnote"            );
        NoLapseFootnote            = p.datum("NoLapseFootnote"                );
        CurrentValuesFootnote      = p.datum("CurrentValuesFootnote"          );
        DBOption1Footnote          = p.datum("DBOption1Footnote"              );
        DBOption2Footnote          = p.datum("DBOption2Footnote"              );
        DBOption3Footnote          = p.datum("DBOption3Footnote"              );
        MinDeathBenefitFootnote    = p.datum("MinDeathBenefitFootnote"        );
        ExpRatRiskChargeFootnote   = p.datum("ExpRatRiskChargeFootnote"       );
        ExchangeChargeFootnote1    = p.datum("ExchangeChargeFootnote1"        );
        FlexiblePremiumFootnote    = p.datum("FlexiblePremiumFootnote"        );
        GuaranteedValuesFootnote   = p.datum("GuaranteedValuesFootnote"       );
        CreditingRateFootnote      = p.datum("CreditingRateFootnote"          );
        GrossRateFootnote          = p.datum("GrossRateFootnote"              );
        NetRateFootnote            = p.datum("NetRateFootnote"                );
        MecFootnote                = p.datum("MecFootnote"                    );
        GptFootnote                = p.datum("GptFootnote"                    );
        MidpointValuesFootnote     = p.datum("MidpointValuesFootnote"         );
        SinglePremiumFootnote      = p.datum("SinglePremiumFootnote"          );
        MonthlyChargesFootnote     = p.datum("MonthlyChargesFootnote"         );
        UltCreditingRateFootnote   = p.datum("UltCreditingRateFootnote"       );
        UltCreditingRateHeader     = p.datum("UltCreditingRateHeader"         );
        MaxNaarFootnote            = p.datum("MaxNaarFootnote"                );
        PremTaxSurrChgFootnote     = p.datum("PremTaxSurrChgFootnote"         );
        PolicyFeeFootnote          = p.datum("PolicyFeeFootnote"              );
        AssetChargeFootnote        = p.datum("AssetChargeFootnote"            );
        InvestmentIncomeFootnote   = p.datum("InvestmentIncomeFootnote"       );
        IrrDbFootnote              = p.datum("IrrDbFootnote"                  );
        IrrCsvFootnote             = p.datum("IrrCsvFootnote"                 );
        MortalityChargesFootnote   = p.datum("MortalityChargesFootnote"       );
        LoanAndWithdrawalFootnote  = p.datum("LoanAndWithdrawalFootnote"      );
        LoanFootnote               = p.datum("LoanFootnote"                   );
        ImprimaturPresale          = p.datum("ImprimaturPresale"              );
        ImprimaturPresaleComposite = p.datum("ImprimaturPresaleComposite"     );
        ImprimaturInforce          = p.datum("ImprimaturInforce"              );
        ImprimaturInforceComposite = p.datum("ImprimaturInforceComposite"     );
        StateMarketingImprimatur   = p.datum("StateMarketingImprimatur"       );
        NonGuaranteedFootnote      = p.datum("NonGuaranteedFootnote"          );
        NonGuaranteedFootnote1     = p.datum("NonGuaranteedFootnote1"         );
        NonGuaranteedFootnote1Tx   = p.datum("NonGuaranteedFootnote1Tx"       );
        FnMonthlyDeductions        = p.datum("FnMonthlyDeductions"            );
        SurrenderFootnote          = p.datum("SurrenderFootnote"              );
        PortabilityFootnote        = p.datum("PortabilityFootnote"            );
        FundRateFootnote           = p.datum("FundRateFootnote"               );
        IssuingCompanyFootnote     = p.datum("IssuingCompanyFootnote"         );
        SubsidiaryFootnote         = p.datum("SubsidiaryFootnote"             );
        PlacementAgentFootnote     = p.datum("PlacementAgentFootnote"         );
        MarketingNameFootnote      = p.datum("MarketingNameFootnote"          );
        GuarIssueDisclaimerNcSc    = p.datum("GuarIssueDisclaimerNcSc"        );
        GuarIssueDisclaimerMd      = p.datum("GuarIssueDisclaimerMd"          );
        GuarIssueDisclaimerTx      = p.datum("GuarIssueDisclaimerTx"          );
        IllRegCertAgent            = p.datum("IllRegCertAgent"                );
        IllRegCertAgentIl          = p.datum("IllRegCertAgentIl"              );
        IllRegCertAgentTx          = p.datum("IllRegCertAgentTx"              );
        IllRegCertClient           = p.datum("IllRegCertClient"               );
        IllRegCertClientIl         = p.datum("IllRegCertClientIl"             );
        IllRegCertClientTx         = p.datum("IllRegCertClientTx"             );
        FnMaturityAge              = p.datum("FnMaturityAge"                  );
        FnPartialMortality         = p.datum("FnPartialMortality"             );
        FnProspectus               = p.datum("FnProspectus"                   );
        FnInitialSpecAmt           = p.datum("FnInitialSpecAmt"               );
        FnInforceAcctVal           = p.datum("FnInforceAcctVal"               );
        FnInforceTaxBasis          = p.datum("FnInforceTaxBasis"              );
        Fn1035Charge               = p.datum("Fn1035Charge"                   );
        FnMecExtraWarning          = p.datum("FnMecExtraWarning"              );
        FnNotTaxAdvice             = p.datum("FnNotTaxAdvice"                 );
        FnNotTaxAdvice2            = p.datum("FnNotTaxAdvice2"                );
        FnImf                      = p.datum("FnImf"                          );
        FnCensus                   = p.datum("FnCensus"                       );
        FnDacTax                   = p.datum("FnDacTax"                       );
        FnDefnLifeIns              = p.datum("FnDefnLifeIns"                  );
        FnBoyEoy                   = p.datum("FnBoyEoy"                       );
        FnGeneralAccount           = p.datum("FnGeneralAccount"               );
        FnPpMemorandum             = p.datum("FnPpMemorandum"                 );
        FnPpAccreditedInvestor     = p.datum("FnPpAccreditedInvestor"         );
        FnPpLoads                  = p.datum("FnPpLoads"                      );
        FnProposalUnderwriting     = p.datum("FnProposalUnderwriting"         );
        FnGuaranteedPremium        = p.datum("FnGuaranteedPremium"            );
        FnOmnibusDisclaimer        = p.datum("FnOmnibusDisclaimer"            );
        FnInitialDbo               = p.datum("FnInitialDbo"                   );
        DefnGuarGenAcctRate        = p.datum("DefnGuarGenAcctRate"            );
        DefnAV                     = p.datum("DefnAV"                         );
        DefnCSV                    = p.datum("DefnCSV"                        );
        DefnMec                    = p.datum("DefnMec"                        );
        DefnOutlay                 = p.datum("DefnOutlay"                     );
        DefnSpecAmt                = p.datum("DefnSpecAmt"                    );
        }

    // Strings from class Input.

    ProductName                = b->yare_input_.ProductName;
    ProducerName               = b->yare_input_.AgentName;

    std::string const agent_city     = b->yare_input_.AgentCity;
    std::string const agent_state    = mc_str(b->yare_input_.AgentState);
    std::string const agent_zip_code = b->yare_input_.AgentZipCode;
    // This is a two-letter USPS abbreviation, so it's never empty.
    std::string agent_city_etc(agent_state);
    if(!agent_city.empty())
        {
        agent_city_etc = agent_city + ", " + agent_state;
        }
    if(!agent_zip_code.empty())
        {
        agent_city_etc += " " + agent_zip_code;
        }

    ProducerStreet             = b->yare_input_.AgentAddress;
    ProducerCityEtc            = agent_city_etc;
    ProducerPhone              = b->yare_input_.AgentPhone;
    ProducerId                 = b->yare_input_.AgentId;

    CorpName                   = b->yare_input_.CorporationName;

    MasterContractNumber       = b->yare_input_.MasterContractNumber;
    ContractNumber             = b->yare_input_.ContractNumber;

    Insured1                   = b->yare_input_.InsuredName;
    Gender                     = mc_str(b->yare_input_.Gender);
    UWType                     = mc_str(b->yare_input_.GroupUnderwritingType);

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

    UWClass                    = mc_str(b->yare_input_.UnderwritingClass);
    SubstandardTable           = mc_str(b->yare_input_.SubstandardTable);

    DefnLifeIns                = mc_str(b->yare_input_.DefinitionOfLifeInsurance);
    DefnMaterialChange         = mc_str(b->yare_input_.DefinitionOfMaterialChange);
    PartMortTableName          = "1983 GAM"; // TODO ?? Hardcoded.
    StateOfJurisdiction        = mc_str(b->GetStateOfJurisdiction());
    PremiumTaxState            = mc_str(b->GetPremiumTaxState());
    CountryIso3166Abbrev       = mc_str(b->yare_input_.Country);
    Comments                   = b->yare_input_.Comments;

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

    mcenum_dbopt const init_dbo = b->DeathBfts_->dbopt()[0];
    InitDBOpt =
         (mce_option1 == init_dbo) ? dbo_name_option1
        :(mce_option2 == init_dbo) ? dbo_name_option2
        :(mce_rop     == init_dbo) ? dbo_name_rop
        :(mce_mdb     == init_dbo) ? dbo_name_mdb
        :throw std::logic_error("Unrecognized initial death benefit option.")
        ;
    InitEeMode                 = mc_str(b->Outlay_->ee_premium_modes()[0]);
    InitErMode                 = mc_str(b->Outlay_->er_premium_modes()[0]);

    // Special-case vectors.

    for(int j = 0; j < Length; ++j)
        {
        DBOpt [j] = b->DeathBfts_->dbopt()[j];
        EeMode[j] = b->Outlay_->ee_premium_modes()[j];
        ErMode[j] = b->Outlay_->er_premium_modes()[j];
        }

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

    // Special-case strings.

    EffDate                    = calendar_date(b->yare_input_.EffectiveDate     ).str();
    DateOfBirth                = calendar_date(b->yare_input_.DateOfBirth       ).str();
    LastCoiReentryDate         = calendar_date(b->yare_input_.LastCoiReentryDate).str();
    ListBillDate               = calendar_date(b->yare_input_.ListBillDate      ).str();
    InforceAsOfDate            = calendar_date(b->yare_input_.InforceAsOfDate   ).str();

    // irr_initialized_ is deliberately not set here: it's not
    // encompassed by 'FullyInitialized'.
    FullyInitialized = true;
}

/// TODO ?? Temporary kludge.
///
/// Objects of this class should be used only to store final values
/// that result from monthiversary processing, and to use those values
/// to generate reports. Therefore, they should need to be initialized
/// only once. However, they've been (ab)used to store intermediate
/// values during monthiversary processing, and thus, defectively,
/// some data members need to be reinitialized when calculations are
/// to be performed again on a different basis.
///
/// Complete reinitialization is costly, so this function does it only
/// for data members that actually require it. Each such member
/// represents a defect. When the last such defect is resolved, this
/// function will be empty and can itself be removed, along with the
/// following commentary transplanted from the call site:
///
/// // Call Init() here. The solve routines can change parameters in
/// // class BasicValues or objects it contains, parameters which
/// // determine ledger values that are used by the solve routines.
/// // It might be more appropriate to treat such parameters instead
/// // as local state of class AccountValue itself, or of a contained
/// // class smaller than the ledger hierarchy--which we need anyway
/// // for 7702 and 7702A. Or perhaps the solve functions should
/// // manipulate the state of just those elements of the ledgers
/// // that it needs to, to avoid the massive overhead of
/// // unconditionally reinitializing all elements.

void LedgerInvariant::ReInit(BasicValues const* b)
{
    HasSupplSpecAmt            = false;
    if(b->yare_input_.TermRider)
        {
        TermSpecAmt            .assign(Length, b->yare_input_.TermRiderAmount);
        }
    else if(b->database().query<bool>(DB_TermIsNotRider))
        {
        TermSpecAmt            = b->DeathBfts_->supplamt();
        if(!each_equal(TermSpecAmt, 0.0))
            {
            HasSupplSpecAmt    = true;
            }
        }
    else
        {
        TermSpecAmt            .assign(Length, 0.0);
        }
    SpecAmt                    = b->DeathBfts_->specamt();

    InitBaseSpecAmt            = b->DeathBfts_->specamt()[0];
    InitTermSpecAmt            = TermSpecAmt[0];

    IsMec                      = false;
    MecMonth                   = 11;
    MecYear                    = Length;

    ModalMinimumPremium        .assign(Length, 0.0);
    EeModalMinimumPremium      .assign(Length, 0.0);
    ErModalMinimumPremium      .assign(Length, 0.0);
}
