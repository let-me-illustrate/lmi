// Product-database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef dbdict_hpp
#define dbdict_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "cache_file_reads.hpp"
#include "dbvalue.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

/// Cached product database.

class LMI_SO DBDictionary
    :public xml_serializable  <DBDictionary>
    ,public MemberSymbolTable <DBDictionary>
    ,public cache_file_reads  <DBDictionary>
{
    friend class DatabaseDocument;
    friend class input_test;        // For test_product_database().
    friend class premium_tax_test;  // For test_rates().

  public:
    DBDictionary();
    explicit DBDictionary(fs::path const& filename);

    ~DBDictionary() override = default;

    database_entity const& datum(std::string const&) const;

    static void write_database_files();
    static void write_proprietary_database_files();

    void InitAntediluvian();

  protected:
    void Add(database_entity const&);

  private:
    DBDictionary(DBDictionary const&) = delete;
    DBDictionary& operator=(DBDictionary const&) = delete;

    void Init(std::string const& filename);

    void ascribe_members();

    database_entity& datum(std::string const&);

    void WriteDB(std::string const& filename) const;
    void InitDB();

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        ) override;
    void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const override;
    void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_leaf_name
        ) const override;

    // To make sure these members match e_database_key enumerators:
    //   <dbdict.hpp sed -e '/database_entity [A-Z]/!d;s/    database_entity //;s/ *;$//' >eraseme0
    //   <dbnames.hpp sed -e '/        ,DB_/!d;s/        ,DB_//' >eraseme1
    // and compare the resulting temporary files.

    database_entity MinIssAge                 ;
    database_entity MaxIssAge                 ;
    database_entity MaxIncrAge                ;
    database_entity AllowFullUw               ;
    database_entity AllowParamedUw            ;
    database_entity AllowNonmedUw             ;
    database_entity AllowSimpUw               ;
    database_entity AllowGuarUw               ;
    database_entity SmokeOrTobacco            ;
    database_entity AllowPreferredClass       ;
    database_entity AllowUltraPrefClass       ;
    database_entity AllowSubstdTable          ;
    database_entity AllowFlatExtras           ;
    database_entity AllowRatedWp              ;
    database_entity AllowRatedAdb             ;
    database_entity AllowRatedTerm            ;
    database_entity AllowRetirees             ;
    database_entity AllowUnisex               ;
    database_entity AllowSexDistinct          ;
    database_entity AllowUnismoke             ;
    database_entity AllowSmokeDistinct        ;
    database_entity StateApproved             ;
    database_entity AllowStateXX              ;
    database_entity AllowForeign              ;
    database_entity GroupIndivSelection       ;
    database_entity TxCallsGuarUwSubstd       ;
    database_entity Allowable                 ;
    database_entity AllowCvat                 ;
    database_entity AllowGpt                  ;
    database_entity AllowNo7702               ;
    database_entity AnnInterestRate7702       ;
    database_entity IgnoreLoanRateFor7702     ;
    database_entity MlyDiscountRate7702Whence ;
    database_entity MlyDiscountRate7702       ;
    database_entity Irc7702Obreption          ;
    database_entity CorridorWhence            ;
    database_entity Irc7702NspWhence          ;
    database_entity SevenPayWhence            ;
    database_entity Irc7702QWhence            ;
    database_entity CorridorTable             ;
    database_entity Irc7702NspTable           ;
    database_entity SevenPayTable             ;
    database_entity Irc7702QTable             ;
    database_entity Irc7702QAxisGender        ;
    database_entity Irc7702QAxisSmoking       ;
    database_entity RatingsAffect7702         ;
    database_entity CvatMatChangeDefn         ;
    database_entity GptMatChangeDefn          ;
    database_entity Irc7702BftIsSpecAmt       ;
    database_entity RiskyInitial7702Db        ;
    database_entity Irc7702Endowment          ;
    database_entity Effective7702DboRop       ;
    database_entity TermIsQABOrDb7702         ;
    database_entity TermIsQABOrDb7702A        ;
    database_entity GioIsQAB                  ;
    database_entity AdbIsQAB                  ;
    database_entity SpouseRiderIsQAB          ;
    database_entity ChildRiderIsQAB           ;
    database_entity WpIsQAB                   ;
    database_entity CsoEra                    ;
    database_entity CsoMisprint               ;
    database_entity GuarCoiWhence             ;
    database_entity GuarCoiTable              ;
    database_entity GuarCoiIsAnnual           ;
    database_entity GuarCoiMultiplier         ;
    database_entity CurrCoiTable              ;
    database_entity CurrCoiIsAnnual           ;
    database_entity MinInputCoiMult           ;
    database_entity CurrCoiMultiplier         ;
    database_entity UnusualCoiBanding         ;
    database_entity CurrCoiTable0Limit        ;
    database_entity CurrCoiTable1             ;
    database_entity CurrCoiTable1Limit        ;
    database_entity CurrCoiTable2             ;
    database_entity MdptCoiTable              ;
    database_entity MdptCoiIsAnnual           ;
    database_entity CoiNyMinTable             ;
    database_entity UseNyCoiFloor             ;
    database_entity MaxMonthlyCoiRate         ;
    database_entity GuarCoiCeiling            ;
    database_entity CoiGuarIsMin              ;
    database_entity SubstdTableMult           ;
    database_entity SubstdTableMultTable      ;
    database_entity CoiUpper12Method          ;
    database_entity CoiInforceReentry         ;
    database_entity CoiResetMinDate           ;
    database_entity CoiResetMaxDate           ;
    database_entity AllowMortBlendSex         ;
    database_entity AllowMortBlendSmoke       ;
    database_entity GuarInt                   ;
    database_entity NaarDiscount              ;
    database_entity GuarIntSpread             ;
    database_entity GuarMandE                 ;
    database_entity CurrIntSpread             ;
    database_entity CurrMandE                 ;
    database_entity GenAcctIntBonus           ;
    database_entity BonusInt                  ;
    database_entity IntFloor                  ;
    database_entity AllowGenAcct              ;
    database_entity AllowSepAcct              ;
    database_entity AllowGenAcctEarnRate      ;
    database_entity AllowSepAcctNetRate       ;
    database_entity MaxGenAcctRate            ;
    database_entity MaxSepAcctRate            ;
    database_entity SepAcctSpreadMethod       ;
    database_entity IntSpreadMode             ;
    database_entity DynamicMandE              ;
    database_entity AllowAmortPremLoad        ;
    database_entity LoadAmortFundCharge       ;
    database_entity AllowImfOverride          ;
    database_entity AssetChargeType           ;
    database_entity GuarFundAdminChg          ;
    database_entity CurrFundAdminChg          ;
    database_entity FundCharge                ;
    database_entity GuarMonthlyPolFee         ;
    database_entity GuarAnnualPolFee          ;
    database_entity GuarPremLoadTgt           ;
    database_entity GuarPremLoadExc           ;
    database_entity GuarPremLoadTgtRfd        ;
    database_entity GuarPremLoadExcRfd        ;
    database_entity GuarSpecAmtLoad           ;
    database_entity GuarSpecAmtLoadTable      ;
    database_entity GuarAcctValLoad           ;
    database_entity CurrMonthlyPolFee         ;
    database_entity CurrAnnualPolFee          ;
    database_entity CurrPremLoadTgt           ;
    database_entity CurrPremLoadExc           ;
    database_entity CurrPremLoadTgtRfd        ;
    database_entity CurrPremLoadExcRfd        ;
    database_entity CurrSpecAmtLoad           ;
    database_entity CurrSpecAmtLoadTable      ;
    database_entity CurrAcctValLoad           ;
    database_entity TgtPremMonthlyPolFee      ;
    database_entity LoadRfdProportion         ;
    database_entity SpecAmtLoadLimit          ;
    database_entity DynamicSepAcctLoad        ;
    database_entity DynSepAcctLoadLimit       ;
    database_entity DacTaxFundCharge          ;
    database_entity DacTaxPremLoad            ;
    database_entity PremTaxFundCharge         ;
    database_entity PremTaxLoad               ;
    database_entity WaivePremTaxInt1035       ;
    database_entity PremTaxRetalLimit         ;
    database_entity PremTaxTierGroup          ;
    database_entity PremTaxTierPeriod         ;
    database_entity PremTaxTierNonDecr        ;
    database_entity PremTaxAmortPeriod        ;
    database_entity PremTaxAmortIntRate       ;
    database_entity PremTaxRate               ;
    database_entity PremTaxState              ;
    database_entity SurrChgOnIncr             ;
    database_entity SurrChgOnDecr             ;
    database_entity Has1035ExchCharge         ;
    database_entity SnflQTable                ;
    database_entity CoiSnflIsGuar             ;
    database_entity SurrChgByFormula          ;
    database_entity SurrChgPeriod             ;
    database_entity SurrChgZeroDur            ;
    database_entity SurrChgNlpMult            ;
    database_entity SurrChgNlpMax             ;
    database_entity SurrChgEaMax              ;
    database_entity SurrChgAmort              ;
    database_entity AllowSpecAmtIncr          ;
    database_entity MinSpecAmtIncr            ;
    database_entity EnforceNaarLimit          ;
    database_entity MinSpecAmt                ;
    database_entity MinIssSpecAmt             ;
    database_entity MinIssBaseSpecAmt         ;
    database_entity MinRenlSpecAmt            ;
    database_entity MinRenlBaseSpecAmt        ;
    database_entity MaxIssSpecAmt             ;
    database_entity MaxRenlSpecAmt            ;
    database_entity AllowDboLvl               ;
    database_entity AllowDboInc               ;
    database_entity AllowDboRop               ;
    database_entity AllowDboMdb               ;
    database_entity DboLvlChangeToWhat        ;
    database_entity DboLvlChangeMethod        ;
    database_entity DboIncChangeToWhat        ;
    database_entity DboIncChangeMethod        ;
    database_entity DboRopChangeToWhat        ;
    database_entity DboRopChangeMethod        ;
    database_entity DboMdbChangeToWhat        ;
    database_entity DboMdbChangeMethod        ;
    database_entity AllowChangeToDbo2         ;
    database_entity DboChgCanIncrSpecAmt      ;
    database_entity DboChgCanDecrSpecAmt      ;
    database_entity AllowExtEndt              ;
    database_entity AllowTerm                 ;
    database_entity GuarTermTable             ;
    database_entity TermTable                 ;
    database_entity TermMinIssAge             ;
    database_entity TermMaxIssAge             ;
    database_entity TermIsNotRider            ;
    database_entity TermCanLapse              ;
    database_entity TermForcedConvAge         ;
    database_entity TermForcedConvDur         ;
    database_entity MaxTermProportion         ;
    database_entity TermCoiRate               ;
    database_entity TermPremRate              ;
    database_entity AllowWp                   ;
    database_entity WpTable                   ;
    database_entity WpMinIssAge               ;
    database_entity WpMaxIssAge               ;
    database_entity WpLimit                   ;
    database_entity WpCoiRate                 ;
    database_entity WpPremRate                ;
    database_entity WpChargeMethod            ;
    database_entity AllowAdb                  ;
    database_entity AdbTable                  ;
    database_entity AdbMinIssAge              ;
    database_entity AdbMaxIssAge              ;
    database_entity AdbLimit                  ;
    database_entity AdbCoiRate                ;
    database_entity AdbPremRate               ;
    database_entity AllowSpouseRider          ;
    database_entity SpouseRiderMinAmt         ;
    database_entity SpouseRiderMaxAmt         ;
    database_entity SpouseRiderMinIssAge      ;
    database_entity SpouseRiderMaxIssAge      ;
    database_entity SpouseRiderGuarTable      ;
    database_entity SpouseRiderTable          ;
    database_entity AllowChildRider           ;
    database_entity ChildRiderMinAmt          ;
    database_entity ChildRiderMaxAmt          ;
    database_entity ChildRiderTable           ;
    database_entity AllowWd                   ;
    database_entity WdFee                     ;
    database_entity WdFeeRate                 ;
    database_entity FreeWdProportion          ;
    database_entity MinWd                     ;
    database_entity MaxWdGenAcctValMult       ;
    database_entity MaxWdSepAcctValMult       ;
    database_entity MaxWdDed                  ;
    database_entity WdDecrSpecAmtDboLvl       ;
    database_entity WdDecrSpecAmtDboInc       ;
    database_entity WdDecrSpecAmtDboRop       ;
    database_entity FirstWdMonth              ;
    database_entity AllowLoan                 ;
    database_entity AllowPrefLoan             ;
    database_entity AllowFixedLoan            ;
    database_entity AllowVlr                  ;
    database_entity FixedLoanRate             ;
    database_entity MaxVlrRate                ;
    database_entity MinVlrRate                ;
    database_entity MaxLoanAcctValMult        ;
    database_entity MaxLoanDed                ;
    database_entity FirstPrefLoanYear         ;
    database_entity PrefLoanRateDecr          ;
    database_entity GuarPrefLoanSpread        ;
    database_entity GuarRegLoanSpread         ;
    database_entity CurrPrefLoanSpread        ;
    database_entity CurrRegLoanSpread         ;
    database_entity FirstLoanMonth            ;
    database_entity MinPremType               ;
    database_entity MinPremTable              ;
    database_entity MinPremIntSpread          ;
    database_entity SplitMinPrem              ;
    database_entity UnsplitSplitMinPrem       ;
    database_entity ErNotionallyPaysTerm      ;
    database_entity TgtPremType               ;
    database_entity TgtPremTable              ;
    database_entity TgtPremFixedAtIssue       ;
    database_entity TgtPremIgnoreSubstd       ;
    database_entity MinPmt                    ;
    database_entity IsSinglePremium           ;
    database_entity NoLapseMinDur             ;
    database_entity NoLapseMinAge             ;
    database_entity NoLapseUnratedOnly        ;
    database_entity NoLapseDboLvlOnly         ;
    database_entity NoLapseAlwaysActive       ;
    database_entity AllowHoneymoon            ;
    database_entity DeductionMethod           ;
    database_entity DeductionAcct             ;
    database_entity DistributionMethod        ;
    database_entity DistributionAcct          ;
    database_entity EePremMethod              ;
    database_entity EePremAcct                ;
    database_entity ErPremMethod              ;
    database_entity ErPremAcct                ;
    database_entity CalculateComp             ;
    database_entity CompTarget                ;
    database_entity CompExcess                ;
    database_entity CompChargeback            ;
    database_entity AssetComp                 ;
    database_entity AllowExtraAssetComp       ;
    database_entity AllowExtraPremComp        ;
    database_entity LedgerType                ;
    database_entity Nonillustrated            ;
    database_entity NoLongerIssued            ;
    database_entity AgeLastOrNearest          ;
    database_entity MaturityAge               ;
    database_entity AllowCashValueEnh         ;
    database_entity CashValueEnhMult          ;
    database_entity LapseIgnoresSurrChg       ;
    database_entity DefaultProcessOrder       ;
    database_entity GroupProxyRateTable       ;
    // SOMEDAY !! This support request:
    //   https://savannah.nongnu.org/support/?105593
    // would move the choice of partial mortality table to the GUI and
    // make this entity superfluous. 'GroupProxyRateTable' could be
    // treated the same way.
    database_entity PartialMortTable          ;
    database_entity AllowGroupQuote           ;
    database_entity PolicyForm                ;
    database_entity PolicyMktgName            ;
    database_entity PolicyLegalName           ;
    database_entity InsCoShortName            ;
    database_entity InsCoName                 ;
    database_entity InsCoAddr                 ;
    database_entity InsCoStreet               ;
    database_entity InsCoPhone                ;
    database_entity MainUnderwriter           ;
    database_entity MainUnderwriterAddress    ;
    database_entity CoUnderwriter             ;
    database_entity CoUnderwriterAddress      ;
    database_entity AvName                    ;
    database_entity CsvName                   ;
    database_entity CsvHeaderName             ;
    database_entity NoLapseProvisionName      ;
    database_entity ContractName              ;
    database_entity DboName                   ;
    database_entity DboNameLevel              ;
    database_entity DboNameIncreasing         ;
    database_entity DboNameReturnOfPremium    ;
    database_entity DboNameMinDeathBenefit    ;
    database_entity GenAcctName               ;
    database_entity GenAcctNameElaborated     ;
    database_entity SepAcctName               ;
    database_entity SpecAmtName               ;
    database_entity SpecAmtNameElaborated     ;
    database_entity UwBasisMedical            ;
    database_entity UwBasisParamedical        ;
    database_entity UwBasisNonmedical         ;
    database_entity UwBasisSimplified         ;
    database_entity UwBasisGuaranteed         ;
    database_entity UwClassPreferred          ;
    database_entity UwClassStandard           ;
    database_entity UwClassRated              ;
    database_entity UwClassUltra              ;
    database_entity AccountValueFootnote      ;
    database_entity AttainedAgeFootnote       ;
    database_entity CashSurrValueFootnote     ;
    database_entity DeathBenefitFootnote      ;
    database_entity InitialPremiumFootnote    ;
    database_entity NetPremiumFootnote        ;
    database_entity GrossPremiumFootnote      ;
    database_entity OutlayFootnote            ;
    database_entity PolicyYearFootnote        ;
    database_entity ADDTerseName              ;
    database_entity InsurabilityTerseName     ;
    database_entity ChildTerseName            ;
    database_entity SpouseTerseName           ;
    database_entity TermTerseName             ;
    database_entity WaiverTerseName           ;
    database_entity AccelBftRiderTerseName    ;
    database_entity OverloanRiderTerseName    ;
    database_entity ADDFootnote               ;
    database_entity ChildFootnote             ;
    database_entity SpouseFootnote            ;
    database_entity TermFootnote              ;
    database_entity WaiverFootnote            ;
    database_entity AccelBftRiderFootnote     ;
    database_entity OverloanRiderFootnote     ;
    database_entity GroupQuoteShortProductName;
    database_entity GroupQuoteIsNotAnOffer    ;
    database_entity GroupQuoteRidersFooter    ;
    database_entity GroupQuotePolicyFormId    ;
    database_entity GroupQuoteStateVariations ;
    database_entity GroupQuoteProspectus      ;
    database_entity GroupQuoteUnderwriter     ;
    database_entity GroupQuoteBrokerDealer    ;
    database_entity GroupQuoteRubricMandatory ;
    database_entity GroupQuoteRubricVoluntary ;
    database_entity GroupQuoteRubricFusion    ;
    database_entity GroupQuoteFooterMandatory ;
    database_entity GroupQuoteFooterVoluntary ;
    database_entity GroupQuoteFooterFusion    ;
    database_entity MinimumPremiumFootnote    ;
    database_entity PremAllocationFootnote    ;
    database_entity InterestDisclaimer        ;
    database_entity GuarMortalityFootnote     ;
    database_entity ProductDescription        ;
    database_entity StableValueFootnote       ;
    database_entity NoVanishPremiumFootnote   ;
    database_entity RejectPremiumFootnote     ;
    database_entity ExpRatingFootnote         ;
    database_entity MortalityBlendFootnote    ;
    database_entity HypotheticalRatesFootnote ;
    database_entity SalesLoadRefundFootnote   ;
    database_entity NoLapseEverFootnote       ;
    database_entity NoLapseFootnote           ;
    database_entity CurrentValuesFootnote     ;
    database_entity DBOption1Footnote         ;
    database_entity DBOption2Footnote         ;
    database_entity DBOption3Footnote         ;
    database_entity MinDeathBenefitFootnote   ;
    database_entity ExpRatRiskChargeFootnote  ;
    database_entity ExchangeChargeFootnote1   ;
    database_entity FlexiblePremiumFootnote   ;
    database_entity GuaranteedValuesFootnote  ;
    database_entity CreditingRateFootnote     ;
    database_entity GrossRateFootnote         ;
    database_entity NetRateFootnote           ;
    database_entity MecFootnote               ;
    database_entity GptFootnote               ;
    database_entity MidpointValuesFootnote    ;
    database_entity SinglePremiumFootnote     ;
    database_entity MonthlyChargesFootnote    ;
    database_entity UltCreditingRateFootnote  ;
    database_entity UltCreditingRateHeader    ;
    database_entity MaxNaarFootnote           ;
    database_entity PremTaxSurrChgFootnote    ;
    database_entity PolicyFeeFootnote         ;
    database_entity AssetChargeFootnote       ;
    database_entity InvestmentIncomeFootnote  ;
    database_entity IrrDbFootnote             ;
    database_entity IrrCsvFootnote            ;
    database_entity MortalityChargesFootnote  ;
    database_entity LoanAndWithdrawalFootnote ;
    database_entity LoanFootnote              ;
    database_entity ImprimaturPresale         ;
    database_entity ImprimaturPresaleComposite;
    database_entity ImprimaturInforce         ;
    database_entity ImprimaturInforceComposite;
    database_entity StateMarketingImprimatur  ;
    database_entity NonGuaranteedFootnote     ;
    database_entity NonGuaranteedFootnote1    ;
    database_entity NonGuaranteedFootnote1Tx  ;
    database_entity FnMonthlyDeductions       ;
    database_entity SurrenderFootnote         ;
    database_entity PortabilityFootnote       ;
    database_entity FundRateFootnote          ;
    database_entity IssuingCompanyFootnote    ;
    database_entity SubsidiaryFootnote        ;
    database_entity PlacementAgentFootnote    ;
    database_entity MarketingNameFootnote     ;
    database_entity GuarIssueDisclaimerNcSc   ;
    database_entity GuarIssueDisclaimerMd     ;
    database_entity GuarIssueDisclaimerTx     ;
    database_entity IllRegCertAgent           ;
    database_entity IllRegCertAgentIl         ;
    database_entity IllRegCertAgentTx         ;
    database_entity IllRegCertClient          ;
    database_entity IllRegCertClientIl        ;
    database_entity IllRegCertClientTx        ;
    database_entity FnMaturityAge             ;
    database_entity FnPartialMortality        ;
    database_entity FnProspectus              ;
    database_entity FnInitialSpecAmt          ;
    database_entity FnInforceAcctVal          ;
    database_entity FnInforceTaxBasis         ;
    database_entity Fn1035Charge              ;
    database_entity FnMecExtraWarning         ;
    database_entity FnNotTaxAdvice            ;
    database_entity FnNotTaxAdvice2           ;
    database_entity FnImf                     ;
    database_entity FnCensus                  ;
    database_entity FnDacTax                  ;
    database_entity FnDefnLifeIns             ;
    database_entity FnBoyEoy                  ;
    database_entity FnGeneralAccount          ;
    database_entity FnPpMemorandum            ;
    database_entity FnPpAccreditedInvestor    ;
    database_entity FnPpLoads                 ;
    database_entity FnProposalUnderwriting    ;
    database_entity FnGuaranteedPremium       ;
    database_entity FnOmnibusDisclaimer       ;
    database_entity FnInitialDbo              ;
    database_entity DefnGuarGenAcctRate       ;
    database_entity DefnAV                    ;
    database_entity DefnCSV                   ;
    database_entity DefnMec                   ;
    database_entity DefnOutlay                ;
    database_entity DefnSpecAmt               ;
    database_entity WeightClass               ;
    database_entity WeightGender              ;
    database_entity WeightSmoking             ;
    database_entity WeightAge                 ;
    database_entity WeightSpecAmt             ;
    database_entity WeightState               ;
    database_entity FullExpPol                ;
    database_entity FullExpPrem               ;
    database_entity FullExpDumpin             ;
    database_entity FullExpSpecAmt            ;
    database_entity VarExpPol                 ;
    database_entity VarExpPrem                ;
    database_entity VarExpDumpin              ;
    database_entity VarExpSpecAmt             ;
    database_entity ExpSpecAmtLimit           ;
    database_entity MedicalProportion         ;
    database_entity UwTestCost                ;
    database_entity VxBasicQTable             ;
    database_entity VxDeficQTable             ;
    database_entity VxTaxQTable               ;
    database_entity StatVxInt                 ;
    database_entity TaxVxInt                  ;
    database_entity StatVxQ                   ;
    database_entity TaxVxQ                    ;
    database_entity DeficVxQ                  ;
    database_entity SnflQ                     ;
    database_entity LapseRate                 ;
    database_entity ReqSurpNaar               ;
    database_entity ReqSurpVx                 ;
    database_entity LicFitRate                ;
    database_entity LicDacTaxRate             ;
    database_entity GdbVxMethod               ;
    database_entity PrimaryHurdle             ;
    database_entity SecondaryHurdle           ;
};

LMI_SO void print_databases();

#endif // dbdict_hpp
