// Ledger values that do not vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_invariant.cpp,v 1.1 2005-02-12 12:59:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_invariant.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "crc32.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "financial.hpp"  // TODO ?? For IRRs--prolly don't blong here.
#include "ihs_funddata.hpp"
#include "ihs_proddata.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "ledger.hpp" // TODO ?? For IRRs--prolly don't blong here.
#include "ledger_variant.hpp" // TODO ?? For IRRs--prolly don't blong here.
#include "loads.hpp"
#include "miscellany.hpp"
#include "outlay.hpp"
#include "rounding_rules.hpp"
#include "tiered_charges.hpp"

#include <algorithm>
#include <numeric>   // std::accumulate()
#include <ostream>

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
    BegYearVectors  ["Loan"                  ] = &Loan                   ;
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
    OtherScalars    ["SubstdTable"           ] = &SubstdTable            ; // Prefer string 'SubstandardTable'.
    OtherScalars    ["Age"                   ] = &Age                    ;
    OtherScalars    ["RetAge"                ] = &RetAge                 ;
    OtherScalars    ["EndtAge"               ] = &EndtAge                ;
    OtherScalars    ["UseExperienceRating"   ] = &UseExperienceRating    ;
    OtherScalars    ["UsePartialMort"        ] = &UsePartialMort         ;
    OtherScalars    ["AvgFund"               ] = &AvgFund                ;
    OtherScalars    ["CustomFund"            ] = &CustomFund             ;
    OtherScalars    ["IsMec"                 ] = &IsMec                  ;
    OtherScalars    ["InforceIsMec"          ] = &InforceIsMec           ;
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
    OtherScalars    ["PremiumTaxIsTiered"    ] = &PremiumTaxIsTiered     ;
    OtherScalars    ["NoLapseAlwaysActive"   ] = &NoLapseAlwaysActive    ;
    OtherScalars    ["NoLapseMinDur"         ] = &NoLapseMinDur          ;
    OtherScalars    ["NoLapseMinAge"         ] = &NoLapseMinAge          ;
    OtherScalars    ["NominallyPar"          ] = &NominallyPar           ;
    OtherScalars    ["Has1035ExchCharge"     ] = &Has1035ExchCharge      ;
    OtherScalars    ["EffDateJdn"            ] = &EffDateJdn             ;
    OtherScalars    ["GenAcctAllocation"     ] = &GenAcctAllocation      ;
    OtherScalars    ["SupplementalReport"    ] = &SupplementalReport     ;

    Strings         ["PolicyShortName"       ] = &PolicyShortName        ;
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

    Strings         ["ProducerName"         ] = &ProducerName           ;
    Strings         ["ProducerStreet"       ] = &ProducerStreet         ;
    Strings         ["ProducerCity"         ] = &ProducerCity           ;
    Strings         ["CorpName"             ] = &CorpName               ;
    Strings         ["CertificateNumber"    ] = &CertificateNumber      ;
    Strings         ["PolicyNumber"         ] = &PolicyNumber           ;
    Strings         ["Insured1"             ] = &Insured1               ;
    Strings         ["Gender"               ] = &Gender                 ;
    Strings         ["UWType"               ] = &UWType                 ;
    Strings         ["Smoker"               ] = &Smoker                 ;
    Strings         ["UWClass"              ] = &UWClass                ;
    Strings         ["SubstandardTable"     ] = &SubstandardTable       ;
    Strings         ["DefnLifeIns"          ] = &DefnLifeIns            ;
    Strings         ["DefnMaterialChange"   ] = &DefnMaterialChange     ;
    Strings         ["AvoidMec"             ] = &AvoidMec               ;
    Strings         ["PartMortTableName"    ] = &PartMortTableName      ;
    Strings         ["StatePostalAbbrev"    ] = &StatePostalAbbrev      ;
    Strings         ["CountryIso3166Abbrev" ] = &CountryIso3166Abbrev   ;
    Strings         ["Comments"             ] = &Comments               ;

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
    // it can hold both BOY and EOY values for all years.
    InforceLives        .assign(1 + Length, 1.0);

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
    TieredSepAcctLoadBands = obj.TieredSepAcctLoadBands;
    TieredSepAcctLoadRates = obj.TieredSepAcctLoadRates;

    // Scalars of type not compatible with double.
    EffDate                = obj.EffDate               ;

    // TODO ?? Kludge to meet a meaningless requirement.
    OffersRiders_          = obj.OffersRiders_         ;

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

    EeMode              .assign(Length, e_mode(e_annual));
    ErMode              .assign(Length, e_mode(e_annual));
    DBOpt               .assign(Length, e_dbopt(e_option1));

    MecYear             = Length;
    MecMonth            = 11;

    // TODO ?? Probably every member should be initialized.
    NoLapseMinDur       = 100;
    NoLapseMinAge       = 100;
    NoLapseAlwaysActive = false;
    NominallyPar        = false;
    Has1035ExchCharge   = false;
