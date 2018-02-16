// Ledger data that do not vary by basis.
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
#include "crc32.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "financial.hpp"                // for CalculateIrrs()
#include "fund_data.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "ledger.hpp"                   // for CalculateIrrs()
#include "ledger_variant.hpp"           // for CalculateIrrs()
#include "loads.hpp"
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "miscellany.hpp"               // each_equal()
#include "outlay.hpp"
#include "premium_tax.hpp"
#include "product_data.hpp"

#include <algorithm>
#include <ostream>
#include <stdexcept>

//============================================================================
LedgerInvariant::LedgerInvariant(int len)
    :LedgerBase(len)
    ,irr_initialized_(false)
    ,FullyInitialized(false)
{
    Alloc(len);
}

//============================================================================
LedgerInvariant::LedgerInvariant(LedgerInvariant const& obj)
    :LedgerBase(obj)
    ,irr_initialized_(false)
    ,FullyInitialized(false)
{
    Alloc(obj.GetLength());
    Copy(obj);
}

//============================================================================
LedgerInvariant& LedgerInvariant::operator=(LedgerInvariant const& obj)
{
    if(this != &obj)
        {
        LedgerBase::operator=(obj);
        Destroy();
        Alloc(obj.Length);
        Copy(obj);
        }
    return *this;
}

//============================================================================
LedgerInvariant::~LedgerInvariant()
{
    Destroy();
}

