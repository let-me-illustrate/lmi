// Ledger data that do not vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "contains.hpp"
#include "crc32.hpp"
#include "financial.hpp"                // for CalculateIrrs()
#include "ledger.hpp"                   // for CalculateIrrs()
#include "ledger_variant.hpp"           // for CalculateIrrs()
#include "mc_enum_aux.hpp"              // mc_e_vector_to_string_vector()
#include "oecumenic_enumerations.hpp"

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
    Length = len;

    // BOY vectors.
    BegYearVectors  ["GrossPmt"                   ] = &GrossPmt                   ;
    BegYearVectors  ["EeGrossPmt"                 ] = &EeGrossPmt                 ;
    BegYearVectors  ["ErGrossPmt"                 ] = &ErGrossPmt                 ;
    BegYearVectors  ["NetWD"                      ] = &NetWD                      ;
    BegYearVectors  ["NewCashLoan"                ] = &NewCashLoan                ;
    BegYearVectors  ["Outlay"                     ] = &Outlay                     ;
    BegYearVectors  ["GptForceout"                ] = &GptForceout                ;
    BegYearVectors  ["NaarForceout"               ] = &NaarForceout               ;
    BegYearVectors  ["ModalMinimumPremium"        ] = &ModalMinimumPremium        ;
    BegYearVectors  ["EeModalMinimumPremium"      ] = &EeModalMinimumPremium      ;
    BegYearVectors  ["ErModalMinimumPremium"      ] = &ErModalMinimumPremium      ;
    BegYearVectors  ["AddonMonthlyFee"            ] = &AddonMonthlyFee            ;

    // EOY vectors.
    EndYearVectors  ["TermSpecAmt"                ] = &TermSpecAmt                ;
    EndYearVectors  ["SpecAmt"                    ] = &SpecAmt                    ;
    EndYearVectors  ["Dcv"                        ] = &Dcv                        ;

    // Forborne vectors.
    ForborneVectors ["Salary"                     ] = &Salary                     ;

    // Nonscalable vectors.
    OtherVectors    ["IndvTaxBracket"             ] = &IndvTaxBracket             ;
    OtherVectors    ["CorpTaxBracket"             ] = &CorpTaxBracket             ;
    OtherVectors    ["AnnualFlatExtra"            ] = &AnnualFlatExtra            ;
    OtherVectors    ["HoneymoonValueSpread"       ] = &HoneymoonValueSpread       ;
    OtherVectors    ["PartMortTableMult"          ] = &PartMortTableMult          ;
    OtherVectors    ["AddonCompOnAssets"          ] = &AddonCompOnAssets          ;
    OtherVectors    ["AddonCompOnPremium"         ] = &AddonCompOnPremium         ;
    OtherVectors    ["CorridorFactor"             ] = &CorridorFactor             ;
    OtherVectors    ["Irc7702ic_usual"            ] = &Irc7702ic_usual            ;
    OtherVectors    ["Irc7702ic_glp"              ] = &Irc7702ic_glp              ;
    OtherVectors    ["Irc7702ic_gsp"              ] = &Irc7702ic_gsp              ;
    OtherVectors    ["Irc7702ig_usual"            ] = &Irc7702ig_usual            ;
    OtherVectors    ["Irc7702ig_glp"              ] = &Irc7702ig_glp              ;
    OtherVectors    ["Irc7702ig_gsp"              ] = &Irc7702ig_gsp              ;
    OtherVectors    ["AnnLoanDueRate"             ] = &AnnLoanDueRate             ;
    OtherVectors    ["CurrMandE"                  ] = &CurrMandE                  ;
    OtherVectors    ["TotalIMF"                   ] = &TotalIMF                   ;
    OtherVectors    ["RefundableSalesLoad"        ] = &RefundableSalesLoad        ;

    // Scalable scalars.
    ScalableScalars ["InitBaseSpecAmt"            ] = &InitBaseSpecAmt            ;
    ScalableScalars ["InitTermSpecAmt"            ] = &InitTermSpecAmt            ;
    ScalableScalars ["ChildRiderAmount"           ] = &ChildRiderAmount           ;
    ScalableScalars ["SpouseRiderAmount"          ] = &SpouseRiderAmount          ;
    ScalableScalars ["InitPrem"                   ] = &InitPrem                   ;
    ScalableScalars ["GuarPrem"                   ] = &GuarPrem                   ;
    ScalableScalars ["InitSevenPayPrem"           ] = &InitSevenPayPrem           ;
    ScalableScalars ["InitGSP"                    ] = &InitGSP                    ;
    ScalableScalars ["InitGLP"                    ] = &InitGLP                    ;
    ScalableScalars ["InitTgtPrem"                ] = &InitTgtPrem                ;
    ScalableScalars ["ListBillPremium"            ] = &ListBillPremium            ;
    ScalableScalars ["EeListBillPremium"          ] = &EeListBillPremium          ;
    ScalableScalars ["ErListBillPremium"          ] = &ErListBillPremium          ;
    ScalableScalars ["Dumpin"                     ] = &Dumpin                     ;
    ScalableScalars ["External1035Amount"         ] = &External1035Amount         ;
    ScalableScalars ["Internal1035Amount"         ] = &Internal1035Amount         ;
    ScalableScalars ["InforceTotalAV"             ] = &InforceTotalAV             ;
    ScalableScalars ["InforceTaxBasis"            ] = &InforceTaxBasis            ;

    // Nonscalable scalars.
    OtherScalars    ["MaleProportion"             ] = &MaleProportion             ;
    OtherScalars    ["NonsmokerProportion"        ] = &NonsmokerProportion        ;
    OtherScalars    ["GuarMaxMandE"               ] = &GuarMaxMandE               ;
    OtherScalars    ["InitDacTaxRate"             ] = &InitDacTaxRate             ;
    OtherScalars    ["InitPremTaxRate"            ] = &InitPremTaxRate            ;
    OtherScalars    ["GenderBlended"              ] = &GenderBlended              ;
    OtherScalars    ["SmokerBlended"              ] = &SmokerBlended              ;
    OtherScalars    ["Age"                        ] = &Age                        ;
    OtherScalars    ["RetAge"                     ] = &RetAge                     ;
    OtherScalars    ["EndtAge"                    ] = &EndtAge                    ;
    OtherScalars    ["GroupIndivSelection"        ] = &GroupIndivSelection        ;
    OtherScalars    ["NoLongerIssued"             ] = &NoLongerIssued             ;
    OtherScalars    ["AllowGroupQuote"            ] = &AllowGroupQuote            ;
    OtherScalars    ["TxCallsGuarUwSubstd"        ] = &TxCallsGuarUwSubstd        ;
    OtherScalars    ["UsePartialMort"             ] = &UsePartialMort             ;

    OtherScalars    ["SurviveToExpectancy"        ] = &SurviveToExpectancy        ;
    OtherScalars    ["SurviveToYear"              ] = &SurviveToYear              ;
    OtherScalars    ["SurviveToAge"               ] = &SurviveToAge               ;
    OtherScalars    ["SurvivalMaxYear"            ] = &SurvivalMaxYear            ;
    OtherScalars    ["SurvivalMaxAge"             ] = &SurvivalMaxAge             ;

    OtherScalars    ["AvgFund"                    ] = &AvgFund                    ;
    OtherScalars    ["CustomFund"                 ] = &CustomFund                 ;
    OtherScalars    ["IsMec"                      ] = &IsMec                      ;
    OtherScalars    ["InforceIsMec"               ] = &InforceIsMec               ;
    OtherScalars    ["InforceYear"                ] = &InforceYear                ;
    OtherScalars    ["InforceMonth"               ] = &InforceMonth               ;
    OtherScalars    ["MecYear"                    ] = &MecYear                    ;
    OtherScalars    ["MecMonth"                   ] = &MecMonth                   ;
    OtherScalars    ["HasWP"                      ] = &HasWP                      ;
    OtherScalars    ["HasADD"                     ] = &HasADD                     ;
    OtherScalars    ["HasTerm"                    ] = &HasTerm                    ;
    OtherScalars    ["HasSupplSpecAmt"            ] = &HasSupplSpecAmt            ;
    OtherScalars    ["HasChildRider"              ] = &HasChildRider              ;
    OtherScalars    ["HasSpouseRider"             ] = &HasSpouseRider             ;
    OtherScalars    ["SpouseIssueAge"             ] = &SpouseIssueAge             ;
    OtherScalars    ["HasHoneymoon"               ] = &HasHoneymoon               ;
    OtherScalars    ["PostHoneymoonSpread"        ] = &PostHoneymoonSpread        ;
    OtherScalars    ["SplitMinPrem"               ] = &SplitMinPrem               ;
    OtherScalars    ["ErNotionallyPaysTerm"       ] = &ErNotionallyPaysTerm       ;
    OtherScalars    ["IsSinglePremium"            ] = &IsSinglePremium            ;
    OtherScalars    ["MaxAnnGuarLoanSpread"       ] = &MaxAnnGuarLoanSpread       ;
    OtherScalars    ["MaxAnnCurrLoanDueRate"      ] = &MaxAnnCurrLoanDueRate      ;
    OtherScalars    ["IsInforce"                  ] = &IsInforce                  ;
    OtherScalars    ["CurrentCoiMultiplier"       ] = &CurrentCoiMultiplier       ;
    OtherScalars    ["NoLapseAlwaysActive"        ] = &NoLapseAlwaysActive        ;
    OtherScalars    ["NoLapseMinDur"              ] = &NoLapseMinDur              ;
    OtherScalars    ["NoLapseMinAge"              ] = &NoLapseMinAge              ;
    OtherScalars    ["Has1035ExchCharge"          ] = &Has1035ExchCharge          ;
    OtherScalars    ["EffDateJdn"                 ] = &EffDateJdn                 ;
    OtherScalars    ["DateOfBirthJdn"             ] = &DateOfBirthJdn             ;
    OtherScalars    ["LastCoiReentryDateJdn"      ] = &LastCoiReentryDateJdn      ;
    OtherScalars    ["ListBillDateJdn"            ] = &ListBillDateJdn            ;
    OtherScalars    ["InforceAsOfDateJdn"         ] = &InforceAsOfDateJdn         ;
    OtherScalars    ["GenAcctAllocation"          ] = &GenAcctAllocation          ;
    OtherScalars    ["SplitFundAllocation"        ] = &SplitFundAllocation        ;
    OtherScalars    ["WriteTsvFile"               ] = &WriteTsvFile               ;
    OtherScalars    ["SupplementalReport"         ] = &SupplementalReport         ;

    // Strings.
    Strings         ["PolicyForm"                 ] = &PolicyForm                 ;
    Strings         ["PolicyMktgName"             ] = &PolicyMktgName             ;
    Strings         ["PolicyLegalName"            ] = &PolicyLegalName            ;
    Strings         ["CsoEra"                     ] = &CsoEra                     ;
    Strings         ["InsCoShortName"             ] = &InsCoShortName             ;
    Strings         ["InsCoName"                  ] = &InsCoName                  ;
    Strings         ["InsCoAddr"                  ] = &InsCoAddr                  ;
    Strings         ["InsCoStreet"                ] = &InsCoStreet                ;
    Strings         ["InsCoPhone"                 ] = &InsCoPhone                 ;
    Strings         ["MainUnderwriter"            ] = &MainUnderwriter            ;
    Strings         ["MainUnderwriterAddress"     ] = &MainUnderwriterAddress     ;
    Strings         ["CoUnderwriter"              ] = &CoUnderwriter              ;
    Strings         ["CoUnderwriterAddress"       ] = &CoUnderwriterAddress       ;

    Strings         ["AvName"                     ] = &AvName                     ;
    Strings         ["CsvName"                    ] = &CsvName                    ;
    Strings         ["CsvHeaderName"              ] = &CsvHeaderName              ;
    Strings         ["NoLapseProvisionName"       ] = &NoLapseProvisionName       ;
    Strings         ["ContractName"               ] = &ContractName               ;
    Strings         ["DboName"                    ] = &DboName                    ;
    Strings         ["DboNameLevel"               ] = &DboNameLevel               ;
    Strings         ["DboNameIncreasing"          ] = &DboNameIncreasing          ;
    Strings         ["DboNameMinDeathBenefit"     ] = &DboNameMinDeathBenefit     ;
    Strings         ["GenAcctName"                ] = &GenAcctName                ;
    Strings         ["GenAcctNameElaborated"      ] = &GenAcctNameElaborated      ;
    Strings         ["SepAcctName"                ] = &SepAcctName                ;
    Strings         ["SpecAmtName"                ] = &SpecAmtName                ;
    Strings         ["SpecAmtNameElaborated"      ] = &SpecAmtNameElaborated      ;
    Strings         ["UwBasisMedical"             ] = &UwBasisMedical             ;
    Strings         ["UwBasisParamedical"         ] = &UwBasisParamedical         ;
    Strings         ["UwBasisNonmedical"          ] = &UwBasisNonmedical          ;
    Strings         ["UwBasisSimplified"          ] = &UwBasisSimplified          ;
    Strings         ["UwBasisGuaranteed"          ] = &UwBasisGuaranteed          ;
    Strings         ["UwClassPreferred"           ] = &UwClassPreferred           ;
    Strings         ["UwClassStandard"            ] = &UwClassStandard            ;
    Strings         ["UwClassRated"               ] = &UwClassRated               ;
    Strings         ["UwClassUltra"               ] = &UwClassUltra               ;

    Strings         ["AccountValueFootnote"       ] = &AccountValueFootnote       ;
    Strings         ["AttainedAgeFootnote"        ] = &AttainedAgeFootnote        ;
    Strings         ["CashSurrValueFootnote"      ] = &CashSurrValueFootnote      ;
    Strings         ["DeathBenefitFootnote"       ] = &DeathBenefitFootnote       ;
    Strings         ["InitialPremiumFootnote"     ] = &InitialPremiumFootnote     ;
    Strings         ["NetPremiumFootnote"         ] = &NetPremiumFootnote         ;
    Strings         ["GrossPremiumFootnote"       ] = &GrossPremiumFootnote       ;
    Strings         ["OutlayFootnote"             ] = &OutlayFootnote             ;
    Strings         ["PolicyYearFootnote"         ] = &PolicyYearFootnote         ;

    Strings         ["ADDTerseName"               ] = &ADDTerseName               ;
    Strings         ["InsurabilityTerseName"      ] = &InsurabilityTerseName      ;
    Strings         ["ChildTerseName"             ] = &ChildTerseName             ;
    Strings         ["SpouseTerseName"            ] = &SpouseTerseName            ;
    Strings         ["TermTerseName"              ] = &TermTerseName              ;
    Strings         ["WaiverTerseName"            ] = &WaiverTerseName            ;
    Strings         ["AccelBftRiderTerseName"     ] = &AccelBftRiderTerseName     ;
    Strings         ["OverloanRiderTerseName"     ] = &OverloanRiderTerseName     ;

    Strings         ["ADDFootnote"                ] = &ADDFootnote                ;
    Strings         ["ChildFootnote"              ] = &ChildFootnote              ;
    Strings         ["SpouseFootnote"             ] = &SpouseFootnote             ;
    Strings         ["TermFootnote"               ] = &TermFootnote               ;
    Strings         ["WaiverFootnote"             ] = &WaiverFootnote             ;
    Strings         ["AccelBftRiderFootnote"      ] = &AccelBftRiderFootnote      ;
    Strings         ["OverloanRiderFootnote"      ] = &OverloanRiderFootnote      ;

    Strings         ["GroupQuoteShortProductName" ] = &GroupQuoteShortProductName ;
    Strings         ["GroupQuoteIsNotAnOffer"     ] = &GroupQuoteIsNotAnOffer     ;
    Strings         ["GroupQuoteRidersFooter"     ] = &GroupQuoteRidersFooter     ;
    Strings         ["GroupQuotePolicyFormId"     ] = &GroupQuotePolicyFormId     ;
    Strings         ["GroupQuoteStateVariations"  ] = &GroupQuoteStateVariations  ;
    Strings         ["GroupQuoteProspectus"       ] = &GroupQuoteProspectus       ;
    Strings         ["GroupQuoteUnderwriter"      ] = &GroupQuoteUnderwriter      ;
    Strings         ["GroupQuoteBrokerDealer"     ] = &GroupQuoteBrokerDealer     ;
    Strings         ["GroupQuoteRubricMandatory"  ] = &GroupQuoteRubricMandatory  ;
    Strings         ["GroupQuoteRubricVoluntary"  ] = &GroupQuoteRubricVoluntary  ;
    Strings         ["GroupQuoteRubricFusion"     ] = &GroupQuoteRubricFusion     ;
    Strings         ["GroupQuoteFooterMandatory"  ] = &GroupQuoteFooterMandatory  ;
    Strings         ["GroupQuoteFooterVoluntary"  ] = &GroupQuoteFooterVoluntary  ;
    Strings         ["GroupQuoteFooterFusion"     ] = &GroupQuoteFooterFusion     ;

    Strings         ["MinimumPremiumFootnote"     ] = &MinimumPremiumFootnote     ;
    Strings         ["PremAllocationFootnote"     ] = &PremAllocationFootnote     ;
    Strings         ["InterestDisclaimer"         ] = &InterestDisclaimer         ;
    Strings         ["GuarMortalityFootnote"      ] = &GuarMortalityFootnote      ;
    Strings         ["ProductDescription"         ] = &ProductDescription         ;
    Strings         ["StableValueFootnote"        ] = &StableValueFootnote        ;
    Strings         ["NoVanishPremiumFootnote"    ] = &NoVanishPremiumFootnote    ;
    Strings         ["RejectPremiumFootnote"      ] = &RejectPremiumFootnote      ;
    Strings         ["ExpRatingFootnote"          ] = &ExpRatingFootnote          ;
    Strings         ["MortalityBlendFootnote"     ] = &MortalityBlendFootnote     ;
    Strings         ["HypotheticalRatesFootnote"  ] = &HypotheticalRatesFootnote  ;
    Strings         ["SalesLoadRefundFootnote"    ] = &SalesLoadRefundFootnote    ;
    Strings         ["NoLapseEverFootnote"        ] = &NoLapseEverFootnote        ;
    Strings         ["NoLapseFootnote"            ] = &NoLapseFootnote            ;
    Strings         ["CurrentValuesFootnote"      ] = &CurrentValuesFootnote      ;
    Strings         ["DBOption1Footnote"          ] = &DBOption1Footnote          ;
    Strings         ["DBOption2Footnote"          ] = &DBOption2Footnote          ;
    Strings         ["DBOption3Footnote"          ] = &DBOption3Footnote          ;
    Strings         ["MinDeathBenefitFootnote"    ] = &MinDeathBenefitFootnote    ;
    Strings         ["ExpRatRiskChargeFootnote"   ] = &ExpRatRiskChargeFootnote   ;
    Strings         ["ExchangeChargeFootnote1"    ] = &ExchangeChargeFootnote1    ;
    Strings         ["FlexiblePremiumFootnote"    ] = &FlexiblePremiumFootnote    ;
    Strings         ["GuaranteedValuesFootnote"   ] = &GuaranteedValuesFootnote   ;
    Strings         ["CreditingRateFootnote"      ] = &CreditingRateFootnote      ;
    Strings         ["GrossRateFootnote"          ] = &GrossRateFootnote          ;
    Strings         ["NetRateFootnote"            ] = &NetRateFootnote            ;
    Strings         ["MecFootnote"                ] = &MecFootnote                ;
    Strings         ["GptFootnote"                ] = &GptFootnote                ;
    Strings         ["MidpointValuesFootnote"     ] = &MidpointValuesFootnote     ;
    Strings         ["SinglePremiumFootnote"      ] = &SinglePremiumFootnote      ;
    Strings         ["MonthlyChargesFootnote"     ] = &MonthlyChargesFootnote     ;
    Strings         ["UltCreditingRateFootnote"   ] = &UltCreditingRateFootnote   ;
    Strings         ["UltCreditingRateHeader"     ] = &UltCreditingRateHeader     ;
    Strings         ["MaxNaarFootnote"            ] = &MaxNaarFootnote            ;
    Strings         ["PremTaxSurrChgFootnote"     ] = &PremTaxSurrChgFootnote     ;
    Strings         ["PolicyFeeFootnote"          ] = &PolicyFeeFootnote          ;
    Strings         ["AssetChargeFootnote"        ] = &AssetChargeFootnote        ;
    Strings         ["InvestmentIncomeFootnote"   ] = &InvestmentIncomeFootnote   ;
    Strings         ["IrrDbFootnote"              ] = &IrrDbFootnote              ;
    Strings         ["IrrCsvFootnote"             ] = &IrrCsvFootnote             ;
    Strings         ["MortalityChargesFootnote"   ] = &MortalityChargesFootnote   ;
    Strings         ["LoanAndWithdrawalFootnote"  ] = &LoanAndWithdrawalFootnote  ;
    Strings         ["LoanFootnote"               ] = &LoanFootnote               ;
    Strings         ["ImprimaturPresale"          ] = &ImprimaturPresale          ;
    Strings         ["ImprimaturPresaleComposite" ] = &ImprimaturPresaleComposite ;
    Strings         ["ImprimaturInforce"          ] = &ImprimaturInforce          ;
    Strings         ["ImprimaturInforceComposite" ] = &ImprimaturInforceComposite ;
    Strings         ["StateMarketingImprimatur"   ] = &StateMarketingImprimatur   ;
    Strings         ["NonGuaranteedFootnote"      ] = &NonGuaranteedFootnote      ;
    Strings         ["NonGuaranteedFootnote1"     ] = &NonGuaranteedFootnote1     ;
    Strings         ["NonGuaranteedFootnote1Tx"   ] = &NonGuaranteedFootnote1Tx   ;
    Strings         ["FnMonthlyDeductions"        ] = &FnMonthlyDeductions        ;
    Strings         ["SurrenderFootnote"          ] = &SurrenderFootnote          ;
    Strings         ["PortabilityFootnote"        ] = &PortabilityFootnote        ;
    Strings         ["FundRateFootnote"           ] = &FundRateFootnote           ;
    Strings         ["IssuingCompanyFootnote"     ] = &IssuingCompanyFootnote     ;
    Strings         ["SubsidiaryFootnote"         ] = &SubsidiaryFootnote         ;
    Strings         ["PlacementAgentFootnote"     ] = &PlacementAgentFootnote     ;
    Strings         ["MarketingNameFootnote"      ] = &MarketingNameFootnote      ;
    Strings         ["GuarIssueDisclaimerNcSc"    ] = &GuarIssueDisclaimerNcSc    ;
    Strings         ["GuarIssueDisclaimerMd"      ] = &GuarIssueDisclaimerMd      ;
    Strings         ["GuarIssueDisclaimerTx"      ] = &GuarIssueDisclaimerTx      ;
    Strings         ["IllRegCertAgent"            ] = &IllRegCertAgent            ;
    Strings         ["IllRegCertAgentIl"          ] = &IllRegCertAgentIl          ;
    Strings         ["IllRegCertAgentTx"          ] = &IllRegCertAgentTx          ;
    Strings         ["IllRegCertClient"           ] = &IllRegCertClient           ;
    Strings         ["IllRegCertClientIl"         ] = &IllRegCertClientIl         ;
    Strings         ["IllRegCertClientTx"         ] = &IllRegCertClientTx         ;

    Strings         ["FnMaturityAge"              ] = &FnMaturityAge              ;
    Strings         ["FnPartialMortality"         ] = &FnPartialMortality         ;
    Strings         ["FnProspectus"               ] = &FnProspectus               ;
    Strings         ["FnInitialSpecAmt"           ] = &FnInitialSpecAmt           ;
    Strings         ["FnInforceAcctVal"           ] = &FnInforceAcctVal           ;
    Strings         ["FnInforceTaxBasis"          ] = &FnInforceTaxBasis          ;
    Strings         ["Fn1035Charge"               ] = &Fn1035Charge               ;
    Strings         ["FnMecExtraWarning"          ] = &FnMecExtraWarning          ;
    Strings         ["FnNotTaxAdvice"             ] = &FnNotTaxAdvice             ;
    Strings         ["FnNotTaxAdvice2"            ] = &FnNotTaxAdvice2            ;
    Strings         ["FnImf"                      ] = &FnImf                      ;
    Strings         ["FnCensus"                   ] = &FnCensus                   ;
    Strings         ["FnDacTax"                   ] = &FnDacTax                   ;
    Strings         ["FnDefnLifeIns"              ] = &FnDefnLifeIns              ;
    Strings         ["FnBoyEoy"                   ] = &FnBoyEoy                   ;
    Strings         ["FnGeneralAccount"           ] = &FnGeneralAccount           ;
    Strings         ["FnPpMemorandum"             ] = &FnPpMemorandum             ;
    Strings         ["FnPpAccreditedInvestor"     ] = &FnPpAccreditedInvestor     ;
    Strings         ["FnPpLoads"                  ] = &FnPpLoads                  ;
    Strings         ["FnProposalUnderwriting"     ] = &FnProposalUnderwriting     ;
    Strings         ["FnGuaranteedPremium"        ] = &FnGuaranteedPremium        ;
    Strings         ["FnOmnibusDisclaimer"        ] = &FnOmnibusDisclaimer        ;
    Strings         ["FnInitialDbo"               ] = &FnInitialDbo               ;

    Strings         ["DefnGuarGenAcctRate"        ] = &DefnGuarGenAcctRate        ;
    Strings         ["DefnAV"                     ] = &DefnAV                     ;
    Strings         ["DefnCSV"                    ] = &DefnCSV                    ;
    Strings         ["DefnMec"                    ] = &DefnMec                    ;
    Strings         ["DefnOutlay"                 ] = &DefnOutlay                 ;
    Strings         ["DefnSpecAmt"                ] = &DefnSpecAmt                ;

    Strings         ["ProductName"                ] = &ProductName                ;
    Strings         ["ProducerName"               ] = &ProducerName               ;
    Strings         ["ProducerStreet"             ] = &ProducerStreet             ;
    Strings         ["ProducerCityEtc"            ] = &ProducerCityEtc            ;
    Strings         ["ProducerPhone"              ] = &ProducerPhone              ;
    Strings         ["ProducerId"                 ] = &ProducerId                 ;
    Strings         ["CorpName"                   ] = &CorpName                   ;
    Strings         ["MasterContractNumber"       ] = &MasterContractNumber       ;
    Strings         ["ContractNumber"             ] = &ContractNumber             ;
    Strings         ["Insured1"                   ] = &Insured1                   ;
    Strings         ["Gender"                     ] = &Gender                     ;
    Strings         ["UWType"                     ] = &UWType                     ;
    Strings         ["Smoker"                     ] = &Smoker                     ;
    Strings         ["UWClass"                    ] = &UWClass                    ;
    Strings         ["SubstandardTable"           ] = &SubstandardTable           ;
    Strings         ["DefnLifeIns"                ] = &DefnLifeIns                ;
    Strings         ["DefnMaterialChange"         ] = &DefnMaterialChange         ;
    Strings         ["PartMortTableName"          ] = &PartMortTableName          ;
    Strings         ["StateOfJurisdiction"        ] = &StateOfJurisdiction        ;
    Strings         ["PremiumTaxState"            ] = &PremiumTaxState            ;
    Strings         ["CountryIso3166Abbrev"       ] = &CountryIso3166Abbrev       ;
    Strings         ["Comments"                   ] = &Comments                   ;

    Strings         ["SupplementalReportColumn00" ] = &SupplementalReportColumn00 ;
    Strings         ["SupplementalReportColumn01" ] = &SupplementalReportColumn01 ;
    Strings         ["SupplementalReportColumn02" ] = &SupplementalReportColumn02 ;
    Strings         ["SupplementalReportColumn03" ] = &SupplementalReportColumn03 ;
    Strings         ["SupplementalReportColumn04" ] = &SupplementalReportColumn04 ;
    Strings         ["SupplementalReportColumn05" ] = &SupplementalReportColumn05 ;
    Strings         ["SupplementalReportColumn06" ] = &SupplementalReportColumn06 ;
    Strings         ["SupplementalReportColumn07" ] = &SupplementalReportColumn07 ;
    Strings         ["SupplementalReportColumn08" ] = &SupplementalReportColumn08 ;
    Strings         ["SupplementalReportColumn09" ] = &SupplementalReportColumn09 ;
    Strings         ["SupplementalReportColumn10" ] = &SupplementalReportColumn10 ;
    Strings         ["SupplementalReportColumn11" ] = &SupplementalReportColumn11 ;

    Strings         ["InitDBOpt"                  ] = &InitDBOpt                  ;
    Strings         ["InitEeMode"                 ] = &InitEeMode                 ;
    Strings         ["InitErMode"                 ] = &InitErMode                 ;

    LedgerBase::Alloc();

    // Scalar or vector data not compatible with type 'double' can't
    // be part of the maps populated above. We can reserve space for
    // such vectors, though, if we know what their lengths will be.

    DBOpt        .reserve(Length);
    EeMode       .reserve(Length);
    ErMode       .reserve(Length);

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
    InforceLives .assign(1 + Length, 0.0);

    // Data excluded from the maps above must be copied explicitly in
    // Copy(), which is called by the copy ctor and assignment operator.

    Init();
}

