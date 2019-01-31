// Ledger data that do not vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "contains.hpp"
#include "crc32.hpp"
#include "financial.hpp"                // for CalculateIrrs()
#include "ledger.hpp"                   // for CalculateIrrs()
#include "ledger_variant.hpp"           // for CalculateIrrs()
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()

#include <algorithm>                    // max(), min()
#include <ostream>

//============================================================================
LedgerInvariant::LedgerInvariant(int len)
    :LedgerBase       (len)
    ,irr_initialized_ {false}
    ,FullyInitialized {false}
{
    Alloc(len);
}

//============================================================================
LedgerInvariant::LedgerInvariant(LedgerInvariant const& obj)
    :LedgerBase       {obj}
    ,irr_initialized_ {false}
    ,FullyInitialized {false}
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
    OtherVectors    ["AnnLoanDueRate"        ] = &AnnLoanDueRate         ;
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
    ScalableScalars ["ListBillPremium"       ] = &ListBillPremium        ;
    ScalableScalars ["EeListBillPremium"     ] = &EeListBillPremium      ;
    ScalableScalars ["ErListBillPremium"     ] = &ErListBillPremium      ;
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
    OtherScalars    ["ErNotionallyPaysTerm"  ] = &ErNotionallyPaysTerm   ;
    OtherScalars    ["InitAnnLoanDueRate"    ] = &InitAnnLoanDueRate     ;
    OtherScalars    ["IsInforce"             ] = &IsInforce              ;
    OtherScalars    ["CurrentCoiMultiplier"  ] = &CurrentCoiMultiplier   ;
    OtherScalars    ["NoLapseAlwaysActive"   ] = &NoLapseAlwaysActive    ;
    OtherScalars    ["NoLapseMinDur"         ] = &NoLapseMinDur          ;
    OtherScalars    ["NoLapseMinAge"         ] = &NoLapseMinAge          ;
    OtherScalars    ["Has1035ExchCharge"     ] = &Has1035ExchCharge      ;
    OtherScalars    ["EffDateJdn"            ] = &EffDateJdn             ;
    OtherScalars    ["DateOfBirthJdn"        ] = &DateOfBirthJdn         ;
    OtherScalars    ["LastCoiReentryDateJdn" ] = &LastCoiReentryDateJdn  ;
    OtherScalars    ["ListBillDateJdn"       ] = &ListBillDateJdn        ;
    OtherScalars    ["InforceAsOfDateJdn"    ] = &InforceAsOfDateJdn     ;
    OtherScalars    ["SplitFundAllocation"   ] = &SplitFundAllocation    ;
    OtherScalars    ["GenAcctAllocation"     ] = &GenAcctAllocation      ;
    OtherScalars    ["WriteTsvFile"          ] = &WriteTsvFile           ;
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
    LastCoiReentryDate     = obj.LastCoiReentryDate    ;
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
    Age                 = 0;
    EndtAge             = 100;
    NoLapseMinDur       = 100;
    NoLapseMinAge       = 100;
    NoLapseAlwaysActive = false;
    Has1035ExchCharge   = false;

    WriteTsvFile        = false;
    SupplementalReport  = false;

    irr_precision_      = 0;
    irr_initialized_    = false;
    FullyInitialized    = false;
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
    for(int j = 0; j < Max; ++j)
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
    LastCoiReentryDate            = a_Addend.LastCoiReentryDate;
    LastCoiReentryDateJdn         = a_Addend.LastCoiReentryDateJdn;
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

    // PDF !! This is the logic used in the variant ledger class.
    // It's not a very good idea, but until it can be replaced
    // everywhere, at least it ensures that the composite value
    // isn't zero. (In some other cases, zero might be a sensible
    // answer--e.g., for ratios like 'CorridorFactor', where an
    // average weighted by number of lives would be inaccurate,
    // and any aggregate value could be misleading.)
    for(int j = 0; j < a_Addend.Length; ++j)
        {
        AnnLoanDueRate[j] = a_Addend.AnnLoanDueRate[j];
        }

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

    HasChildRider      = HasChildRider      || a_Addend.HasChildRider     ;
    HasSpouseRider     = HasSpouseRider     || a_Addend.HasSpouseRider    ;

    HasHoneymoon       = HasHoneymoon || a_Addend.HasHoneymoon ;
    PostHoneymoonSpread= a_Addend.PostHoneymoonSpread          ;
    SplitMinPrem       = SplitMinPrem || a_Addend.SplitMinPrem ;

    ErNotionallyPaysTerm = ErNotionallyPaysTerm || a_Addend.ErNotionallyPaysTerm;

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

    WriteTsvFile  = WriteTsvFile  || a_Addend.WriteTsvFile ;

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