//============================================================================
void LedgerInvariant::Alloc(int len)
{
    Length  = len;

    BegYearVectors  ["EePmt"                 ] = &EePmt                  ;
    BegYearVectors  ["ErPmt"                 ] = &ErPmt                  ;
    BegYearVectors  ["TgtPrem"               ] = &TgtPrem                ;
    BegYearVectors  ["GrossPmt"              ] = &GrossPmt               ;
    BegYearVectors  ["EeGrossPmt"            ] = &EeGrossPmt             ;
    BegYearVectors  ["ErGrossPmt"            ] = &ErGrossPmt             ;
    BegYearVectors  ["NetWD"                 ] = &NetWD                  ;
    BegYearVectors  ["NewCashLoan"           ] = &NewCashLoan            ;
    BegYearVectors  ["Outlay"                ] = &Outlay                 ;
    BegYearVectors  ["GptForceout"           ] = &GptForceout            ;
    BegYearVectors  ["NaarForceout"          ] = &NaarForceout           ;
    BegYearVectors  ["ModalMinimumPremium"   ] = &ModalMinimumPremium    ;
    BegYearVectors  ["EeModalMinimumPremium" ] = &EeModalMinimumPremium  ;
    BegYearVectors  ["ErModalMinimumPremium" ] = &ErModalMinimumPremium  ;

    EndYearVectors  ["TermSpecAmt"           ] = &TermSpecAmt            ;
    EndYearVectors  ["SpecAmt"               ] = &SpecAmt                ;
    EndYearVectors  ["ProducerCompensation"  ] = &ProducerCompensation   ;

    OtherVectors    ["IndvTaxBracket"        ] = &IndvTaxBracket         ;
    OtherVectors    ["CorpTaxBracket"        ] = &CorpTaxBracket         ;
    OtherVectors    ["Salary"                ] = &Salary                 ;
    OtherVectors    ["AnnualFlatExtra"       ] = &AnnualFlatExtra        ;
    OtherVectors    ["HoneymoonValueSpread"  ] = &HoneymoonValueSpread   ;
    OtherVectors    ["PartMortTableMult"     ] = &PartMortTableMult      ;
    OtherVectors    ["AddonMonthlyFee"       ] = &AddonMonthlyFee        ;
    OtherVectors    ["AddonCompOnAssets"     ] = &AddonCompOnAssets      ;
    OtherVectors    ["AddonCompOnPremium"    ] = &AddonCompOnPremium     ;
    OtherVectors    ["CorridorFactor"        ] = &CorridorFactor         ;
    OtherVectors    ["CurrMandE"             ] = &CurrMandE              ;
    OtherVectors    ["TotalIMF"              ] = &TotalIMF               ;
    OtherVectors    ["RefundableSalesLoad"   ] = &RefundableSalesLoad    ;

    ScalableScalars ["InitBaseSpecAmt"       ] = &InitBaseSpecAmt        ;
    ScalableScalars ["InitTermSpecAmt"       ] = &InitTermSpecAmt        ;
    ScalableScalars ["ChildRiderAmount"      ] = &ChildRiderAmount       ;
    ScalableScalars ["SpouseRiderAmount"     ] = &SpouseRiderAmount      ;
    ScalableScalars ["InitPrem"              ] = &InitPrem               ;
    ScalableScalars ["GuarPrem"              ] = &GuarPrem               ;
    ScalableScalars ["InitSevenPayPrem"      ] = &InitSevenPayPrem       ;
    ScalableScalars ["InitGSP"               ] = &InitGSP                ;
    ScalableScalars ["InitGLP"               ] = &InitGLP                ;
    ScalableScalars ["InitTgtPrem"           ] = &InitTgtPrem            ;
    ScalableScalars ["InitMinPrem"           ] = &InitMinPrem            ;
    ScalableScalars ["ListBillPremium"       ] = &ListBillPremium        ;
    ScalableScalars ["EeListBillPremium"     ] = &EeListBillPremium      ;
    ScalableScalars ["ErListBillPremium"     ] = &ErListBillPremium      ;
    ScalableScalars ["ModalMinimumDumpin"    ] = &ModalMinimumDumpin     ;
    ScalableScalars ["Dumpin"                ] = &Dumpin                 ;
    ScalableScalars ["External1035Amount"    ] = &External1035Amount     ;
    ScalableScalars ["Internal1035Amount"    ] = &Internal1035Amount     ;
    ScalableScalars ["InforceUnloanedAV"     ] = &InforceUnloanedAV      ;
    ScalableScalars ["InforceTaxBasis"       ] = &InforceTaxBasis        ;

    OtherScalars    ["MaleProportion"        ] = &MaleProportion         ;
    OtherScalars    ["NonsmokerProportion"   ] = &NonsmokerProportion    ;
    OtherScalars    ["GuarMaxMandE"          ] = &GuarMaxMandE           ;
    OtherScalars    ["InitDacTaxRate"        ] = &InitDacTaxRate         ;
    OtherScalars    ["InitPremTaxRate"       ] = &InitPremTaxRate        ;
    OtherScalars    ["GenderDistinct"        ] = &GenderDistinct         ;
    OtherScalars    ["GenderBlended"         ] = &GenderBlended          ;
    OtherScalars    ["SmokerDistinct"        ] = &SmokerDistinct         ;
    OtherScalars    ["SmokerBlended"         ] = &SmokerBlended          ;
    OtherScalars    ["SubstdTable"           ] = &SubstdTable            ;
    OtherScalars    ["Age"                   ] = &Age                    ;
    OtherScalars    ["RetAge"                ] = &RetAge                 ;
    OtherScalars    ["EndtAge"               ] = &EndtAge                ;
    OtherScalars    ["GroupIndivSelection"   ] = &GroupIndivSelection    ;
    OtherScalars    ["UseExperienceRating"   ] = &UseExperienceRating    ;
    OtherScalars    ["UsePartialMort"        ] = &UsePartialMort         ;
    OtherScalars    ["AvgFund"               ] = &AvgFund                ;
    OtherScalars    ["CustomFund"            ] = &CustomFund             ;
    OtherScalars    ["IsMec"                 ] = &IsMec                  ;
    OtherScalars    ["InforceIsMec"          ] = &InforceIsMec           ;
    OtherScalars    ["InforceYear"           ] = &InforceYear            ;
    OtherScalars    ["InforceMonth"          ] = &InforceMonth           ;
    OtherScalars    ["MecYear"               ] = &MecYear                ;
    OtherScalars    ["MecMonth"              ] = &MecMonth               ;
    OtherScalars    ["HasWP"                 ] = &HasWP                  ;
    OtherScalars    ["HasADD"                ] = &HasADD                 ;
    OtherScalars    ["HasTerm"               ] = &HasTerm                ;
    OtherScalars    ["HasSupplSpecAmt"       ] = &HasSupplSpecAmt        ;
    OtherScalars    ["HasChildRider"         ] = &HasChildRider          ;
    OtherScalars    ["HasSpouseRider"        ] = &HasSpouseRider         ;
    OtherScalars    ["SpouseIssueAge"        ] = &SpouseIssueAge         ;
    OtherScalars    ["HasHoneymoon"          ] = &HasHoneymoon           ;
    OtherScalars    ["PostHoneymoonSpread"   ] = &PostHoneymoonSpread    ;
    OtherScalars    ["SplitMinPrem"          ] = &SplitMinPrem           ;
    OtherScalars    ["AllowDbo3"             ] = &AllowDbo3              ;
    OtherScalars    ["InitAnnLoanDueRate"    ] = &InitAnnLoanDueRate     ;
    OtherScalars    ["IsInforce"             ] = &IsInforce              ;
    OtherScalars    ["CurrentCoiMultiplier"  ] = &CurrentCoiMultiplier   ;
    OtherScalars    ["NoLapseAlwaysActive"   ] = &NoLapseAlwaysActive    ;
    OtherScalars    ["NoLapseMinDur"         ] = &NoLapseMinDur          ;
    OtherScalars    ["NoLapseMinAge"         ] = &NoLapseMinAge          ;
    OtherScalars    ["Has1035ExchCharge"     ] = &Has1035ExchCharge      ;
    OtherScalars    ["EffDateJdn"            ] = &EffDateJdn             ;
    OtherScalars    ["DateOfBirthJdn"        ] = &DateOfBirthJdn         ;
    OtherScalars    ["ListBillDateJdn"       ] = &ListBillDateJdn        ;
    OtherScalars    ["InforceAsOfDateJdn"    ] = &InforceAsOfDateJdn     ;
    OtherScalars    ["SplitFundAllocation"   ] = &SplitFundAllocation    ;
    OtherScalars    ["GenAcctAllocation"     ] = &GenAcctAllocation      ;
    OtherScalars    ["SupplementalReport"    ] = &SupplementalReport     ;

    Strings["PolicyMktgName"                ] = &PolicyMktgName                ;
    Strings["PolicyLegalName"               ] = &PolicyLegalName               ;
    Strings["PolicyForm"                    ] = &PolicyForm                    ;
    Strings["InsCoShortName"                ] = &InsCoShortName                ;
    Strings["InsCoName"                     ] = &InsCoName                     ;
    Strings["InsCoAddr"                     ] = &InsCoAddr                     ;
    Strings["InsCoStreet"                   ] = &InsCoStreet                   ;
    Strings["InsCoPhone"                    ] = &InsCoPhone                    ;
    Strings["MainUnderwriter"               ] = &MainUnderwriter               ;
    Strings["MainUnderwriterAddress"        ] = &MainUnderwriterAddress        ;
    Strings["CoUnderwriter"                 ] = &CoUnderwriter                 ;
    Strings["CoUnderwriterAddress"          ] = &CoUnderwriterAddress          ;

    Strings["AvName"                        ] = &AvName                        ;
    Strings["CsvName"                       ] = &CsvName                       ;
    Strings["CsvHeaderName"                 ] = &CsvHeaderName                 ;
    Strings["NoLapseProvisionName"          ] = &NoLapseProvisionName          ;
    Strings["ContractName"                  ] = &ContractName                  ;

    Strings["AccountValueFootnote"          ] = &AccountValueFootnote          ;
    Strings["AttainedAgeFootnote"           ] = &AttainedAgeFootnote           ;
    Strings["CashSurrValueFootnote"         ] = &CashSurrValueFootnote         ;
    Strings["DeathBenefitFootnote"          ] = &DeathBenefitFootnote          ;
    Strings["InitialPremiumFootnote"        ] = &InitialPremiumFootnote        ;
    Strings["NetPremiumFootnote"            ] = &NetPremiumFootnote            ;
    Strings["GrossPremiumFootnote"          ] = &GrossPremiumFootnote          ;
    Strings["OutlayFootnote"                ] = &OutlayFootnote                ;
    Strings["PolicyYearFootnote"            ] = &PolicyYearFootnote            ;

    Strings["ADDTerseName"                  ] = &ADDTerseName                  ;
    Strings["InsurabilityTerseName"         ] = &InsurabilityTerseName         ;
    Strings["ChildTerseName"                ] = &ChildTerseName                ;
    Strings["SpouseTerseName"               ] = &SpouseTerseName               ;
    Strings["TermTerseName"                 ] = &TermTerseName                 ;
    Strings["WaiverTerseName"               ] = &WaiverTerseName               ;
    Strings["AccelBftRiderTerseName"        ] = &AccelBftRiderTerseName        ;
    Strings["OverloanRiderTerseName"        ] = &OverloanRiderTerseName        ;

    Strings["ADDFootnote"                   ] = &ADDFootnote                   ;
    Strings["ChildFootnote"                 ] = &ChildFootnote                 ;
    Strings["SpouseFootnote"                ] = &SpouseFootnote                ;
    Strings["TermFootnote"                  ] = &TermFootnote                  ;
    Strings["WaiverFootnote"                ] = &WaiverFootnote                ;
    Strings["AccelBftRiderFootnote"         ] = &AccelBftRiderFootnote         ;
    Strings["OverloanRiderFootnote"         ] = &OverloanRiderFootnote         ;

    Strings["GroupQuoteShortProductName"    ] = &GroupQuoteShortProductName    ;
    Strings["GroupQuoteIsNotAnOffer"        ] = &GroupQuoteIsNotAnOffer        ;
    Strings["GroupQuoteRidersFooter"        ] = &GroupQuoteRidersFooter        ;
    Strings["GroupQuotePolicyFormId"        ] = &GroupQuotePolicyFormId        ;
    Strings["GroupQuoteStateVariations"     ] = &GroupQuoteStateVariations     ;
    Strings["GroupQuoteProspectus"          ] = &GroupQuoteProspectus          ;
    Strings["GroupQuoteUnderwriter"         ] = &GroupQuoteUnderwriter         ;
    Strings["GroupQuoteBrokerDealer"        ] = &GroupQuoteBrokerDealer        ;
    Strings["GroupQuoteRubricMandatory"     ] = &GroupQuoteRubricMandatory     ;
    Strings["GroupQuoteRubricVoluntary"     ] = &GroupQuoteRubricVoluntary     ;
    Strings["GroupQuoteRubricFusion"        ] = &GroupQuoteRubricFusion        ;
    Strings["GroupQuoteFooterMandatory"     ] = &GroupQuoteFooterMandatory     ;
    Strings["GroupQuoteFooterVoluntary"     ] = &GroupQuoteFooterVoluntary     ;
    Strings["GroupQuoteFooterFusion"        ] = &GroupQuoteFooterFusion        ;

    Strings["MinimumPremiumFootnote"        ] = &MinimumPremiumFootnote        ;
    Strings["PremAllocationFootnote"        ] = &PremAllocationFootnote        ;
    Strings["InterestDisclaimer"            ] = &InterestDisclaimer            ;
    Strings["GuarMortalityFootnote"         ] = &GuarMortalityFootnote         ;
    Strings["ProductDescription"            ] = &ProductDescription            ;
    Strings["StableValueFootnote"           ] = &StableValueFootnote           ;
    Strings["NoVanishPremiumFootnote"       ] = &NoVanishPremiumFootnote       ;
    Strings["RejectPremiumFootnote"         ] = &RejectPremiumFootnote         ;
    Strings["ExpRatingFootnote"             ] = &ExpRatingFootnote             ;
    Strings["MortalityBlendFootnote"        ] = &MortalityBlendFootnote        ;
    Strings["HypotheticalRatesFootnote"     ] = &HypotheticalRatesFootnote     ;
    Strings["SalesLoadRefundFootnote"       ] = &SalesLoadRefundFootnote       ;
    Strings["NoLapseFootnote"               ] = &NoLapseFootnote               ;
    Strings["MarketValueAdjFootnote"        ] = &MarketValueAdjFootnote        ;
    Strings["ExchangeChargeFootnote0"       ] = &ExchangeChargeFootnote0       ;
    Strings["CurrentValuesFootnote"         ] = &CurrentValuesFootnote         ;
    Strings["DBOption1Footnote"             ] = &DBOption1Footnote             ;
    Strings["DBOption2Footnote"             ] = &DBOption2Footnote             ;
    Strings["ExpRatRiskChargeFootnote"      ] = &ExpRatRiskChargeFootnote      ;
    Strings["ExchangeChargeFootnote1"       ] = &ExchangeChargeFootnote1       ;
    Strings["FlexiblePremiumFootnote"       ] = &FlexiblePremiumFootnote       ;
    Strings["GuaranteedValuesFootnote"      ] = &GuaranteedValuesFootnote      ;
    Strings["CreditingRateFootnote"         ] = &CreditingRateFootnote         ;
    Strings["GrossRateFootnote"             ] = &GrossRateFootnote             ;
    Strings["NetRateFootnote"               ] = &NetRateFootnote               ;
    Strings["MecFootnote"                   ] = &MecFootnote                   ;
    Strings["GptFootnote"                   ] = &GptFootnote                   ;
    Strings["MidpointValuesFootnote"        ] = &MidpointValuesFootnote        ;
    Strings["SinglePremiumFootnote"         ] = &SinglePremiumFootnote         ;
    Strings["MonthlyChargesFootnote"        ] = &MonthlyChargesFootnote        ;
    Strings["UltCreditingRateFootnote"      ] = &UltCreditingRateFootnote      ;
    Strings["MaxNaarFootnote"               ] = &MaxNaarFootnote               ;
    Strings["PremTaxSurrChgFootnote"        ] = &PremTaxSurrChgFootnote        ;
    Strings["PolicyFeeFootnote"             ] = &PolicyFeeFootnote             ;
    Strings["AssetChargeFootnote"           ] = &AssetChargeFootnote           ;
    Strings["InvestmentIncomeFootnote"      ] = &InvestmentIncomeFootnote      ;
    Strings["IrrDbFootnote"                 ] = &IrrDbFootnote                 ;
    Strings["IrrCsvFootnote"                ] = &IrrCsvFootnote                ;
    Strings["MortalityChargesFootnote"      ] = &MortalityChargesFootnote      ;
    Strings["LoanAndWithdrawalFootnote"     ] = &LoanAndWithdrawalFootnote     ;
    Strings["LoanFootnote"                  ] = &LoanFootnote                  ;
    Strings["ImprimaturPresale"             ] = &ImprimaturPresale             ;
    Strings["ImprimaturPresaleComposite"    ] = &ImprimaturPresaleComposite    ;
    Strings["ImprimaturInforce"             ] = &ImprimaturInforce             ;
    Strings["ImprimaturInforceComposite"    ] = &ImprimaturInforceComposite    ;
    Strings["StateMarketingImprimatur"      ] = &StateMarketingImprimatur      ;
    Strings["InforceNonGuaranteedFootnote0" ] = &InforceNonGuaranteedFootnote0 ;
    Strings["InforceNonGuaranteedFootnote1" ] = &InforceNonGuaranteedFootnote1 ;
    Strings["InforceNonGuaranteedFootnote2" ] = &InforceNonGuaranteedFootnote2 ;
    Strings["InforceNonGuaranteedFootnote3" ] = &InforceNonGuaranteedFootnote3 ;
    Strings["NonGuaranteedFootnote"         ] = &NonGuaranteedFootnote         ;
    Strings["MonthlyChargesPaymentFootnote" ] = &MonthlyChargesPaymentFootnote ;
    Strings["SurrenderFootnote"             ] = &SurrenderFootnote             ;
    Strings["PortabilityFootnote"           ] = &PortabilityFootnote           ;
    Strings["FundRateFootnote"              ] = &FundRateFootnote              ;
    Strings["FundRateFootnote0"             ] = &FundRateFootnote0             ;
    Strings["FundRateFootnote1"             ] = &FundRateFootnote1             ;
    Strings["IssuingCompanyFootnote"        ] = &IssuingCompanyFootnote        ;
    Strings["SubsidiaryFootnote"            ] = &SubsidiaryFootnote            ;
    Strings["PlacementAgentFootnote"        ] = &PlacementAgentFootnote        ;
    Strings["MarketingNameFootnote"         ] = &MarketingNameFootnote         ;

    Strings["ProductName"                   ] = &ProductName                   ;
    Strings["ProducerName"                  ] = &ProducerName                  ;
    Strings["ProducerStreet"                ] = &ProducerStreet                ;
    Strings["ProducerCity"                  ] = &ProducerCity                  ;
    Strings["CorpName"                      ] = &CorpName                      ;
    Strings["MasterContractNumber"          ] = &MasterContractNumber          ;
    Strings["ContractNumber"                ] = &ContractNumber                ;
    Strings["Insured1"                      ] = &Insured1                      ;
    Strings["Gender"                        ] = &Gender                        ;
    Strings["UWType"                        ] = &UWType                        ;
    Strings["Smoker"                        ] = &Smoker                        ;
    Strings["UWClass"                       ] = &UWClass                       ;
    Strings["SubstandardTable"              ] = &SubstandardTable              ;
    Strings["DefnLifeIns"                   ] = &DefnLifeIns                   ;
    Strings["DefnMaterialChange"            ] = &DefnMaterialChange            ;
    Strings["AvoidMec"                      ] = &AvoidMec                      ;
    Strings["PartMortTableName"             ] = &PartMortTableName             ;
    Strings["StatePostalAbbrev"             ] = &StatePostalAbbrev             ;
    Strings["PremiumTaxState"               ] = &PremiumTaxState               ;
    Strings["CountryIso3166Abbrev"          ] = &CountryIso3166Abbrev          ;
    Strings["Comments"                      ] = &Comments                      ;

    Strings["SupplementalReportColumn00"    ] = &SupplementalReportColumn00    ;
    Strings["SupplementalReportColumn01"    ] = &SupplementalReportColumn01    ;
    Strings["SupplementalReportColumn02"    ] = &SupplementalReportColumn02    ;
    Strings["SupplementalReportColumn03"    ] = &SupplementalReportColumn03    ;
    Strings["SupplementalReportColumn04"    ] = &SupplementalReportColumn04    ;
    Strings["SupplementalReportColumn05"    ] = &SupplementalReportColumn05    ;
    Strings["SupplementalReportColumn06"    ] = &SupplementalReportColumn06    ;
    Strings["SupplementalReportColumn07"    ] = &SupplementalReportColumn07    ;
    Strings["SupplementalReportColumn08"    ] = &SupplementalReportColumn08    ;
    Strings["SupplementalReportColumn09"    ] = &SupplementalReportColumn09    ;
    Strings["SupplementalReportColumn10"    ] = &SupplementalReportColumn10    ;
    Strings["SupplementalReportColumn11"    ] = &SupplementalReportColumn11    ;

    // Special-case strings.

    Strings["InitDBOpt"                     ] = &InitDBOpt                     ;

    LedgerBase::Alloc();

    // Scalar or vector data not compatible with type 'double' can't
    // be part of the maps populated above. We can reserve space for
    // such vectors, though, if we know what their lengths will be.

    EeMode              .reserve(Length);
    ErMode              .reserve(Length);
    DBOpt               .reserve(Length);

    // Vectors of length other than 'Length' can't be part of the maps
    // populated above, but we can reserve space for them here if we
    // know what their lengths will be.

    // 'InforceLives' must be one longer than most vectors, so that
    // it can hold both BOY and EOY values for all years. It might
    // seem more natural to initialize it to unity here because it's
    // used as a multiplier, but the composite when constructed must
    // have zeros, so that adding each cell to it produces the
    // correct total. For each actual non-composite cell, it's
    // initialized correctly by the account-value class.
    InforceLives        .assign(1 + Length, 0.0);

    // Data excluded from the maps above must be copied explicitly in
    // Copy(), which is called by the copy ctor and assignment operator.

    Init();
}