//============================================================================
void LedgerInvariant::Copy(LedgerInvariant const& obj)
{
    LedgerBase::Copy(obj);

    // Vectors of type not compatible with double.
    DBOpt                  = obj.DBOpt                 ;
    EeMode                 = obj.EeMode                ;
    ErMode                 = obj.ErMode                ;

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

    DBOpt                .assign(Length, mce_dbopt(mce_option1));
    EeMode               .assign(Length, mce_mode(mce_annual));
    ErMode               .assign(Length, mce_mode(mce_annual));

    // Nonscalable scalars.

    MaleProportion             = 0;
    NonsmokerProportion        = 0;
    Age                        = 0;
    EndtAge                    = 100;
    NoLongerIssued             = false;
    AllowGroupQuote            = true;
    SurviveToExpectancy        = false;
    SurviveToYear              = false;
    SurviveToAge               = false;
    SurvivalMaxYear            = 0;
    SurvivalMaxAge             = 0;
    InforceYear                = Length;
    InforceMonth               = 11;
    IsMec                      = false;
    MecYear                    = Length;
    MecMonth                   = 11;
    SpouseIssueAge             = 100;
    IsSinglePremium            = oe_flexible_premium;
    CurrentCoiMultiplier       = 0;
    NoLapseAlwaysActive        = false;
    NoLapseMinDur              = 100;
    NoLapseMinAge              = 100;
    Has1035ExchCharge          = false;
    WriteTsvFile               = false;
    SupplementalReport         = true;

    // Probably this should be an "oecumenic" enumeration.
    enum {gregorian_epoch_jdn = 2361222};
    EffDateJdn                 = gregorian_epoch_jdn;
    DateOfBirthJdn             = gregorian_epoch_jdn;
    LastCoiReentryDateJdn      = gregorian_epoch_jdn;
    ListBillDateJdn            = gregorian_epoch_jdn;
    InforceAsOfDateJdn         = gregorian_epoch_jdn;

    // Private internals.

    irr_precision_             = 0;
    irr_initialized_           = false;
    FullyInitialized           = false;
}