// TODO ?? Kludge to meet a meaningless requirement.
    OffersRiders_       = false;

    SupplementalReport  = false;

    FullyInitialized    = false;
}

//============================================================================
void LedgerInvariant::Init(BasicValues* b)
{
    // Zero-initialize almost everything.
    Init();

    irr_precision = b->GetRoundingRules().round_irr().decimals();

    InputParms const& Input     = *b->Input;
    InputStatus const& Status   = Input.Status[0]; // TODO ?? Based on first life only.

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

    NetWD           = b->Outlay_->withdrawals();
    Loan            = b->Outlay_->new_cash_loans();

//    GptForceout     =
//    NaarForceout    =
//    ProducerCompensation =

    if(Status.HasTerm)
        {
        TermSpecAmt     .assign(Length, Status.TermAmt);
        }
    else
        {
        TermSpecAmt     .assign(Length, 0.0);
        }
    SpecAmt         = b->DeathBfts_->specamt();
    EeMode          = b->Outlay_->ee_premium_modes();
    ErMode          = b->Outlay_->er_premium_modes();
    DBOpt           = b->DeathBfts_->dbopt();

    IndvTaxBracket       = Input.VectorIndvTaxBracket       ;
    CorpTaxBracket       = Input.VectorCorpTaxBracket       ;
    Salary               = Input.Salary                     ;
    MonthlyFlatExtra     = Status.VectorMonthlyFlatExtra    ;
    HoneymoonValueSpread = Input.VectorHoneymoonValueSpread ;

    AddonMonthlyFee      = Input.VectorAddonMonthlyCustodialFee;
    AddonCompOnAssets    = Input.VectorAddonCompOnAssets;
    AddonCompOnPremium   = Input.VectorAddonCompOnPremium;
    CorridorFactor       = b->GetCorridorFactor();
    CurrMandE            = b->InterestRates_->MAndERate(e_basis(e_currbasis));
    TotalIMF             = b->InterestRates_->InvestmentManagementFee();
    RefundableSalesLoad  = b->Loads_->refundable_sales_load_proportion();

    CountryCOIMultiplier = Input.CountryCOIMultiplier;

    CountryIso3166Abbrev = Input.Country.str();
    Comments             = Input.Comments;

    FundNumbers           .resize(0);
    FundNames             .resize(0);
    FundAllocs            .resize(0);
    FundAllocations       .resize(0);
    TieredSepAcctLoadBands.resize(0);
    TieredSepAcctLoadRates.resize(0);

    // The antediluvian branch has a null FundData object.
    int number_of_funds(0);
    if(b->FundData)
        {
        number_of_funds = b->FundData->GetNumberOfFunds();
        }
    for(int j = 0; j < number_of_funds; j++)
        {
        FundNumbers.push_back(j);
        FundNames.push_back(b->FundData->GetFundInfo(j).LongName_);

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

        FundAllocs.push_back
            ((j < b->Input->NumberOfFunds)
            ? (b->Input->FundAllocs[j].operator const int&())
            : 0
            );

        FundAllocations.push_back
            ((j < b->Input->NumberOfFunds)
            ? .01 * (b->Input->FundAllocs[j].operator const int&())
            : 0.0
            );
        }

    GenAcctAllocation = 1.0 - std::accumulate
        (FundAllocations.begin()
        ,FundAllocations.end()
        ,0.0
        );

    // Default initialization to 1.0 is OK: class AccountValue assigns
    // yearly values reflecting partial mortality.
//  InforceLives =

    // TODO ?? It is somewhat unusual that this 'invariant' class
    // includes this item that varies by basis. Perhaps it should be
    // in the complementary 'variant' class. But apparently the
    // guaranteed version of this load isn't implemented.

    // The antediluvian branch has a null TieredCharges_ object.
    if(b->TieredCharges_)
        {
        // TRICKY !! This const reference is required for overload
        // resolution to choose the const version of
        // tiered_charges::tiered_item().
        //
        tiered_charges const& x(*b->TieredCharges_);
        tiered_item_rep const& tiered_sep_acct_load
            (x.tiered_item(tiered_charges::e_tier_current_separate_account_load));

        TieredSepAcctLoadBands = tiered_sep_acct_load.bands();
        TieredSepAcctLoadRates = tiered_sep_acct_load.data();

        PremiumTaxIsTiered = x.premium_tax_is_tiered(b->GetStateOfJurisdiction());
        }
    else
        {
        PremiumTaxIsTiered = false;
        }

    NoLapseAlwaysActive     = b->Database->Query(DB_NoLapseAlwaysActive);
    NoLapseMinDur           = b->Database->Query(DB_NoLapseMinDur);
    NoLapseMinAge           = b->Database->Query(DB_NoLapseMinAge);
    NominallyPar            = b->Database->Query(DB_NominallyPar);
    Has1035ExchCharge       = b->Database->Query(DB_Has1035ExchCharge);

    InitBaseSpecAmt         = b->DeathBfts_->specamt()[0];
    InitTermSpecAmt         = TermSpecAmt[0];
    ChildRiderAmount        = Input.ChildRiderAmount;
    SpouseRiderAmount       = Input.SpouseRiderAmount;

//  InitPrem                = 0;
//  GuarPrem                = 0;
//  InitSevenPayPrem        =
//  InitTgtPrem     =

    MaleProportion          = Input.MaleProportion;
    NonsmokerProportion     = Input.NonsmokerProportion;
    PartMortTableMult       = Input.VectorPartialMortalityMultiplier;

    // Assert this because the illustration currently prints a scalar
    // guaranteed max, assuming that it's the same for all years.
    std::vector<double> const& guar_m_and_e_rate = b->InterestRates_->MAndERate
        (e_basis(e_guarbasis)
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
    GenderBlended           = Input.BlendMortGender;
//  SmokerDistinct          = 0;
    SmokerBlended           = Input.BlendMortSmoking;

    SubstdTable             = Status.SubstdTable; // Prefer string 'SubstandardTable'.

    Age                     = Status.IssueAge;
    RetAge                  = Status.RetAge;
    EndtAge                 = Status.IssueAge + b->GetLength();
    UseExperienceRating     = Input.UseExperienceRating;
    UsePartialMort          = Input.UsePartialMort;
    AvgFund                 = Input.AvgFund;
    CustomFund              = Input.OverrideFundMgmtFee;

    HasWP                   = Status.HasWP;
    HasADD                  = Status.HasADD;
    HasTerm                 = Status.HasTerm;

    HasChildRider           = Input.HasChildRider;
    HasSpouseRider          = Input.HasSpouseRider;
    SpouseIssueAge          = Input.SpouseIssueAge;

    HasHoneymoon            = Input.HasHoneymoon;
    AllowDbo3               = b->Database->Query(DB_AllowDBO3);
    PostHoneymoonSpread     = Input.PostHoneymoonSpread;

    // The antediluvian branch has a null ProductData object.
    if(b->ProductData)
        {
        PolicyMktgName         = b->ProductData->GetPolicyMktgName();
        PolicyLegalName        = b->ProductData->GetPolicyLegalName();
        PolicyForm             = b->ProductData->GetPolicyForm();
        InsCoShortName         = b->ProductData->GetInsCoShortName();
        InsCoName              = b->ProductData->GetInsCoName();
        InsCoAddr              = b->ProductData->GetInsCoAddr();
        InsCoStreet            = b->ProductData->GetInsCoStreet();
        InsCoPhone             = b->ProductData->GetInsCoPhone();
        MainUnderwriter        = b->ProductData->GetMainUnderwriter();
        MainUnderwriterAddress = b->ProductData->GetMainUnderwriterAddress();
        CoUnderwriter          = b->ProductData->GetCoUnderwriter();
        CoUnderwriterAddress   = b->ProductData->GetCoUnderwriterAddress();

        AvName                 = b->ProductData->GetAvName();
        CsvName                = b->ProductData->GetCsvName();
        CsvHeaderName          = b->ProductData->GetCsvHeaderName();
        NoLapseProvisionName   = b->ProductData->GetNoLapseProvisionName();
        InterestDisclaimer     = b->ProductData->GetInterestDisclaimer();
        }

    ProducerName            = Input.AgentFullName();

    ProducerStreet  =
            Input.AgentAddr1
        ;
    ProducerCity    =
            Input.AgentCity
        +   ", "
        +   Input.AgentState.str()
        +   " "
        +   Input.AgentZipCode
        ;
    CorpName                = Input.SponsorFirstName;
//  CertificateNumber       =
//  PolicyNumber            =

    Insured1                = Input.InsdFullName();
    Gender                  = Status.Gender.str();
    UWType                  = Input.GroupUWType.str();

    e_smoking_or_tobacco smoke_or_tobacco =
        static_cast<e_smoking_or_tobacco>
            (static_cast<int>(b->Database->Query(DB_SmokeOrTobacco))
            );
    if(e_tobacco_nontobacco == smoke_or_tobacco)
        {
            switch(e_smoking(Status.Smoking))
            {
            case e_smoker:
                {
                Smoker = "Tobacco";
                }
                break;
            case e_nonsmoker:
                {
                Smoker = "Nontobacco";
                }
                break;
            case e_unismoke:
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
    else if(e_smoker_nonsmoker == smoke_or_tobacco)
        {
        Smoker                  = Status.Smoking.str();
        }
    // TODO ?? Use a switch-statement instead. The original version of
    // this code was just if...else, and silently deemed the convention
    // to be smoker/nonsmoker if it wasn't specified as tobacco/nontobacco;
    // but if it were neither, that was silently 'fixed' in a way that's
    // not likely to be correct. If we later added 'cigarette/noncigarette',
    // which some companies use, then we would have gotten smoker/nonsmoker!
    else
        {
        throw std::logic_error("Unknown e_smoker_nonsmoker convention.");
        }

    UWClass                 = Status.Class.str();
    SubstandardTable        = Status.SubstdTable.str();

    EffDate                 = calendar_date(Input.EffDate).str();
    EffDateJdn              = calendar_date(Input.EffDate).julian_day_number();
    DefnLifeIns             = Input.DefnLifeIns.str();
    DefnMaterialChange      = Input.DefnMaterialChange.str();
    AvoidMec                = Input.AvoidMec.str();
    PartMortTableName       = "1983 GAM"; // TODO ?? Hardcoded.
    StatePostalAbbrev       = b->GetStateOfJurisdiction().str();

    StatePremTaxRate        = b->GetPremTaxRate();
    // TODO ?? Output forms presuppose that the premium tax load is a
    // scalar unless it is tiered.
    StatePremTaxLoad        = b->Loads_->premium_tax_load()[0];
    LMI_ASSERT
        (PremiumTaxIsTiered || each_equal
            (b->Loads_->premium_tax_load().begin()
            ,b->Loads_->premium_tax_load().end()
            ,b->Loads_->premium_tax_load().front()
            )
        );
    DacTaxPremLoadRate      = b->Loads_->dac_tax_load()[0];
    // TODO ?? Output forms presuppose that the DAC tax load is scalar.
    LMI_ASSERT
        (PremiumTaxIsTiered || each_equal
            (b->Loads_->dac_tax_load().begin()
            ,b->Loads_->dac_tax_load().end()
            ,b->Loads_->dac_tax_load().front()
            )
        );
    // TODO ?? The database allows a distinct DAC tax fund charge, but
    // it seems that output forms assume that the DAC tax premium load
    // represents the entire DAC tax charge, so they're incorrect if
    // the DAC tax fund charge isn't zero.
    LMI_ASSERT(0.0 == b->Database->Query(DB_DACTaxFundCharge));

    InitAnnLoanDueRate      = b->InterestRates_->RegLnDueRate
        (e_basis(e_currbasis)
        ,e_rate_period(e_annual_rate)
        )[0];

    IsInforce               = 0 != Input.InforceYear || 0 != Input.InforceMonth;

// TODO ?? Kludge to meet a meaningless requirement.
    OffersRiders_ =
            b->Database->Query(DB_AllowWP)
        ||  b->Database->Query(DB_AllowADD)
        ||  b->Database->Query(DB_AllowSpouse)
        ||  b->Database->Query(DB_AllowChild)
        ;

    SupplementalReport         = Input.CreateSupplementalReport  ;
    SupplementalReportColumn00 = Input.SupplementalReportColumn00;
    SupplementalReportColumn01 = Input.SupplementalReportColumn01;
    SupplementalReportColumn02 = Input.SupplementalReportColumn02;
    SupplementalReportColumn03 = Input.SupplementalReportColumn03;
    SupplementalReportColumn04 = Input.SupplementalReportColumn04;
    SupplementalReportColumn05 = Input.SupplementalReportColumn05;
    SupplementalReportColumn06 = Input.SupplementalReportColumn06;
    SupplementalReportColumn07 = Input.SupplementalReportColumn07;
    SupplementalReportColumn08 = Input.SupplementalReportColumn08;
    SupplementalReportColumn09 = Input.SupplementalReportColumn09;
    SupplementalReportColumn10 = Input.SupplementalReportColumn10;
    SupplementalReportColumn11 = Input.SupplementalReportColumn11;

    FullyInitialized = true;
}

//============================================================================
LedgerInvariant& LedgerInvariant::PlusEq(LedgerInvariant const& a_Addend)
{
    LedgerBase::PlusEq(a_Addend, a_Addend.InforceLives);

    irr_precision = a_Addend.irr_precision;

    std::vector<double> const& N = a_Addend.InforceLives;
    int Max = std::min(Length, a_Addend.Length);

// TODO ?? STL offers better ways to do this.
    // Make sure total (this) has enough years to add all years of a_Addend to
    LMI_ASSERT(Length >= a_Addend.Length);
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
    TieredSepAcctLoadBands      = a_Addend.TieredSepAcctLoadBands;
    TieredSepAcctLoadRates      = a_Addend.TieredSepAcctLoadRates;
    GenderDistinct              = a_Addend.GenderDistinct;
    GenderBlended               = a_Addend.GenderBlended;
    Smoker                      = a_Addend.Smoker;
    SmokerDistinct              = a_Addend.SmokerDistinct;
    SmokerBlended               = a_Addend.SmokerBlended;

    RefundableSalesLoad         = a_Addend.RefundableSalesLoad;

    IsMec                       = a_Addend.IsMec        || IsMec;
    InforceIsMec                = a_Addend.InforceIsMec || InforceIsMec;
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

    PremiumTaxIsTiered = a_Addend.PremiumTaxIsTiered || PremiumTaxIsTiered;

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

// TODO ?? Kludge to meet a meaningless requirement.
    OffersRiders_ = OffersRiders_ || a_Addend.OffersRiders_;

    // TODO ?? This doesn't seem quite right, but what would be better?
    // We can't take the union of all columns selected for any life,
    // because its cardinality might exceed the maximum.
    SupplementalReport  = SupplementalReport || a_Addend.SupplementalReport;
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
            ,e_run_curr_basis_sa_zero
            // Proxy for e_run_guar_basis_sa_zero too.
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
    a_crc += EeMode;
    a_crc += ErMode;
    a_crc += DBOpt;
    a_crc += FundNumbers;
    a_crc += FundNames;
    a_crc += FundAllocs;
    a_crc += FundAllocations;
    a_crc += TieredSepAcctLoadBands;
    a_crc += TieredSepAcctLoadRates;
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
    SpewVector(os, std::string("TieredSepAcctLoadBands") ,TieredSepAcctLoadBands);
    SpewVector(os, std::string("TieredSepAcctLoadRates") ,TieredSepAcctLoadRates);
}