//============================================================================
void LedgerInvariant::Copy(LedgerInvariant const& obj)
{
    LedgerBase::Copy(obj);

    // Vectors of type not compatible with double.
    EeMode                 = obj.EeMode                ;
    ErMode                 = obj.ErMode                ;
    DBOpt                  = obj.DBOpt                 ;

    // Vectors of idiosyncratic length.
    InforceLives           = obj.InforceLives          ;
    FundNumbers            = obj.FundNumbers           ;
    FundNames              = obj.FundNames             ;
    FundAllocs             = obj.FundAllocs            ;
    FundAllocations        = obj.FundAllocations       ;

    // Scalars of type not compatible with double.
    EffDate                = obj.EffDate               ;
    DateOfBirth            = obj.DateOfBirth           ;
    ListBillDate           = obj.ListBillDate          ;
    InforceAsOfDate        = obj.InforceAsOfDate       ;
    InitErMode             = obj.InitErMode            ;
    InitDBOpt              = obj.InitDBOpt             ;

    // Private internals.
    irr_precision_         = obj.irr_precision_        ;
    irr_initialized_       = false; // IRR vectors are not copied.
    FullyInitialized       = obj.FullyInitialized      ;
}

//============================================================================
void LedgerInvariant::Destroy()
{
    irr_initialized_ = false;
    FullyInitialized = false;
}