/// Perform costly IRR calculations on demand only.
///
/// IRRs are not calculated for inforce illustrations because full
/// payment history is generally not available. It would be possible
/// of course to calculate IRRs from the inforce date forward, but
/// it is feared that they'd be misinterpreted: e.g., IRR columns
/// on illustrations run in different inforce years might be taken
/// as directly comparable when they certainly are not.
///
/// IRRs on zero-sepacct-interest bases cannot be calculated for
/// ledger types that do not generate values on those bases (any
/// attempt to access such values as irr() arguments would throw).
/// Here, such impossible calculations are avoided by explicit
/// logic (they might be avoided implicitly if IRRs were set in
/// class LedgerVariant instead).
///
/// Post-construction invariants: All IRR vectors have the same length
/// as any typical vector member. They contain calculated IRRs if
/// possible, or safe initial values of -100% otherwise.
///
/// TODO ?? This function's purpose is to let formatting routines
/// decide whether to calculate IRRs, because those calculations
/// are costly and their results might not be used. Yet pushing any
/// calculations into formatting routines vitiates the separation of
/// concerns.
///
/// TODO ?? It is extraordinary that this function in an "invariant"
/// class uses and even sets data that vary by basis. Consider moving
/// this function into the base class, and the IRR variables into class
/// LedgerVariant.

void LedgerInvariant::CalculateIrrs(Ledger const& LedgerValues)
{
    irr_initialized_ = false;

    IrrCsvGuar0    .resize(Length, -1.0);
    IrrDbGuar0     .resize(Length, -1.0);
    IrrCsvCurr0    .resize(Length, -1.0);
    IrrDbCurr0     .resize(Length, -1.0);
    IrrCsvGuarInput.resize(Length, -1.0);
    IrrDbGuarInput .resize(Length, -1.0);
    IrrCsvCurrInput.resize(Length, -1.0);
    IrrDbCurrInput .resize(Length, -1.0);

    if(IsInforce) {irr_initialized_ = true; return;}

    auto const& r = LedgerValues.GetRunBases();
    bool const run_curr_sep_zero = contains(r, mce_run_gen_curr_sep_zero);
    bool const run_guar_sep_zero = contains(r, mce_run_gen_guar_sep_zero);
    LMI_ASSERT(run_curr_sep_zero == run_guar_sep_zero);
    // Emphasize that one of those is used as a proxy for both:
    bool const zero_sepacct_interest_bases_undefined = !run_curr_sep_zero;

    // Terse aliases for invariants.
    int const m = LedgerValues.greatest_lapse_dur();
    int const n = irr_precision_;

    LedgerVariant const& Curr_ = LedgerValues.GetCurrFull();
    LedgerVariant const& Guar_ = LedgerValues.GetGuarFull();

    irr(Outlay, Guar_.CSVNet,      IrrCsvGuarInput, bourn_cast<int>(Guar_.LapseYear), m, n);
    irr(Outlay, Guar_.EOYDeathBft, IrrDbGuarInput,  bourn_cast<int>(Guar_.LapseYear), m, n);
    irr(Outlay, Curr_.CSVNet,      IrrCsvCurrInput, bourn_cast<int>(Curr_.LapseYear), m, n);
    irr(Outlay, Curr_.EOYDeathBft, IrrDbCurrInput,  bourn_cast<int>(Curr_.LapseYear), m, n);

    if(zero_sepacct_interest_bases_undefined) {irr_initialized_ = true; return;}

    LedgerVariant const& Curr0 = LedgerValues.GetCurrZero();
    LedgerVariant const& Guar0 = LedgerValues.GetGuarZero();

    irr(Outlay, Guar0.CSVNet,      IrrCsvGuar0,     bourn_cast<int>(Guar0.LapseYear), m, n);
    irr(Outlay, Guar0.EOYDeathBft, IrrDbGuar0,      bourn_cast<int>(Guar0.LapseYear), m, n);
    irr(Outlay, Curr0.CSVNet,      IrrCsvCurr0,     bourn_cast<int>(Curr0.LapseYear), m, n);
    irr(Outlay, Curr0.EOYDeathBft, IrrDbCurr0,      bourn_cast<int>(Curr0.LapseYear), m, n);

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
