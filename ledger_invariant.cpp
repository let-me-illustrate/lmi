// Ledger data that do not vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "ledger_invariant.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "crc32.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "financial.hpp"  // TODO ?? For IRRs--prolly don't blong here.
#include "ihs_funddata.hpp"
#include "input.hpp"
#include "interest_rates.hpp"
#include "ledger.hpp" // TODO ?? For IRRs--prolly don't blong here.
#include "ledger_variant.hpp" // TODO ?? For IRRs--prolly don't blong here.
#include "loads.hpp"
#include "mc_enum_aux.hpp" // mc_e_vector_to_string_vector()
#include "mc_enum_types_aux.hpp" // mc_str()
#include "miscellany.hpp"
#include "outlay.hpp"
#include "product_data.hpp"

#include <algorithm>
#include <ostream>
#include <stdexcept>

// TODO ?? It is extraordinary that this 'invariant' class includes
// some data that vary by basis. Perhaps they should be in the
// complementary 'variant' class.

//============================================================================
LedgerInvariant::LedgerInvariant(int len)
    :LedgerBase(len)
    ,FullyInitialized(false)
{
    Alloc(len);
}

//============================================================================
LedgerInvariant::LedgerInvariant(LedgerInvariant const& obj)
    :LedgerBase(obj)
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

    EndYearVectors  ["TermSpecAmt"           ] = &TermSpecAmt            ;
    EndYearVectors  ["SpecAmt"               ] = &SpecAmt                ;
    BegYearVectors  ["ProducerCompensation"  ] = &ProducerCompensation   ;

    OtherVectors    ["IndvTaxBracket"        ] = &IndvTaxBracket         ;
    OtherVectors    ["CorpTaxBracket"        ] = &CorpTaxBracket         ;
    OtherVectors    ["Salary"                ] = &Salary                 ;
    OtherVectors    ["MonthlyFlatExtra"      ] = &MonthlyFlatExtra       ;
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
    ScalableScalars ["PostHoneymoonSpread"   ] = &PostHoneymoonSpread    ;
    ScalableScalars ["Dumpin"                ] = &Dumpin                 ;
    ScalableScalars ["External1035Amount"    ] = &External1035Amount     ;
    ScalableScalars ["Internal1035Amount"    ] = &Internal1035Amount     ;

    OtherScalars    ["MaleProportion"        ] = &MaleProportion         ;
    OtherScalars    ["NonsmokerProportion"   ] = &NonsmokerProportion    ;
    OtherScalars    ["GuarMaxMandE"          ] = &GuarMaxMandE           ;
    OtherScalars    ["GenderDistinct"        ] = &GenderDistinct         ;
    OtherScalars    ["GenderBlended"         ] = &GenderBlended          ;
    OtherScalars    ["SmokerDistinct"        ] = &SmokerDistinct         ;
    OtherScalars    ["SmokerBlended"         ] = &SmokerBlended          ;
    OtherScalars    ["SubstdTable"           ] = &SubstdTable            ; // Prefer string 'b->Input_->SubstandardTable'.
    OtherScalars    ["Age"                   ] = &Age                    ;
    OtherScalars    ["RetAge"                ] = &RetAge                 ;
    OtherScalars    ["EndtAge"               ] = &EndtAge                ;
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
    OtherScalars    ["HasChildRider"         ] = &HasChildRider          ;
    OtherScalars    ["HasSpouseRider"        ] = &HasSpouseRider         ;
    OtherScalars    ["SpouseIssueAge"        ] = &SpouseIssueAge         ;
    OtherScalars    ["HasHoneymoon"          ] = &HasHoneymoon           ;
    OtherScalars    ["AllowDbo3"             ] = &AllowDbo3              ;
    OtherScalars    ["StatePremTaxLoad"      ] = &StatePremTaxLoad       ;
    OtherScalars    ["StatePremTaxRate"      ] = &StatePremTaxRate       ;
    OtherScalars    ["DacTaxPremLoadRate"    ] = &DacTaxPremLoadRate     ;
    OtherScalars    ["InitAnnLoanDueRate"    ] = &InitAnnLoanDueRate     ;
    OtherScalars    ["IsInforce"             ] = &IsInforce              ;
    OtherScalars    ["CountryCOIMultiplier"  ] = &CountryCOIMultiplier   ;
    OtherScalars    ["PremiumTaxLoadIsTiered"] = &PremiumTaxLoadIsTiered ;
    OtherScalars    ["NoLapseAlwaysActive"   ] = &NoLapseAlwaysActive    ;
    OtherScalars    ["NoLapseMinDur"         ] = &NoLapseMinDur          ;
    OtherScalars    ["NoLapseMinAge"         ] = &NoLapseMinAge          ;
    OtherScalars    ["NominallyPar"          ] = &NominallyPar           ;
    OtherScalars    ["Has1035ExchCharge"     ] = &Has1035ExchCharge      ;
    OtherScalars    ["EffDateJdn"            ] = &EffDateJdn             ;
    OtherScalars    ["GenAcctAllocation"     ] = &GenAcctAllocation      ;
    OtherScalars    ["SupplementalReport"    ] = &SupplementalReport     ;

    Strings         ["PolicyMktgName"        ] = &PolicyMktgName         ;
    Strings         ["PolicyLegalName"       ] = &PolicyLegalName        ;
    Strings         ["PolicyForm"            ] = &PolicyForm             ;
    Strings         ["InsCoShortName"        ] = &InsCoShortName         ;
    Strings         ["InsCoName"             ] = &InsCoName              ;
    Strings         ["InsCoAddr"             ] = &InsCoAddr              ;
    Strings         ["InsCoStreet"           ] = &InsCoStreet            ;
    Strings         ["InsCoPhone"            ] = &InsCoPhone             ;
    Strings         ["MainUnderwriter"       ] = &MainUnderwriter        ;
    Strings         ["MainUnderwriterAddress"] = &MainUnderwriterAddress ;
    Strings         ["CoUnderwriter"         ] = &CoUnderwriter          ;
    Strings         ["CoUnderwriterAddress"  ] = &CoUnderwriterAddress   ;

    Strings         ["AvName"                ] = &AvName                 ;
    Strings         ["CsvName"               ] = &CsvName                ;
    Strings         ["CsvHeaderName"         ] = &CsvHeaderName          ;
    Strings         ["NoLapseProvisionName"  ] = &NoLapseProvisionName   ;
    Strings         ["InterestDisclaimer"    ] = &InterestDisclaimer     ;
    Strings         ["GuarMortalityFootnote" ] = &GuarMortalityFootnote  ;

    Strings         ["AccountValueFootnote"  ] = &AccountValueFootnote   ;
    Strings         ["AttainedAgeFootnote"   ] = &AttainedAgeFootnote    ;
    Strings         ["CashSurrValueFootnote" ] = &CashSurrValueFootnote  ;
    Strings         ["DeathBenefitFootnote"  ] = &DeathBenefitFootnote   ;
    Strings         ["InitialPremiumFootnote"] = &InitialPremiumFootnote ;
    Strings         ["NetPremiumFootnote"    ] = &NetPremiumFootnote     ;
    Strings         ["OutlayFootnote"        ] = &OutlayFootnote         ;
    Strings         ["PolicyYearFootnote"    ] = &PolicyYearFootnote     ;

    Strings         ["ADDFootnote"           ] = &ADDFootnote            ;
    Strings         ["ChildFootnote"         ] = &ChildFootnote          ;
    Strings         ["SpouseFootnote"        ] = &SpouseFootnote         ;
    Strings         ["TermFootnote"          ] = &TermFootnote           ;
    Strings         ["WaiverFootnote"        ] = &WaiverFootnote         ;

    Strings         ["MinimumPremiumFootnote"        ] = &MinimumPremiumFootnote         ;
    Strings         ["PremAllocationFootnote"        ] = &PremAllocationFootnote         ;
    Strings         ["ProductDescription"            ] = &ProductDescription             ;
    Strings         ["StableValueFootnote"           ] = &StableValueFootnote            ;
    Strings         ["NoVanishPremiumFootnote"       ] = &NoVanishPremiumFootnote        ;
    Strings         ["RejectPremiumFootnote"         ] = &RejectPremiumFootnote          ;
    Strings         ["ExpRatingFootnote"             ] = &ExpRatingFootnote              ;
    Strings         ["MortalityBlendFootnote"        ] = &MortalityBlendFootnote         ;
    Strings         ["HypotheticalRatesFootnote"     ] = &HypotheticalRatesFootnote      ;
    Strings         ["SalesLoadRefundFootnote"       ] = &SalesLoadRefundFootnote        ;
    Strings         ["NoLapseFootnote"               ] = &NoLapseFootnote                ;
    Strings         ["MarketValueAdjFootnote"        ] = &MarketValueAdjFootnote         ;
    Strings         ["ExchangeChargeFootnote0"       ] = &ExchangeChargeFootnote0        ;
    Strings         ["CurrentValuesFootnote"         ] = &CurrentValuesFootnote          ;
    Strings         ["DBOption1Footnote"             ] = &DBOption1Footnote              ;
    Strings         ["DBOption2Footnote"             ] = &DBOption2Footnote              ;
    Strings         ["ExpRatRiskChargeFootnote"      ] = &ExpRatRiskChargeFootnote       ;
    Strings         ["ExchangeChargeFootnote1"       ] = &ExchangeChargeFootnote1        ;
    Strings         ["FlexiblePremiumFootnote"       ] = &FlexiblePremiumFootnote        ;
    Strings         ["GuaranteedValuesFootnote"      ] = &GuaranteedValuesFootnote       ;
    Strings         ["CreditingRateFootnote"         ] = &CreditingRateFootnote          ;
    Strings         ["MecFootnote"                   ] = &MecFootnote                    ;
    Strings         ["MidpointValuesFootnote"        ] = &MidpointValuesFootnote         ;
    Strings         ["SinglePremiumFootnote"         ] = &SinglePremiumFootnote          ;
    Strings         ["MonthlyChargesFootnote"        ] = &MonthlyChargesFootnote         ;
    Strings         ["UltCreditingRateFootnote"      ] = &UltCreditingRateFootnote       ;
    Strings         ["MaxNaarFootnote"               ] = &MaxNaarFootnote                ;
    Strings         ["PremTaxSurrChgFootnote"        ] = &PremTaxSurrChgFootnote         ;
    Strings         ["PolicyFeeFootnote"             ] = &PolicyFeeFootnote              ;
    Strings         ["AssetChargeFootnote"           ] = &AssetChargeFootnote            ;
    Strings         ["InvestmentIncomeFootnote"      ] = &InvestmentIncomeFootnote       ;
    Strings         ["IrrDbFootnote"                 ] = &IrrDbFootnote                  ;
    Strings         ["IrrCsvFootnote"                ] = &IrrCsvFootnote                 ;
    Strings         ["MortalityChargesFootnote"      ] = &MortalityChargesFootnote       ;
    Strings         ["LoanAndWithdrawalFootnote"     ] = &LoanAndWithdrawalFootnote      ;
    Strings         ["PresaleTrackingNumber"         ] = &PresaleTrackingNumber          ;
    Strings         ["CompositeTrackingNumber"       ] = &CompositeTrackingNumber        ;
    Strings         ["InforceTrackingNumber"         ] = &InforceTrackingNumber          ;
    Strings         ["InforceCompositeTrackingNumber"] = &InforceCompositeTrackingNumber ;
    Strings         ["InforceNonGuaranteedFootnote0" ] = &InforceNonGuaranteedFootnote0  ;
    Strings         ["InforceNonGuaranteedFootnote1" ] = &InforceNonGuaranteedFootnote1  ;
    Strings         ["InforceNonGuaranteedFootnote2" ] = &InforceNonGuaranteedFootnote2  ;
    Strings         ["InforceNonGuaranteedFootnote3" ] = &InforceNonGuaranteedFootnote3  ;
    Strings         ["NonGuaranteedFootnote"         ] = &NonGuaranteedFootnote          ;
    Strings         ["MonthlyChargesPaymentFootnote" ] = &MonthlyChargesPaymentFootnote  ;

    Strings         ["ProducerName"          ] = &ProducerName           ;
    Strings         ["ProducerStreet"        ] = &ProducerStreet         ;
    Strings         ["ProducerCity"          ] = &ProducerCity           ;
    Strings         ["CorpName"              ] = &CorpName               ;
    Strings         ["Franchise"             ] = &Franchise              ;
    Strings         ["PolicyNumber"          ] = &PolicyNumber           ;
    Strings         ["Insured1"              ] = &Insured1               ;
    Strings         ["Gender"                ] = &Gender                 ;
    Strings         ["UWType"                ] = &UWType                 ;
    Strings         ["Smoker"                ] = &Smoker                 ;
    Strings         ["UWClass"               ] = &UWClass                ;
    Strings         ["SubstandardTable"      ] = &SubstandardTable       ;
    Strings         ["DefnLifeIns"           ] = &DefnLifeIns            ;
    Strings         ["DefnMaterialChange"    ] = &DefnMaterialChange     ;
    Strings         ["AvoidMec"              ] = &AvoidMec               ;
    Strings         ["PartMortTableName"     ] = &PartMortTableName      ;
    Strings         ["StatePostalAbbrev"     ] = &StatePostalAbbrev      ;
    Strings         ["CountryIso3166Abbrev"  ] = &CountryIso3166Abbrev   ;
    Strings         ["Comments"              ] = &Comments               ;

    Strings         ["SupplementalReportColumn00" ] = &SupplementalReportColumn00;
    Strings         ["SupplementalReportColumn01" ] = &SupplementalReportColumn01;
    Strings         ["SupplementalReportColumn02" ] = &SupplementalReportColumn02;
    Strings         ["SupplementalReportColumn03" ] = &SupplementalReportColumn03;
    Strings         ["SupplementalReportColumn04" ] = &SupplementalReportColumn04;
    Strings         ["SupplementalReportColumn05" ] = &SupplementalReportColumn05;
    Strings         ["SupplementalReportColumn06" ] = &SupplementalReportColumn06;
    Strings         ["SupplementalReportColumn07" ] = &SupplementalReportColumn07;
    Strings         ["SupplementalReportColumn08" ] = &SupplementalReportColumn08;
    Strings         ["SupplementalReportColumn09" ] = &SupplementalReportColumn09;
    Strings         ["SupplementalReportColumn10" ] = &SupplementalReportColumn10;
    Strings         ["SupplementalReportColumn11" ] = &SupplementalReportColumn11;

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

    irr_precision          = obj.irr_precision         ;

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

    FullyInitialized       = obj.FullyInitialized      ;
}