//============================================================================
void LedgerInvariant::Init()
{
    // Zero-initialize elements of AllVectors and AllScalars.
    LedgerBase::Initialize(GetLength());

    EeMode              .assign(Length, mce_mode(mce_annual));
    ErMode              .assign(Length, mce_mode(mce_annual));
    DBOpt               .assign(Length, mce_dbopt(mce_option1));

    InforceYear         = Length;
    InforceMonth        = 11;

    MecYear             = Length;
    MecMonth            = 11;

    // TODO ?? Probably every member should be initialized.
    Age                 = 100;
    NoLapseMinDur       = 100;
    NoLapseMinAge       = 100;
    NoLapseAlwaysActive = false;
    Has1035ExchCharge   = false;

    SupplementalReport  = false;

    irr_precision_      = 0;
    irr_initialized_    = false;
    FullyInitialized    = false;
}

//============================================================================
void LedgerInvariant::Init(BasicValues const* b)
{
    // Zero-initialize almost everything.
    Init();

    irr_precision_ = b->round_irr().decimals();

// TODO ?? These names are confusing. EePmt and ErPmt are *input* values.
// If they're entered as $1000 for all years, then they have that value
// every year, even after lapse. Variables whose names end in -'GrossPmt'
// hold the results of transaction processing, e.g. $0 after lapse.
// EePmt and ErPmt are used e.g. in premium-strategy calculations.

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
//    ProducerCompensation  =

    HasSupplSpecAmt = false;
    if(b->yare_input_.TermRider)
        {
        TermSpecAmt     .assign(Length, b->yare_input_.TermRiderAmount);
        }
    else if(b->Database_->Query(DB_TermIsNotRider))
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
    CurrMandE            = b->InterestRates_->MAndERate(mce_gen_curr);
    TotalIMF             = b->InterestRates_->InvestmentManagementFee();
    RefundableSalesLoad  = b->Loads_->refundable_sales_load_proportion();

    std::vector<double> coimult;
    b->Database_->Query(coimult, DB_CurrCoiMultiplier);
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

    // The antediluvian branch has a null FundData_ object.
    int number_of_funds(0);
    if(b->FundData_)
        {
        number_of_funds = b->FundData_->GetNumberOfFunds();
        }

//    enum{NumberOfFunds = 30}; // DEPRECATED
    int const NumberOfFunds = 30; // DEPRECATED
    int expected_number_of_funds = std::max(number_of_funds, NumberOfFunds);
    std::vector<double> v(b->yare_input_.FundAllocations);
    if(v.size() < static_cast<unsigned int>(expected_number_of_funds))
        {
        v.insert(v.end(), expected_number_of_funds - v.size(), 0.0);
        }

    for(int j = 0; j < number_of_funds; j++)
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

    NoLapseAlwaysActive     = b->Database_->Query(DB_NoLapseAlwaysActive);
    NoLapseMinDur           = b->Database_->Query(DB_NoLapseMinDur);
    NoLapseMinAge           = b->Database_->Query(DB_NoLapseMinAge);
    Has1035ExchCharge       = b->Database_->Query(DB_Has1035ExchCharge);

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
    GroupIndivSelection     = b->Database_->Query(DB_GroupIndivSelection);
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
    SplitMinPrem            = b->Database_->Query(DB_SplitMinPrem);
    AllowDbo3               = b->Database_->Query(DB_AllowDbo3);

    // These are reassigned below based on product data if available.
    std::string dbo_name_option1 = mc_str(mce_option1);
    std::string dbo_name_option2 = mc_str(mce_option2);
    std::string dbo_name_rop     = mc_str(mce_rop    );

    // The antediluvian branch has a null ProductData_ object.
    if(b->ProductData_)
        {
        product_data const& p = *b->ProductData_;
        // Accommodate one alternative policy-form name.
        // DATABASE !! It would be much better, of course, to let all
        // strings in class product_data vary across the same axes as
        // database_entity objects.
        bool alt_form = b->Database_->Query(DB_UsePolicyFormAlt);
        dbo_name_option1               = p.datum("DboNameLevel"                   );
        dbo_name_option2               = p.datum("DboNameIncreasing"              );
        dbo_name_rop                   = p.datum("DboNameReturnOfPremium"         );
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
    oenum_smoking_or_tobacco smoke_or_tobacco =
        static_cast<oenum_smoking_or_tobacco>
            (static_cast<int>(b->Database_->Query(DB_SmokeOrTobacco))
            );
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
        :throw std::logic_error("Unrecognized initial death benefit option.")
        ;

    DefnLifeIns             = mc_str(b->yare_input_.DefinitionOfLifeInsurance);
    DefnMaterialChange      = mc_str(b->yare_input_.DefinitionOfMaterialChange);
    AvoidMec                = mc_str(b->yare_input_.AvoidMecMethod);
    PartMortTableName       = "1983 GAM"; // TODO ?? Hardcoded.
    StatePostalAbbrev       = mc_str(b->GetStateOfJurisdiction());
    PremiumTaxState         = mc_str(b->GetPremiumTaxState());

    InitAnnLoanDueRate      = b->InterestRates_->RegLnDueRate
        (mce_gen_curr
        ,mce_annual_rate
        )[0];

    IsInforce = b->yare_input_.EffectiveDate != b->yare_input_.InforceAsOfDate;
    // This test is probably redundant because it is already performed
    // in class Input. But it's difficult to prove that it is actually
    // redundant and will always remain so, while repeating it here
    // costs little and gives a stronger guarantee that illustrations
    // that would violate this rule cannot be produced.
// Fails:
//   File | New | Illustration
//   subtract one day from "Effective date"
//   OK
//   Illustration | Edit cell... [fails irrecoverably]
// Therefore, these diagnostics are temporarily suppressed for input
// files created by lmi--but not for extracts from vendor systems,
// whose dates should not be altered by lmi users.
if(1 != b->yare_input_.InforceDataSource)
  {
    if(IsInforce && (0 == b->yare_input_.InforceYear && 0 == b->yare_input_.InforceMonth))
        {
        alarum()
            << "Inforce illustrations not permitted during month of issue."
            << LMI_FLUSH
            ;
        }
  }

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

//============================================================================
LedgerInvariant& LedgerInvariant::PlusEq(LedgerInvariant const& a_Addend)
{
    LedgerBase::PlusEq(a_Addend, a_Addend.InforceLives);

    irr_precision_ = a_Addend.irr_precision_;

    std::vector<double> const& N = a_Addend.InforceLives;
    int Max = std::min(Length, a_Addend.Length);

    // ET !! This is of the form 'x = (lengthof x) take y'.
    // Make sure total (this) has enough years to add all years of a_Addend to.
    LMI_ASSERT(a_Addend.Length <= Length);
    for(int j = 0; j < Max; j++)
        {
        if(0.0 == N[j])
            break;
        // Don't multiply InforceLives by N--it *is* N.
        InforceLives    [j] += a_Addend.InforceLives    [j];
        }
    // InforceLives is one longer than the other vectors.
    InforceLives        [Max] += a_Addend.InforceLives  [Max];

//  GenderDistinct          = 0;
//  GenderBlended           = 0;
//  Smoker                  = a_Addend.Smoking;
//  SmokerDistinct          = 0;
//  SmokerBlended           = 0;
//  UWClass                 = a_Addend.Class;
//  SubstandardTable

// Should different cells in a census have different effective dates?
// Should there be any consistency requirements at all?
//
// Pro: A census is just a collection of individuals. We could consider
// adding the ability to append any individual (.ill) to any census (.cns).
// That might be handy for putting together a regression test deck. Most
// important, suppose a client bought one product for a number of employees
// a few years ago, and is now adding new employees under a successor product;
// don't we want to give them a combined composite if they ask?
//
// Con: Our task is simpler if we enforce strict consistency requirements.
// There's less to think of, so the program will probably have fewer bugs;
// although if overly strict requirements prevent the user from doing what
// they want in some cases, that's a defect. I believe that vendor systems
// generally have very strict consistency requirements.
//
// As to effective date in particular, I'm not sure what this item will
// actually mean when we do reproposals. If it's plan inception date, then
// it should be allowed to vary. If it's "as-of date" then I can't see a
// reason to let it vary. Maybe these are different needs requiring separate
// input items. What would we do with a plan established thirty years ago
// that includes an individual who's already past the maturity date?
//
// Anyway, those are my thoughts; what conclusion would you reach?

    EffDate                       = a_Addend.EffDate;
    EffDateJdn                    = a_Addend.EffDateJdn;
    DateOfBirth                   = a_Addend.DateOfBirth;
    DateOfBirthJdn                = a_Addend.DateOfBirthJdn;
    ListBillDate                  = a_Addend.ListBillDate;
    ListBillDateJdn               = a_Addend.ListBillDateJdn;
    InforceAsOfDate               = a_Addend.InforceAsOfDate;
    InforceAsOfDateJdn            = a_Addend.InforceAsOfDateJdn;
    InitErMode                    = a_Addend.InitErMode;
    InitDBOpt                     = a_Addend.InitDBOpt;
    Age                           = std::min(Age, a_Addend.Age);
    RetAge                        = std::min(RetAge, a_Addend.RetAge); // TODO ?? Does this make sense?
    EndtAge                       = std::max(EndtAge, a_Addend.EndtAge);

    DefnLifeIns                   = a_Addend.DefnLifeIns;
    DefnMaterialChange            = a_Addend.DefnMaterialChange;
    AvoidMec                      = a_Addend.AvoidMec;

    // TODO ?? Probably we should assert that these don't vary by life.
    CorpName                      = a_Addend.CorpName;
    MasterContractNumber          = a_Addend.MasterContractNumber;
    ProductName                   = a_Addend.ProductName;
    ProducerName                  = a_Addend.ProducerName;
    ProducerStreet                = a_Addend.ProducerStreet;
    ProducerCity                  = a_Addend.ProducerCity;
    // This would necessarily vary by life:
//  ContractNumber                = "";

    PolicyForm                    = a_Addend.PolicyForm;
    PolicyMktgName                = a_Addend.PolicyMktgName;
    PolicyLegalName               = a_Addend.PolicyLegalName;
    InsCoShortName                = a_Addend.InsCoShortName;
    InsCoName                     = a_Addend.InsCoName;
    InsCoAddr                     = a_Addend.InsCoAddr;
    InsCoStreet                   = a_Addend.InsCoStreet;
    InsCoPhone                    = a_Addend.InsCoPhone;
    MainUnderwriter               = a_Addend.MainUnderwriter;
    MainUnderwriterAddress        = a_Addend.MainUnderwriterAddress;
    CoUnderwriter                 = a_Addend.CoUnderwriter;
    CoUnderwriterAddress          = a_Addend.CoUnderwriterAddress;

    AvName                        = a_Addend.AvName;
    CsvName                       = a_Addend.CsvName;
    CsvHeaderName                 = a_Addend.CsvHeaderName;
    NoLapseProvisionName          = a_Addend.NoLapseProvisionName;
    ContractName                  = a_Addend.ContractName;

    AccountValueFootnote          = a_Addend.AccountValueFootnote;
    AttainedAgeFootnote           = a_Addend.AttainedAgeFootnote;
    CashSurrValueFootnote         = a_Addend.CashSurrValueFootnote;
    DeathBenefitFootnote          = a_Addend.DeathBenefitFootnote;
    InitialPremiumFootnote        = a_Addend.InitialPremiumFootnote;
    NetPremiumFootnote            = a_Addend.NetPremiumFootnote;
    GrossPremiumFootnote          = a_Addend.GrossPremiumFootnote;
    OutlayFootnote                = a_Addend.OutlayFootnote;
    PolicyYearFootnote            = a_Addend.PolicyYearFootnote;

    ADDTerseName                  = a_Addend.ADDTerseName;
    InsurabilityTerseName         = a_Addend.InsurabilityTerseName;
    ChildTerseName                = a_Addend.ChildTerseName;
    SpouseTerseName               = a_Addend.SpouseTerseName;
    TermTerseName                 = a_Addend.TermTerseName;
    WaiverTerseName               = a_Addend.WaiverTerseName;
    AccelBftRiderTerseName        = a_Addend.AccelBftRiderTerseName;
    OverloanRiderTerseName        = a_Addend.OverloanRiderTerseName;

    ADDFootnote                   = a_Addend.ADDFootnote;
    ChildFootnote                 = a_Addend.ChildFootnote;
    SpouseFootnote                = a_Addend.SpouseFootnote;
    TermFootnote                  = a_Addend.TermFootnote;
    WaiverFootnote                = a_Addend.WaiverFootnote;
    AccelBftRiderFootnote         = a_Addend.AccelBftRiderFootnote;
    OverloanRiderFootnote         = a_Addend.OverloanRiderFootnote;

    GroupQuoteShortProductName    = a_Addend.GroupQuoteShortProductName;
    GroupQuoteIsNotAnOffer        = a_Addend.GroupQuoteIsNotAnOffer    ;
    GroupQuoteRidersFooter        = a_Addend.GroupQuoteRidersFooter    ;
    GroupQuotePolicyFormId        = a_Addend.GroupQuotePolicyFormId    ;
    GroupQuoteStateVariations     = a_Addend.GroupQuoteStateVariations ;
    GroupQuoteProspectus          = a_Addend.GroupQuoteProspectus      ;
    GroupQuoteUnderwriter         = a_Addend.GroupQuoteUnderwriter     ;
    GroupQuoteBrokerDealer        = a_Addend.GroupQuoteBrokerDealer    ;
    GroupQuoteRubricMandatory     = a_Addend.GroupQuoteRubricMandatory ;
    GroupQuoteRubricVoluntary     = a_Addend.GroupQuoteRubricVoluntary ;
    GroupQuoteRubricFusion        = a_Addend.GroupQuoteRubricFusion    ;
    GroupQuoteFooterMandatory     = a_Addend.GroupQuoteFooterMandatory ;
    GroupQuoteFooterVoluntary     = a_Addend.GroupQuoteFooterVoluntary ;
    GroupQuoteFooterFusion        = a_Addend.GroupQuoteFooterFusion    ;

    MinimumPremiumFootnote        = a_Addend.MinimumPremiumFootnote;
    PremAllocationFootnote        = a_Addend.PremAllocationFootnote;

    InterestDisclaimer            = a_Addend.InterestDisclaimer;
    GuarMortalityFootnote         = a_Addend.GuarMortalityFootnote;
    ProductDescription            = a_Addend.ProductDescription;
    StableValueFootnote           = a_Addend.StableValueFootnote;
    NoVanishPremiumFootnote       = a_Addend.NoVanishPremiumFootnote;
    RejectPremiumFootnote         = a_Addend.RejectPremiumFootnote;
    ExpRatingFootnote             = a_Addend.ExpRatingFootnote;
    MortalityBlendFootnote        = a_Addend.MortalityBlendFootnote;
    HypotheticalRatesFootnote     = a_Addend.HypotheticalRatesFootnote;
    SalesLoadRefundFootnote       = a_Addend.SalesLoadRefundFootnote;
    NoLapseFootnote               = a_Addend.NoLapseFootnote;
    MarketValueAdjFootnote        = a_Addend.MarketValueAdjFootnote;
    ExchangeChargeFootnote0       = a_Addend.ExchangeChargeFootnote0;
    CurrentValuesFootnote         = a_Addend.CurrentValuesFootnote;
    DBOption1Footnote             = a_Addend.DBOption1Footnote;
    DBOption2Footnote             = a_Addend.DBOption2Footnote;
    ExpRatRiskChargeFootnote      = a_Addend.ExpRatRiskChargeFootnote;
    ExchangeChargeFootnote1       = a_Addend.ExchangeChargeFootnote1;
    FlexiblePremiumFootnote       = a_Addend.FlexiblePremiumFootnote;
    GuaranteedValuesFootnote      = a_Addend.GuaranteedValuesFootnote;
    CreditingRateFootnote         = a_Addend.CreditingRateFootnote;
    GrossRateFootnote             = a_Addend.GrossRateFootnote;
    NetRateFootnote               = a_Addend.NetRateFootnote;
    MecFootnote                   = a_Addend.MecFootnote;
    GptFootnote                   = a_Addend.GptFootnote;
    MidpointValuesFootnote        = a_Addend.MidpointValuesFootnote;
    SinglePremiumFootnote         = a_Addend.SinglePremiumFootnote;
    MonthlyChargesFootnote        = a_Addend.MonthlyChargesFootnote;
    UltCreditingRateFootnote      = a_Addend.UltCreditingRateFootnote;
    MaxNaarFootnote               = a_Addend.MaxNaarFootnote;
    PremTaxSurrChgFootnote        = a_Addend.PremTaxSurrChgFootnote;
    PolicyFeeFootnote             = a_Addend.PolicyFeeFootnote;
    AssetChargeFootnote           = a_Addend.AssetChargeFootnote;
    InvestmentIncomeFootnote      = a_Addend.InvestmentIncomeFootnote;
    IrrDbFootnote                 = a_Addend.IrrDbFootnote;
    IrrCsvFootnote                = a_Addend.IrrCsvFootnote;
    MortalityChargesFootnote      = a_Addend.MortalityChargesFootnote;
    LoanAndWithdrawalFootnote     = a_Addend.LoanAndWithdrawalFootnote;
    LoanFootnote                  = a_Addend.LoanFootnote;
    ImprimaturPresale             = a_Addend.ImprimaturPresale;
    ImprimaturPresaleComposite    = a_Addend.ImprimaturPresaleComposite;
    ImprimaturInforce             = a_Addend.ImprimaturInforce;
    ImprimaturInforceComposite    = a_Addend.ImprimaturInforceComposite;
    StateMarketingImprimatur      = a_Addend.StateMarketingImprimatur;
    InforceNonGuaranteedFootnote0 = a_Addend.InforceNonGuaranteedFootnote0;
    InforceNonGuaranteedFootnote1 = a_Addend.InforceNonGuaranteedFootnote1;
    InforceNonGuaranteedFootnote2 = a_Addend.InforceNonGuaranteedFootnote2;
    InforceNonGuaranteedFootnote3 = a_Addend.InforceNonGuaranteedFootnote3;
    NonGuaranteedFootnote         = a_Addend.NonGuaranteedFootnote;
    MonthlyChargesPaymentFootnote = a_Addend.MonthlyChargesPaymentFootnote;
    SurrenderFootnote             = a_Addend.SurrenderFootnote;
    PortabilityFootnote           = a_Addend.PortabilityFootnote;
    FundRateFootnote              = a_Addend.FundRateFootnote;
    FundRateFootnote0             = a_Addend.FundRateFootnote0;
    FundRateFootnote1             = a_Addend.FundRateFootnote1;
    IssuingCompanyFootnote        = a_Addend.IssuingCompanyFootnote;
    SubsidiaryFootnote            = a_Addend.SubsidiaryFootnote;
    PlacementAgentFootnote        = a_Addend.PlacementAgentFootnote;
    MarketingNameFootnote         = a_Addend.MarketingNameFootnote;

    Comments                      = a_Addend.Comments;

    StatePostalAbbrev             = a_Addend.StatePostalAbbrev;
    PremiumTaxState               = a_Addend.PremiumTaxState;
    InitAnnLoanDueRate            = a_Addend.InitAnnLoanDueRate;
    GroupIndivSelection           = GroupIndivSelection   || a_Addend.GroupIndivSelection;
    UseExperienceRating           = a_Addend.UseExperienceRating;
    UsePartialMort                = a_Addend.UsePartialMort;
    PartMortTableName             = a_Addend.PartMortTableName;
    GuarMaxMandE                  = std::max(GuarMaxMandE   , a_Addend.GuarMaxMandE   );
    InitDacTaxRate                = std::max(InitDacTaxRate , a_Addend.InitDacTaxRate );
    InitPremTaxRate               = std::max(InitPremTaxRate, a_Addend.InitPremTaxRate);
    AvgFund                       = a_Addend.AvgFund;
    CustomFund                    = a_Addend.CustomFund;
    FundNumbers                   = a_Addend.FundNumbers;
    FundNames                     = a_Addend.FundNames;
    FundAllocs                    = a_Addend.FundAllocs;
    FundAllocations               = a_Addend.FundAllocations;
    SplitFundAllocation           = SplitFundAllocation   || a_Addend.SplitFundAllocation;
    GenAcctAllocation             = a_Addend.GenAcctAllocation;
    GenderDistinct                = a_Addend.GenderDistinct;
    GenderBlended                 = a_Addend.GenderBlended;
    Smoker                        = a_Addend.Smoker;
    SmokerDistinct                = a_Addend.SmokerDistinct;
    SmokerBlended                 = a_Addend.SmokerBlended;

    PartMortTableMult             = a_Addend.PartMortTableMult;
    TotalIMF                      = a_Addend.TotalIMF;
    RefundableSalesLoad           = a_Addend.RefundableSalesLoad;

    IsMec                         = a_Addend.IsMec        || IsMec;
    InforceIsMec                  = a_Addend.InforceIsMec || InforceIsMec;

    if(InforceYear == a_Addend.InforceYear)
        {
        InforceMonth              = std::min(InforceMonth, a_Addend.InforceMonth);
        }
    else if(a_Addend.InforceYear < InforceYear)
        {
        InforceMonth              = a_Addend.InforceMonth;
        }
    InforceYear                   = std::min(InforceYear, a_Addend.InforceYear);

    if(MecYear == a_Addend.MecYear)
        {
        MecMonth                  = std::min(MecMonth, a_Addend.MecMonth);
        }
    else if(a_Addend.MecYear < MecYear)
        {
        MecMonth                  = a_Addend.MecMonth;
        }
    MecYear                       = std::min(MecYear, a_Addend.MecYear);

    HasWP           = HasWP           || a_Addend.HasWP          ;
    HasADD          = HasADD          || a_Addend.HasADD         ;
    HasTerm         = HasTerm         || a_Addend.HasTerm        ;
    HasSupplSpecAmt = HasSupplSpecAmt || a_Addend.HasSupplSpecAmt;

// TODO ?? Can these be meaningful on a composite? If totals are desired,
// then term should be treated the same way.
//    ChildRiderAmount   = ChildRiderAmount   || a_Addend.ChildRiderAmount  ;
//    SpouseRiderAmount  = SpouseRiderAmount  || a_Addend.SpouseRiderAmount ;

    HasChildRider      = HasChildRider      || a_Addend.HasChildRider     ;
    HasSpouseRider     = HasSpouseRider     || a_Addend.HasSpouseRider    ;

// TODO ?? For some ages, we use min; for others, max; how about this one?
//    SpouseIssueAge     =

    HasHoneymoon       = HasHoneymoon || a_Addend.HasHoneymoon ;
    PostHoneymoonSpread= a_Addend.PostHoneymoonSpread          ;
    SplitMinPrem       = SplitMinPrem || a_Addend.SplitMinPrem ;
    AllowDbo3          = AllowDbo3    || a_Addend.AllowDbo3    ;

    NoLapseMinDur      = std::min(a_Addend.NoLapseMinDur, NoLapseMinDur);
    NoLapseMinAge      = std::min(a_Addend.NoLapseMinAge, NoLapseMinAge);
    NoLapseAlwaysActive= a_Addend.NoLapseAlwaysActive|| NoLapseAlwaysActive;
    Has1035ExchCharge  = a_Addend.Has1035ExchCharge  || Has1035ExchCharge;

    // Logical OR because IsInforce is a taint that prevents us from
    // calculating a meaningful IRR. For one thing, we lack payment
    // history. For another, even if we had it, payments probably
    // wouldn't be equally spaced, so we'd need a more general irr
    // routine.
    IsInforce     = IsInforce     || a_Addend.IsInforce    ;

    // TODO ?? This doesn't seem quite right, but what would be better?
    // We can't take the union of all columns selected for any life,
    // because its cardinality might exceed the maximum.
    SupplementalReport  = SupplementalReport || a_Addend.SupplementalReport;
    SupplementalReportColumn00 = a_Addend.SupplementalReportColumn00;
    SupplementalReportColumn01 = a_Addend.SupplementalReportColumn01;
    SupplementalReportColumn02 = a_Addend.SupplementalReportColumn02;
    SupplementalReportColumn03 = a_Addend.SupplementalReportColumn03;
    SupplementalReportColumn04 = a_Addend.SupplementalReportColumn04;
    SupplementalReportColumn05 = a_Addend.SupplementalReportColumn05;
    SupplementalReportColumn06 = a_Addend.SupplementalReportColumn06;
    SupplementalReportColumn07 = a_Addend.SupplementalReportColumn07;
    SupplementalReportColumn08 = a_Addend.SupplementalReportColumn08;
    SupplementalReportColumn09 = a_Addend.SupplementalReportColumn09;
    SupplementalReportColumn10 = a_Addend.SupplementalReportColumn10;
    SupplementalReportColumn11 = a_Addend.SupplementalReportColumn11;

    return *this;
}

//============================================================================
// TODO ?? It is extraordinary that this "invariant" class uses and
// even sets some data that vary by basis and therefore seem to belong
// in the complementary "variant" class.

void LedgerInvariant::CalculateIrrs(Ledger const& LedgerValues)
{
    irr_initialized_ = false;

    // Terse aliases for invariants.
    int const m = LedgerValues.GetMaxLength();
    int const n = irr_precision_;

    LedgerVariant const& Curr_ = LedgerValues.GetCurrFull();
    LedgerVariant const& Guar_ = LedgerValues.GetGuarFull();

    irr(Outlay, Guar_.CSVNet,      IrrCsvGuarInput, Guar_.LapseYear, m, n);
    irr(Outlay, Guar_.EOYDeathBft, IrrDbGuarInput,  Guar_.LapseYear, m, n);
    irr(Outlay, Curr_.CSVNet,      IrrCsvCurrInput, Curr_.LapseYear, m, n);
    irr(Outlay, Curr_.EOYDeathBft, IrrDbCurrInput,  Curr_.LapseYear, m, n);

    // Calculate these IRRs only for ledger types that actually use a
    // basis with a zero percent separate-account rate. This is a
    // matter not of efficiency but of validity: values for unused
    // bases are not dependably initialized.
    //
    // This calculation should be distributed among the variant
    // ledgers, so that it gets run for every basis actually used.
    if
        (0 == std::count
            (LedgerValues.GetRunBases().begin()
            ,LedgerValues.GetRunBases().end()
            ,mce_run_gen_curr_sep_zero
            // Proxy for mce_run_gen_guar_sep_zero too.
            )
        )
        {
        // PDF !! Initialize the '0'-suffixed IRRs here.
        irr_initialized_ = true;
        return;
        }

    LedgerVariant const& Curr0 = LedgerValues.GetCurrZero();
    LedgerVariant const& Guar0 = LedgerValues.GetGuarZero();

    irr(Outlay, Guar0.CSVNet,      IrrCsvGuar0, Guar0.LapseYear, m, n);
    irr(Outlay, Guar0.EOYDeathBft, IrrDbGuar0,  Guar0.LapseYear, m, n);
    irr(Outlay, Curr0.CSVNet,      IrrCsvCurr0, Curr0.LapseYear, m, n);
    irr(Outlay, Curr0.EOYDeathBft, IrrDbCurr0,  Curr0.LapseYear, m, n);

    irr_initialized_ = true;
}

//============================================================================
void LedgerInvariant::UpdateCRC(CRC& a_crc) const
{
    LedgerBase::UpdateCRC(a_crc);

    a_crc += InforceLives;
    a_crc += mc_e_vector_to_string_vector(EeMode);
    a_crc += mc_e_vector_to_string_vector(ErMode);
    a_crc += mc_e_vector_to_string_vector(DBOpt);
    a_crc += FundNumbers;
    a_crc += FundNames;
    a_crc += FundAllocs;
    a_crc += FundAllocations;
}

//============================================================================
void LedgerInvariant::Spew(std::ostream& os) const
{
    LedgerBase::Spew(os);

    SpewVector(os, std::string("InforceLives")     ,InforceLives    );
    SpewVector(os, std::string("EeMode")           ,EeMode          );
    SpewVector(os, std::string("ErMode")           ,ErMode          );
    SpewVector(os, std::string("DBOpt")            ,DBOpt           );
    SpewVector(os, std::string("FundNumbers")      ,FundNumbers     );
    SpewVector(os, std::string("FundNames")        ,FundNames       );
    SpewVector(os, std::string("FundAllocs")       ,FundAllocs      );
    SpewVector(os, std::string("FundAllocations")  ,FundAllocations );
}