// Notes on effective date.
//
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

/// Accumulate an individual cell into a composite ledger.
///
/// For ledger members that are naturally additive, such as premiums,
/// the composite value is the total (weighted by an inforce factor).
/// For nonadditive members, some other operation may clearly be more
/// appropriate than addition--e.g.:
///
///   NoLongerIssued: This flag prohibits new-business illustrations
///   when necessary. Its composite value is a logical OR, so that the
///   prohibition applies to a composite if it applies to any cell.
///
///   AllowGroupQuote: This flag permits group quotes if appropriate.
///   Its composite value is a logical AND, so that the permission
///   applies to a composite only if it applies to every cell.
///
/// Other members may use the same (or other) methods, although with a
/// less cut-and-dried rationale--e.g.:
///
///   IsMec: Composite value is 'true' if any cell is 'true' (i.e.,
///   logical OR). This doesn't mean that the aggregate is actually a
///   MEC; it just means that the composite illustration ought to
///   include the stern tax warnings that are required for a MEC.
///   With a lot more work, footnotes could be specialized for
///   composites ("One or more of these contracts is a MEC", e.g.),
///   but insurers are generally unwilling to go to such lengths.
///
///   MecYear: Composite value is the least of any cell. Thus, if a
///   footnote says that a contract becomes a MEC in a particular
///   year, the composite uses the lowest such year. That's the least
///   bad choice because it produces the most severe footnote.
///
/// For some other members, the composite value is the value for the
/// last cell. This is the default behavior for non-additive members,
/// and at least it has the advantage that if all cells have the same
/// value, then the composite also has that value. For example:
///
///   GenAcctAllocation: Arguably a weighted average would be better,
///   but averages aren't supported in this simple code yet. However,
///   any average would be inaccurate.
///
///   SubstandardTable: This represents a mortality multiplier, for
///   which an average might seem somewhat suitable, but that's hardly
///   possible because substandard tables are quantized. The average
///   of "L=+300%" and "P=+400%" is not table N (350%) because no such
///   table is defined.
///
///   WaiverFootnote: String members in general cannot be composited
///   in any more reasonable way than taking the last cell's value.
///   A composite may include cells whose policy forms have different
///   premium-waiver footnotes, but insurers have little desire to
///   combine them thoughtfully.
///
/// Thus, there exists One True Way for compositing additive members
/// and some boolean members, but composite values assigned here for
/// other members are often arbitrary.