//============================================================================
void LedgerInvariant::Destroy()
{
    FullyInitialized = false;
}

//============================================================================
void LedgerInvariant::Init()
{
    // Zero-initialize elements of AllVectors and AllScalars.
    LedgerBase::Initialize(GetLength());

    irr_precision       = 0;

    EeMode              .assign(Length, mce_mode(mce_annual));
    ErMode              .assign(Length, mce_mode(mce_annual));
    DBOpt               .assign(Length, mce_dbopt(mce_option1));

    InforceYear         = Length;
    InforceMonth        = 11;

    MecYear             = Length;
    MecMonth            = 11;

    // TODO ?? Probably every member should be initialized.
    NoLapseMinDur       = 100;
    NoLapseMinAge       = 100;
    NoLapseAlwaysActive = false;
    NominallyPar        = false;
    Has1035ExchCharge   = false;

    SupplementalReport  = false;

    FullyInitialized    = false;
}

//============================================================================
void LedgerInvariant::Init(BasicValues* b)
{
    // Zero-initialize almost everything.
    Init();

    irr_precision = b->round_irr().decimals();

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

    // Certain data members, including but almost certainly not
    // limited to these, should not be initialized to any non-zero
    // value here. Actual values are inserted in account-value
    // processing, subject to various restrictions that often cause
    // them to differ from input values. Notably, values need to be
    // zero after lapse.
//    NetWD           =
//    NewCashLoan     =
//    GptForceout     =
//    NaarForceout    =
//    ProducerCompensation =

    if(b->yare_input_.TermRider)
        {
        TermSpecAmt     .assign(Length, b->yare_input_.TermRiderAmount);
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
    MonthlyFlatExtra     = b->yare_input_.FlatExtra                 ;
    HoneymoonValueSpread = b->yare_input_.HoneymoonValueSpread      ;
    AddonMonthlyFee      = b->yare_input_.ExtraMonthlyCustodialFee  ;
    AddonCompOnAssets    = b->yare_input_.ExtraCompensationOnAssets ;
    AddonCompOnPremium   = b->yare_input_.ExtraCompensationOnPremium;
    CorridorFactor       = b->GetCorridorFactor();
    CurrMandE            = b->InterestRates_->MAndERate(mce_gen_curr);
    TotalIMF             = b->InterestRates_->InvestmentManagementFee();
    RefundableSalesLoad  = b->Loads_->refundable_sales_load_proportion();

    CountryCOIMultiplier = b->yare_input_.CountryCoiMultiplier;

    CountryIso3166Abbrev = (*b->Input_)["Country"].str();
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

    PremiumTaxLoadIsTiered  = b->IsPremiumTaxLoadTiered();

    NoLapseAlwaysActive     = b->Database_->Query(DB_NoLapseAlwaysActive);
    NoLapseMinDur           = b->Database_->Query(DB_NoLapseMinDur);
    NoLapseMinAge           = b->Database_->Query(DB_NoLapseMinAge);
    NominallyPar            = b->Database_->Query(DB_NominallyPar);
    Has1035ExchCharge       = b->Database_->Query(DB_Has1035ExchCharge);

    InitBaseSpecAmt         = b->DeathBfts_->specamt()[0];
    InitTermSpecAmt         = TermSpecAmt[0];
    ChildRiderAmount        = b->yare_input_.ChildRiderAmount;
    SpouseRiderAmount       = b->yare_input_.SpouseRiderAmount;

//  InitPrem                = 0;
//  GuarPrem                = 0;
//  InitSevenPayPrem        =
//  InitTgtPrem     =

    MaleProportion          = b->yare_input_.MaleProportion;
    NonsmokerProportion     = b->yare_input_.NonsmokerProportion;
    PartMortTableMult       = b->yare_input_.PartialMortalityMultiplier;

    // Assert this because the illustration currently prints a scalar
    // guaranteed max, assuming that it's the same for all years.
    std::vector<double> const& guar_m_and_e_rate = b->InterestRates_->MAndERate
        (mce_gen_guar
        );
    LMI_ASSERT
        (each_equal
            (guar_m_and_e_rate.begin()
            ,guar_m_and_e_rate.end()
            ,guar_m_and_e_rate.front()
            )
        );
    GuarMaxMandE            = guar_m_and_e_rate[0];
//  GenderDistinct          = 0;
    GenderBlended           = b->yare_input_.BlendGender;
//  SmokerDistinct          = 0;
    SmokerBlended           = b->yare_input_.BlendSmoking;

    SubstdTable             = b->yare_input_.SubstandardTable; // Prefer string 'b->Input_->SubstandardTable'.

    Age                     = b->yare_input_.IssueAge;
    RetAge                  = b->yare_input_.RetirementAge;
    EndtAge                 = b->yare_input_.IssueAge + b->GetLength();
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
    AllowDbo3               = b->Database_->Query(DB_AllowDBO3);
    PostHoneymoonSpread     = b->yare_input_.PostHoneymoonSpread;

    // The antediluvian branch has a null ProductData_ object.
    if(b->ProductData_)
        {
        product_data const& p = *b->ProductData_;
        PolicyMktgName                 = p.datum("PolicyMktgName"                 );
        PolicyLegalName                = p.datum("PolicyLegalName"                );
        PolicyForm                     = p.datum("PolicyForm"                     );
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
        InterestDisclaimer             = p.datum("InterestDisclaimer"             );
        GuarMortalityFootnote          = p.datum("GuarMortalityFootnote"          );

        AccountValueFootnote           = p.datum("AccountValueFootnote"           );
        AttainedAgeFootnote            = p.datum("AttainedAgeFootnote"            );
        CashSurrValueFootnote          = p.datum("CashSurrValueFootnote"          );
        DeathBenefitFootnote           = p.datum("DeathBenefitFootnote"           );
        InitialPremiumFootnote         = p.datum("InitialPremiumFootnote"         );
        NetPremiumFootnote             = p.datum("NetPremiumFootnote"             );
        OutlayFootnote                 = p.datum("OutlayFootnote"                 );
        PolicyYearFootnote             = p.datum("PolicyYearFootnote"             );

        ADDFootnote                    = p.datum("ADDFootnote"                    );
        ChildFootnote                  = p.datum("ChildFootnote"                  );
        SpouseFootnote                 = p.datum("SpouseFootnote"                 );
        TermFootnote                   = p.datum("TermFootnote"                   );
        WaiverFootnote                 = p.datum("WaiverFootnote"                 );

        MinimumPremiumFootnote         = p.datum("MinimumPremiumFootnote"         );
        PremAllocationFootnote         = p.datum("PremAllocationFootnote"         );

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
        MecFootnote                    = p.datum("MecFootnote"                    );
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
        PresaleTrackingNumber          = p.datum("PresaleTrackingNumber"          );
        CompositeTrackingNumber        = p.datum("CompositeTrackingNumber"        );
        InforceTrackingNumber          = p.datum("InforceTrackingNumber"          );
        InforceCompositeTrackingNumber = p.datum("InforceCompositeTrackingNumber" );
        InforceNonGuaranteedFootnote0  = p.datum("InforceNonGuaranteedFootnote0"  );
        InforceNonGuaranteedFootnote1  = p.datum("InforceNonGuaranteedFootnote1"  );
        InforceNonGuaranteedFootnote2  = p.datum("InforceNonGuaranteedFootnote2"  );
        InforceNonGuaranteedFootnote3  = p.datum("InforceNonGuaranteedFootnote3"  );
        NonGuaranteedFootnote          = p.datum("NonGuaranteedFootnote"          );
        MonthlyChargesPaymentFootnote  = p.datum("MonthlyChargesPaymentFootnote"  );
        }

    ProducerName            = (*b->Input_)["AgentName"].str();

    std::string agent_city     = (*b->Input_)["AgentCity"   ].str();
    std::string agent_state    = (*b->Input_)["AgentState"  ].str();
    std::string agent_zip_code = (*b->Input_)["AgentZipCode"].str();
    std::string agent_city_etc(agent_city + ", " + agent_state);
    if(!agent_zip_code.empty())
        {
        agent_city_etc += " ";
        }
    agent_city_etc += agent_zip_code;

    ProducerStreet          = (*b->Input_)["AgentAddress"].str();
    ProducerCity            = agent_city_etc;
    CorpName                = (*b->Input_)["CorporationName"].str();

    Franchise               = (*b->Input_)["Franchise"].str();
    PolicyNumber            = (*b->Input_)["PolicyNumber"].str();

    Insured1                = (*b->Input_)["InsuredName"].str();
    Gender                  = (*b->Input_)["Gender"].str();
    UWType                  = (*b->Input_)["GroupUnderwritingType"].str();

    oenum_smoking_or_tobacco smoke_or_tobacco =
        static_cast<oenum_smoking_or_tobacco>
            (static_cast<int>(b->Database_->Query(DB_SmokeOrTobacco))
            );
    if(oe_tobacco_nontobacco == smoke_or_tobacco)
        {
        switch(mce_smoking(b->yare_input_.Smoking).value())
            {
            case mce_smoker:
                {
                Smoker = "Tobacco";
                }
                break;
            case mce_nonsmoker:
                {
                Smoker = "Nontobacco";
                }
                break;
            case mce_unismoke:
                {
                Smoker = "Unitobacco";
                }
                break;
            default:
                {
                fatal_error()
                    << "Case '"
                    << smoke_or_tobacco
                    << "' not found."
                    << LMI_FLUSH
                    ;
                }
                break;
            }
        }
    else if(oe_smoker_nonsmoker == smoke_or_tobacco)
        {
        Smoker = (*b->Input_)["Smoking"].str();
        }
    // TODO ?? Use a switch-statement instead. The original version of
    // this code was just if...else, and silently deemed the convention
    // to be smoker/nonsmoker if it wasn't specified as tobacco/nontobacco;
    // but if it were neither, that was silently 'fixed' in a way that's
    // not likely to be correct. If we later added 'cigarette/noncigarette',
    // which some companies use, then we would have gotten smoker/nonsmoker!
    else
        {
        throw std::logic_error("Unknown oe_smoker_nonsmoker convention.");
        }

    UWClass                 = (*b->Input_)["UnderwritingClass"].str();
    SubstandardTable        = (*b->Input_)["SubstandardTable"].str();

    EffDate                 = calendar_date(b->yare_input_.EffectiveDate).str();
    EffDateJdn              = calendar_date(b->yare_input_.EffectiveDate).julian_day_number();
    DefnLifeIns             = (*b->Input_)["DefinitionOfLifeInsurance"].str();
    DefnMaterialChange      = (*b->Input_)["DefinitionOfMaterialChange"].str();
    AvoidMec                = (*b->Input_)["AvoidMecMethod"].str();
    PartMortTableName       = "1983 GAM"; // TODO ?? Hardcoded.
    StatePostalAbbrev       = mc_str(b->GetStateOfJurisdiction());

    StatePremTaxRate        = b->PremiumTaxRate();
    // TODO ?? Output forms presuppose that the premium tax load is a
    // scalar unless it is tiered.
    StatePremTaxLoad        = b->Loads_->premium_tax_load()[0];
    LMI_ASSERT
        (PremiumTaxLoadIsTiered || each_equal
            (b->Loads_->premium_tax_load().begin()
            ,b->Loads_->premium_tax_load().end()
            ,b->Loads_->premium_tax_load().front()
            )
        );
    DacTaxPremLoadRate      = b->Loads_->dac_tax_load()[0];
    // TODO ?? Output forms presuppose that the DAC tax load is scalar;
    // and it seems odd that the DAC-tax load would have much to do
    // with whether the premium-tax is tiered.
    LMI_ASSERT
        (PremiumTaxLoadIsTiered || each_equal
            (b->Loads_->dac_tax_load().begin()
            ,b->Loads_->dac_tax_load().end()
            ,b->Loads_->dac_tax_load().front()
            )
        );
    // TODO ?? The database allows a distinct DAC tax fund charge, but
    // it seems that output forms assume that the DAC tax premium load
    // represents the entire DAC tax charge, so they're incorrect if
    // the DAC tax fund charge isn't zero.
    LMI_ASSERT(0.0 == b->Database_->Query(DB_DACTaxFundCharge));

    InitAnnLoanDueRate      = b->InterestRates_->RegLnDueRate
        (mce_gen_curr
        ,mce_annual_rate
        )[0];

    IsInforce = 0 != b->yare_input_.InforceYear || 0 != b->yare_input_.InforceMonth;

    SupplementalReport         = "Yes" == (*b->Input_)["CreateSupplementalReport"].str();
    SupplementalReportColumn00 = (*b->Input_)["SupplementalReportColumn00"].str();
    SupplementalReportColumn01 = (*b->Input_)["SupplementalReportColumn01"].str();
    SupplementalReportColumn02 = (*b->Input_)["SupplementalReportColumn02"].str();
    SupplementalReportColumn03 = (*b->Input_)["SupplementalReportColumn03"].str();
    SupplementalReportColumn04 = (*b->Input_)["SupplementalReportColumn04"].str();
    SupplementalReportColumn05 = (*b->Input_)["SupplementalReportColumn05"].str();
    SupplementalReportColumn06 = (*b->Input_)["SupplementalReportColumn06"].str();
    SupplementalReportColumn07 = (*b->Input_)["SupplementalReportColumn07"].str();
    SupplementalReportColumn08 = (*b->Input_)["SupplementalReportColumn08"].str();
    SupplementalReportColumn09 = (*b->Input_)["SupplementalReportColumn09"].str();
    SupplementalReportColumn10 = (*b->Input_)["SupplementalReportColumn10"].str();
    SupplementalReportColumn11 = (*b->Input_)["SupplementalReportColumn11"].str();

    FullyInitialized = true;
}

//============================================================================
LedgerInvariant& LedgerInvariant::PlusEq(LedgerInvariant const& a_Addend)
{
    LedgerBase::PlusEq(a_Addend, a_Addend.InforceLives);

    irr_precision = a_Addend.irr_precision;

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

    EffDate                 = a_Addend.EffDate;
    EffDateJdn              = a_Addend.EffDateJdn;
    Age                     = std::min(Age, a_Addend.Age);
    RetAge                  = std::min(RetAge, a_Addend.RetAge); // TODO ?? Does this make sense?
    EndtAge                 = std::max(EndtAge, a_Addend.EndtAge);

    DefnLifeIns             = a_Addend.DefnLifeIns;
    DefnMaterialChange      = a_Addend.DefnMaterialChange;
    AvoidMec                = a_Addend.AvoidMec;

    // TODO ?? Probably we should assert that these don't vary by life.
    CorpName                    = a_Addend.CorpName;
    Franchise                   = a_Addend.Franchise;
    ProducerName                = a_Addend.ProducerName;
    ProducerStreet              = a_Addend.ProducerStreet;
    ProducerCity                = a_Addend.ProducerCity;
    DefnLifeIns                 = a_Addend.DefnLifeIns;
    DefnMaterialChange          = a_Addend.DefnMaterialChange;
    AvoidMec                    = a_Addend.AvoidMec;

    PolicyForm                  = a_Addend.PolicyForm;
    PolicyMktgName              = a_Addend.PolicyMktgName;
    PolicyLegalName             = a_Addend.PolicyLegalName;
    InsCoShortName              = a_Addend.InsCoShortName;
    InsCoName                   = a_Addend.InsCoName;
    InsCoAddr                   = a_Addend.InsCoAddr;
    InsCoStreet                 = a_Addend.InsCoStreet;
    InsCoPhone                  = a_Addend.InsCoPhone;
    MainUnderwriter             = a_Addend.MainUnderwriter;
    MainUnderwriterAddress      = a_Addend.MainUnderwriterAddress;
    CoUnderwriter               = a_Addend.CoUnderwriter;
    CoUnderwriterAddress        = a_Addend.CoUnderwriterAddress;

    AvName                      = a_Addend.AvName;
    CsvName                     = a_Addend.CsvName;
    CsvHeaderName               = a_Addend.CsvHeaderName;
    NoLapseProvisionName        = a_Addend.NoLapseProvisionName;
    InterestDisclaimer          = a_Addend.InterestDisclaimer;
    GuarMortalityFootnote       = a_Addend.GuarMortalityFootnote;

    AccountValueFootnote        = a_Addend.AccountValueFootnote;
    AttainedAgeFootnote         = a_Addend.AttainedAgeFootnote;
    CashSurrValueFootnote       = a_Addend.CashSurrValueFootnote;
    DeathBenefitFootnote        = a_Addend.DeathBenefitFootnote;
    InitialPremiumFootnote      = a_Addend.InitialPremiumFootnote;
    NetPremiumFootnote          = a_Addend.NetPremiumFootnote;
    OutlayFootnote              = a_Addend.OutlayFootnote;
    PolicyYearFootnote          = a_Addend.PolicyYearFootnote;

    ADDFootnote                 = a_Addend.ADDFootnote;
    ChildFootnote               = a_Addend.ChildFootnote;
    SpouseFootnote              = a_Addend.SpouseFootnote;
    TermFootnote                = a_Addend.TermFootnote;
    WaiverFootnote              = a_Addend.WaiverFootnote;

    MinimumPremiumFootnote      = a_Addend.MinimumPremiumFootnote;
    PremAllocationFootnote      = a_Addend.PremAllocationFootnote;

    ProductDescription             = a_Addend.ProductDescription;
    StableValueFootnote            = a_Addend.StableValueFootnote;
    NoVanishPremiumFootnote        = a_Addend.NoVanishPremiumFootnote;
    RejectPremiumFootnote          = a_Addend.RejectPremiumFootnote;
    ExpRatingFootnote              = a_Addend.ExpRatingFootnote;
    MortalityBlendFootnote         = a_Addend.MortalityBlendFootnote;
    HypotheticalRatesFootnote      = a_Addend.HypotheticalRatesFootnote;
    SalesLoadRefundFootnote        = a_Addend.SalesLoadRefundFootnote;
    NoLapseFootnote                = a_Addend.NoLapseFootnote;
    MarketValueAdjFootnote         = a_Addend.MarketValueAdjFootnote;
    ExchangeChargeFootnote0        = a_Addend.ExchangeChargeFootnote0;
    CurrentValuesFootnote          = a_Addend.CurrentValuesFootnote;
    DBOption1Footnote              = a_Addend.DBOption1Footnote;
    DBOption2Footnote              = a_Addend.DBOption2Footnote;
    ExpRatRiskChargeFootnote       = a_Addend.ExpRatRiskChargeFootnote;
    ExchangeChargeFootnote1        = a_Addend.ExchangeChargeFootnote1;
    FlexiblePremiumFootnote        = a_Addend.FlexiblePremiumFootnote;
    GuaranteedValuesFootnote       = a_Addend.GuaranteedValuesFootnote;
    CreditingRateFootnote          = a_Addend.CreditingRateFootnote;
    MecFootnote                    = a_Addend.MecFootnote;
    MidpointValuesFootnote         = a_Addend.MidpointValuesFootnote;
    SinglePremiumFootnote          = a_Addend.SinglePremiumFootnote;
    MonthlyChargesFootnote         = a_Addend.MonthlyChargesFootnote;
    UltCreditingRateFootnote       = a_Addend.UltCreditingRateFootnote;
    MaxNaarFootnote                = a_Addend.MaxNaarFootnote;
    PremTaxSurrChgFootnote         = a_Addend.PremTaxSurrChgFootnote;
    PolicyFeeFootnote              = a_Addend.PolicyFeeFootnote;
    AssetChargeFootnote            = a_Addend.AssetChargeFootnote;
    InvestmentIncomeFootnote       = a_Addend.InvestmentIncomeFootnote;
    IrrDbFootnote                  = a_Addend.IrrDbFootnote;
    IrrCsvFootnote                 = a_Addend.IrrCsvFootnote;
    MortalityChargesFootnote       = a_Addend.MortalityChargesFootnote;
    LoanAndWithdrawalFootnote      = a_Addend.LoanAndWithdrawalFootnote;
    PresaleTrackingNumber          = a_Addend.PresaleTrackingNumber;
    CompositeTrackingNumber        = a_Addend.CompositeTrackingNumber;
    InforceTrackingNumber          = a_Addend.InforceTrackingNumber;
    InforceCompositeTrackingNumber = a_Addend.InforceCompositeTrackingNumber;
    InforceNonGuaranteedFootnote0  = a_Addend.InforceNonGuaranteedFootnote0;
    InforceNonGuaranteedFootnote1  = a_Addend.InforceNonGuaranteedFootnote1;
    InforceNonGuaranteedFootnote2  = a_Addend.InforceNonGuaranteedFootnote2;
    InforceNonGuaranteedFootnote3  = a_Addend.InforceNonGuaranteedFootnote3;
    NonGuaranteedFootnote   = a_Addend.NonGuaranteedFootnote;
    MonthlyChargesPaymentFootnote  = a_Addend.MonthlyChargesPaymentFootnote;

    Comments                    = a_Addend.Comments;

    StatePostalAbbrev           = a_Addend.StatePostalAbbrev;
    StatePremTaxLoad            = a_Addend.StatePremTaxLoad;
    StatePremTaxRate            = a_Addend.StatePremTaxRate;
    DacTaxPremLoadRate          = a_Addend.DacTaxPremLoadRate;
    InitAnnLoanDueRate          = a_Addend.InitAnnLoanDueRate;
    UseExperienceRating         = a_Addend.UseExperienceRating;
    UsePartialMort              = a_Addend.UsePartialMort;
    PartMortTableName           = a_Addend.PartMortTableName;
    GuarMaxMandE                = a_Addend.GuarMaxMandE;
    AvgFund                     = a_Addend.AvgFund;
    CustomFund                  = a_Addend.CustomFund;
    FundNumbers                 = a_Addend.FundNumbers;
    FundNames                   = a_Addend.FundNames;
    FundAllocs                  = a_Addend.FundAllocs;
    FundAllocations             = a_Addend.FundAllocations;
    GenAcctAllocation           = a_Addend.GenAcctAllocation;
    GenderDistinct              = a_Addend.GenderDistinct;
    GenderBlended               = a_Addend.GenderBlended;
    Smoker                      = a_Addend.Smoker;
    SmokerDistinct              = a_Addend.SmokerDistinct;
    SmokerBlended               = a_Addend.SmokerBlended;

    PartMortTableMult           = a_Addend.PartMortTableMult;
    TotalIMF                    = a_Addend.TotalIMF;
    RefundableSalesLoad         = a_Addend.RefundableSalesLoad;

    IsMec                       = a_Addend.IsMec        || IsMec;
    InforceIsMec                = a_Addend.InforceIsMec || InforceIsMec;

    if(InforceYear == a_Addend.InforceYear)
        {
        InforceMonth            = std::min(InforceMonth, a_Addend.InforceMonth);
        }
    else if(a_Addend.InforceYear < InforceYear)
        {
        InforceMonth            = a_Addend.InforceMonth;
        }
    InforceYear                 = std::min(InforceYear, a_Addend.InforceYear);

    if(MecYear == a_Addend.MecYear)
        {
        MecMonth                = std::min(MecMonth, a_Addend.MecMonth);
        }
    else if(a_Addend.MecYear < MecYear)
        {
        MecMonth                = a_Addend.MecMonth;
        }
    MecYear                     = std::min(MecYear, a_Addend.MecYear);

    HasWP        = HasWP        || a_Addend.HasWP        ;
    HasADD       = HasADD       || a_Addend.HasADD       ;
    HasTerm      = HasTerm      || a_Addend.HasTerm      ;

// TODO ?? Can these be meaningful on a composite? If totals are desired,
// then term should be treated the same way.
//    ChildRiderAmount   = ChildRiderAmount   || a_Addend.ChildRiderAmount  ;
//    SpouseRiderAmount  = SpouseRiderAmount  || a_Addend.SpouseRiderAmount ;

    HasChildRider      = HasChildRider      || a_Addend.HasChildRider     ;
    HasSpouseRider     = HasSpouseRider     || a_Addend.HasSpouseRider    ;

// TODO ?? For some ages, we use min; for others, max; how about this one?
//    SpouseIssueAge     =

    HasHoneymoon = HasHoneymoon || a_Addend.HasHoneymoon ;
    AllowDbo3    = AllowDbo3    || a_Addend.AllowDbo3    ;

    PremiumTaxLoadIsTiered =
            a_Addend.PremiumTaxLoadIsTiered
        ||  PremiumTaxLoadIsTiered
        ;

    NoLapseMinDur      = std::min(a_Addend.NoLapseMinDur, NoLapseMinDur);
    NoLapseMinAge      = std::min(a_Addend.NoLapseMinAge, NoLapseMinAge);
    NoLapseAlwaysActive= a_Addend.NoLapseAlwaysActive|| NoLapseAlwaysActive;
    NominallyPar       = a_Addend.NominallyPar       || NominallyPar;
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
// TODO ?? Prolly don't blong here.
void LedgerInvariant::CalculateIrrs(Ledger const& LedgerValues)
{
    int max_length = LedgerValues.GetMaxLength();

    LedgerVariant const& Curr_ = LedgerValues.GetCurrFull();
    LedgerVariant const& Guar_ = LedgerValues.GetGuarFull();
    irr
        (Outlay
        ,Guar_.CSVNet
        ,IrrCsvGuarInput
        ,static_cast<unsigned int>(Guar_.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Guar_.EOYDeathBft
        ,IrrDbGuarInput
        ,static_cast<unsigned int>(Guar_.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Curr_.CSVNet
        ,IrrCsvCurrInput
        ,static_cast<unsigned int>(Curr_.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Curr_.EOYDeathBft
        ,IrrDbCurrInput
        ,static_cast<unsigned int>(Curr_.LapseYear)
        ,max_length
        ,irr_precision
        );

    // Calculate these IRRs only for ledger types that actually use a
    // basis with a zero percent separate-account rate. This is a
    // matter not of efficiency but of validity: values for unused
    // bases are not dependably initialized.
    //
    // TODO ?? This calculation really needs to be distributed among
    // the variant ledgers, so that it gets run for every basis
    // actually used.
    //
    if
        (0 == std::count
            (LedgerValues.GetRunBases().begin()
            ,LedgerValues.GetRunBases().end()
            ,mce_run_gen_curr_sep_zero
            // Proxy for mce_run_gen_guar_sep_zero too.
            )
        )
        {
        return;
        }

    LedgerVariant const& Curr0 = LedgerValues.GetCurrZero();
    LedgerVariant const& Guar0 = LedgerValues.GetGuarZero();

    irr
        (Outlay
        ,Guar0.CSVNet
        ,IrrCsvGuar0
        ,static_cast<unsigned int>(Guar0.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Guar0.EOYDeathBft
        ,IrrDbGuar0
        ,static_cast<unsigned int>(Guar0.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Curr0.CSVNet
        ,IrrCsvCurr0
        ,static_cast<unsigned int>(Curr0.LapseYear)
        ,max_length
        ,irr_precision
        );

    irr
        (Outlay
        ,Curr0.EOYDeathBft
        ,IrrDbCurr0
        ,static_cast<unsigned int>(Curr0.LapseYear)
        ,max_length
        ,irr_precision
        );
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