LedgerInvariant& LedgerInvariant::PlusEq(LedgerInvariant const& a_Addend)
{
    LedgerBase::PlusEq(a_Addend, a_Addend.InforceLives);

    // For 'CorridorFactor', an average weighted by number of lives
    // would be inaccurate, and indeed any aggregate value could be
    // surprising because it would not necessarily be monotone (for
    // two lives with a ten-year age difference, the last ten would
    // depend on the younger life only, most likely producing a
    // discontinuity at the older life's maturity age).
    for(int j = 0; j < a_Addend.Length; ++j)
        {
        CorridorFactor [j]  = 0.0;
        }

    irr_precision_ = a_Addend.irr_precision_;

    std::vector<double> const& N = a_Addend.InforceLives;
    int Max = std::min(Length, a_Addend.Length);

    // ET !! This is of the form 'x = (lengthof x) take y'.
    // C++2x provides std::views::{drop,take}, which are somewhat
    // similar to APL's take and drop; however, for
    //   x = (1 + lengthof(y) take y
    // std::views would not extend the data with zeros, as APL would
    // (in APL circles, that's called "overtake").
    //
    // Make sure total (this) has enough years to add all years of a_Addend to.
    LMI_ASSERT(a_Addend.Length <= Length);
    for(int j = 0; j < Max; ++j)
        {
        if(0.0 == N[j])
            break;
        // Don't multiply InforceLives by N--it *is* N.
        InforceLives   [j] += a_Addend.InforceLives    [j];
        }
    // InforceLives is one longer than the other vectors.
    InforceLives     [Max] += a_Addend.InforceLives  [Max];

    FundNumbers                = a_Addend.FundNumbers;
    FundNames                  = a_Addend.FundNames;
    FundAllocs                 = a_Addend.FundAllocs;
    FundAllocations            = a_Addend.FundAllocations;

    // Nonscalable scalars.

    MaleProportion             = std::max(MaleProportion     , a_Addend.MaleProportion);
    NonsmokerProportion        = std::max(NonsmokerProportion, a_Addend.NonsmokerProportion);
    GuarMaxMandE               = std::max(GuarMaxMandE       , a_Addend.GuarMaxMandE);
    InitDacTaxRate             = std::max(InitDacTaxRate     , a_Addend.InitDacTaxRate);
    InitPremTaxRate            = std::max(InitPremTaxRate    , a_Addend.InitPremTaxRate);
    GenderBlended              = a_Addend.GenderBlended;
    SmokerBlended              = a_Addend.SmokerBlended;
    Age                        = std::min(Age, a_Addend.Age);
    RetAge                     = std::min(RetAge, a_Addend.RetAge);
    EndtAge                    = std::max(EndtAge, a_Addend.EndtAge);
    GroupIndivSelection        = GroupIndivSelection   || a_Addend.GroupIndivSelection;
    NoLongerIssued             = NoLongerIssued        || a_Addend.NoLongerIssued;
    AllowGroupQuote            = AllowGroupQuote       && a_Addend.AllowGroupQuote;
    TxCallsGuarUwSubstd        = TxCallsGuarUwSubstd   || a_Addend.TxCallsGuarUwSubstd;
    UsePartialMort             = a_Addend.UsePartialMort;

    SurviveToExpectancy        = a_Addend.SurviveToExpectancy;
    SurviveToYear              = a_Addend.SurviveToYear;
    SurviveToAge               = a_Addend.SurviveToAge;
    SurvivalMaxYear            = a_Addend.SurvivalMaxYear;
    SurvivalMaxAge             = a_Addend.SurvivalMaxAge;

    AvgFund                    = a_Addend.AvgFund;
    CustomFund                 = a_Addend.CustomFund;
    IsMec                      = a_Addend.IsMec        || IsMec;
    InforceIsMec               = a_Addend.InforceIsMec || InforceIsMec;

    if(InforceYear == a_Addend.InforceYear)
        {
        InforceMonth           = std::min(InforceMonth, a_Addend.InforceMonth);
        }
    else if(a_Addend.InforceYear < InforceYear)
        {
        InforceMonth           = a_Addend.InforceMonth;
        }
    InforceYear                = std::min(InforceYear, a_Addend.InforceYear);

    if(MecYear == a_Addend.MecYear)
        {
        MecMonth               = std::min(MecMonth, a_Addend.MecMonth);
        }
    else if(a_Addend.MecYear < MecYear)
        {
        MecMonth               = a_Addend.MecMonth;
        }
    MecYear                    = std::min(MecYear, a_Addend.MecYear);

    HasWP                      = HasWP           || a_Addend.HasWP          ;
    HasADD                     = HasADD          || a_Addend.HasADD         ;
    HasTerm                    = HasTerm         || a_Addend.HasTerm        ;
    HasSupplSpecAmt            = HasSupplSpecAmt || a_Addend.HasSupplSpecAmt;
    HasChildRider              = HasChildRider      || a_Addend.HasChildRider     ;
    HasSpouseRider             = HasSpouseRider     || a_Addend.HasSpouseRider    ;
    SpouseIssueAge             = std::min(SpouseIssueAge, a_Addend.SpouseIssueAge);
    HasHoneymoon               = HasHoneymoon || a_Addend.HasHoneymoon ;
    PostHoneymoonSpread        = a_Addend.PostHoneymoonSpread          ;
    SplitMinPrem               = SplitMinPrem || a_Addend.SplitMinPrem ;
    ErNotionallyPaysTerm       = ErNotionallyPaysTerm || a_Addend.ErNotionallyPaysTerm;
    IsSinglePremium            = std::max(a_Addend.IsSinglePremium      , IsSinglePremium      );
    MaxAnnGuarLoanSpread       = std::max(a_Addend.MaxAnnGuarLoanSpread , MaxAnnGuarLoanSpread );
    MaxAnnCurrLoanDueRate      = std::max(a_Addend.MaxAnnCurrLoanDueRate, MaxAnnCurrLoanDueRate);

    // Logical OR because IsInforce is a taint that prevents us from
    // calculating a meaningful IRR. For one thing, we lack payment
    // history. For another, even if we had it, payments probably
    // wouldn't be equally spaced, so we'd need a more general irr
    // routine.
    IsInforce                  = IsInforce     || a_Addend.IsInforce    ;

    CurrentCoiMultiplier       = std::max(a_Addend.CurrentCoiMultiplier , CurrentCoiMultiplier );
    NoLapseAlwaysActive        = a_Addend.NoLapseAlwaysActive|| NoLapseAlwaysActive;
    NoLapseMinDur              = std::min(a_Addend.NoLapseMinDur, NoLapseMinDur);
    NoLapseMinAge              = std::min(a_Addend.NoLapseMinAge, NoLapseMinAge);
    Has1035ExchCharge          = a_Addend.Has1035ExchCharge  || Has1035ExchCharge;
    EffDateJdn                 = a_Addend.EffDateJdn;
    DateOfBirthJdn             = a_Addend.DateOfBirthJdn;
    LastCoiReentryDateJdn      = a_Addend.LastCoiReentryDateJdn;
    ListBillDateJdn            = a_Addend.ListBillDateJdn;
    InforceAsOfDateJdn         = a_Addend.InforceAsOfDateJdn;
    GenAcctAllocation          = a_Addend.GenAcctAllocation;
    SplitFundAllocation        = SplitFundAllocation   || a_Addend.SplitFundAllocation;
    WriteTsvFile               = WriteTsvFile || a_Addend.WriteTsvFile ;

    // The composite has a supplemental report iff every cell has one,
    // in which case it uses the same columns as the last cell. There
    // is no better general way to decide which columns to use. (The
    // union of all columns selected for any life becomes infeasible
    // when its cardinality exceeds the maximum.)
    //
    SupplementalReport         = SupplementalReport && a_Addend.SupplementalReport;
    // Nonscalable scalars end.

    // Strings.
    //
    // Override the behavior of LedgerBase::PlusEq() for this handful
    // of strings, which would often or even necessarily vary by life.
    ContractNumber             = "";
    Insured1                   = "";
    Gender                     = "";
    Smoker                     = "";
    UWClass                    = "";
    SubstandardTable           = "";

    // Special-case strings.

    EffDate                    = a_Addend.EffDate;
    DateOfBirth                = a_Addend.DateOfBirth;
    LastCoiReentryDate         = a_Addend.LastCoiReentryDate;
    ListBillDate               = a_Addend.ListBillDate;
    InforceAsOfDate            = a_Addend.InforceAsOfDate;

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
    a_crc += mc_e_vector_to_string_vector(DBOpt);
    a_crc += mc_e_vector_to_string_vector(EeMode);
    a_crc += mc_e_vector_to_string_vector(ErMode);
    a_crc += FundNumbers;
    a_crc += FundNames;
    a_crc += FundAllocs;
    a_crc += FundAllocations;
}

//============================================================================
void LedgerInvariant::Spew(std::ostream& os) const
{
    LedgerBase::Spew(os);

    SpewVector(os, std::string("InforceLives")    ,InforceLives    );
    SpewVector(os, std::string("DBOpt")           ,DBOpt           );
    SpewVector(os, std::string("EeMode")          ,EeMode          );
    SpewVector(os, std::string("ErMode")          ,ErMode          );
    SpewVector(os, std::string("FundNumbers")     ,FundNumbers     );
    SpewVector(os, std::string("FundNames")       ,FundNames       );
    SpewVector(os, std::string("FundAllocs")      ,FundAllocs      );
    SpewVector(os, std::string("FundAllocations") ,FundAllocations );
}
