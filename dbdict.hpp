// Product-database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifndef dbdict_hpp
#define dbdict_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "dbvalue.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "xml_serializable.hpp"

#include <string>

/// Cached product database.

class LMI_SO DBDictionary
    :        private lmi::uncopyable   <DBDictionary>
    ,virtual private obstruct_slicing  <DBDictionary>
    ,        public  xml_serializable  <DBDictionary>
    ,        public  MemberSymbolTable <DBDictionary>
{
    friend class DatabaseDocument;
    friend class input_test;
    friend class product_file_test;

  public:
    static DBDictionary& instance();
    ~DBDictionary();

    database_entity const& datum(std::string const&) const;

    void Init(std::string const& filename);
    void WriteSampleDBFile();
    void WriteProprietaryDBFiles();

    void InitAntediluvian();

  private:
    DBDictionary();

    void ascribe_members();

    database_entity& datum(std::string const&);

    void WriteDB(std::string const& filename) const;
    void Add(database_entity const&);
    void InitDB();

    static void InvalidateCache();

    // xml_serializable required implementation.
    virtual int                class_version() const;
    virtual std::string const& xml_root_name() const;

    // xml_serializable overrides.
    virtual void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        );
    virtual void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const;

    static std::string cached_filename_;

    database_entity MinIssAge           ;
    database_entity MaxIssAge           ;
    database_entity MaxIncrAge          ;
    database_entity AllowFullUw         ;
    database_entity AllowParamedUw      ;
    database_entity AllowNonmedUw       ;
    database_entity AllowSimpUw         ;
    database_entity AllowGuarUw         ;
    database_entity SmokeOrTobacco      ;
    database_entity PrefOrSelect        ;
    database_entity AllowPreferredClass ;
    database_entity AllowUltraPrefClass ;
    database_entity AllowSubstdTable    ;
    database_entity AllowFlatExtras     ;
    database_entity AllowRatedWp        ;
    database_entity AllowRatedAdb       ;
    database_entity AllowRatedTerm      ;
    database_entity AllowRetirees       ;
    database_entity AllowUnisex         ;
    database_entity AllowSexDistinct    ;
    database_entity AllowUnismoke       ;
    database_entity AllowSmokeDistinct  ;
    database_entity StateApproved       ;
    database_entity AllowStateXX        ;
    database_entity AllowForeign        ;
    database_entity Allowable           ;
    database_entity AllowCvat           ;
    database_entity AllowGpt            ;
    database_entity AllowNo7702         ;
    database_entity CorridorTable       ;
    database_entity SevenPayTable       ;
    database_entity Irc7702QTable       ;
    database_entity PremLoad7702        ;
    database_entity Equiv7702Dbo3       ;
    database_entity GuarCoiTable        ;
    database_entity GuarCoiIsAnnual     ;
    database_entity GuarCoiMultiplier   ;
    database_entity CurrCoiTable        ;
    database_entity CurrCoiIsAnnual     ;
    database_entity MinInputCoiMult     ;
    database_entity CurrCoiMultiplier   ;
    database_entity UnusualCoiBanding   ;
    database_entity CurrCoiTable0Limit  ;
    database_entity CurrCoiTable1       ;
    database_entity CurrCoiTable1Limit  ;
    database_entity CurrCoiTable2       ;
    database_entity MdptCoiTable        ;
    database_entity MdptCoiIsAnnual     ;
    database_entity CoiNyMinTable       ;
    database_entity UseNyCoiFloor       ;
    database_entity MaxMonthlyCoiRate   ;
    database_entity GuarCoiCeiling      ;
    database_entity CoiGuarIsMin        ;
    database_entity SubstdTableMult     ;
    database_entity SubstdTableMultTable;
    database_entity CoiUpper12Method    ;
    database_entity CoiInforceReentry   ;
    database_entity AllowMortBlendSex   ;
    database_entity AllowMortBlendSmoke ;
    database_entity GuarInt             ;
    database_entity NaarDiscount        ;
    database_entity GuarIntSpread       ;
    database_entity GuarMandE           ;
    database_entity CurrIntSpread       ;
    database_entity CurrMandE           ;
    database_entity GenAcctIntBonus     ;
    database_entity BonusInt            ;
    database_entity IntFloor            ;
    database_entity AllowGenAcct        ;
    database_entity AllowSepAcct        ;
    database_entity AllowGenAcctEarnRate;
    database_entity AllowSepAcctNetRate ;
    database_entity MaxGenAcctRate      ;
    database_entity MaxSepAcctRate      ;
    database_entity SepAcctSpreadMethod ;
    database_entity IntSpreadMode       ;
    database_entity DynamicMandE        ;
    database_entity AllowAmortPremLoad  ;
    database_entity LoadAmortFundCharge ;
    database_entity AllowImfOverride    ;
    database_entity AssetChargeType     ;
    database_entity StableValFundCharge ;
    database_entity GuarFundAdminChg    ;
    database_entity CurrFundAdminChg    ;
    database_entity FundCharge          ;
    database_entity GuarMonthlyPolFee   ;
    database_entity GuarAnnualPolFee    ;
    database_entity GuarPremLoadTgt     ;
    database_entity GuarPremLoadExc     ;
    database_entity GuarPremLoadTgtRfd  ;
    database_entity GuarPremLoadExcRfd  ;
    database_entity GuarSpecAmtLoad     ;
    database_entity GuarSpecAmtLoadTable;
    database_entity GuarAcctValLoad     ;
    database_entity CurrMonthlyPolFee   ;
    database_entity CurrAnnualPolFee    ;
    database_entity CurrPremLoadTgt     ;
    database_entity CurrPremLoadExc     ;
    database_entity CurrPremLoadTgtRfd  ;
    database_entity CurrPremLoadExcRfd  ;
    database_entity CurrSpecAmtLoad     ;
    database_entity CurrSpecAmtLoadTable;
    database_entity CurrAcctValLoad     ;
    database_entity TgtPremMonthlyPolFee;
    database_entity LoadRfdProportion   ;
    database_entity SpecAmtLoadLimit    ;
    database_entity DynamicSepAcctLoad  ;
    database_entity DynSepAcctLoadLimit ;
    database_entity DacTaxFundCharge    ;
    database_entity DacTaxPremLoad      ;
    database_entity PremTaxFundCharge   ;
    database_entity PremTaxLoad         ;
    database_entity WaivePremTaxInt1035 ;
    database_entity PremTaxRetalLimit   ;
    database_entity PremTaxTierGroup    ;
    database_entity PremTaxTierPeriod   ;
    database_entity PremTaxTierNonDecr  ;
    database_entity PremTaxAmortPeriod  ;
    database_entity PremTaxAmortIntRate ;
    database_entity PremTaxRate         ;
    database_entity PremTaxState        ;
    database_entity SurrChgAcctValMult  ;
    database_entity SurrChgAcctValSlope ;
    database_entity SurrChgSpecAmtMult  ;
    database_entity SurrChgSpecAmtSlope ;
    database_entity SurrChgPremMult     ;
    database_entity SurrChgOnIncr       ;
    database_entity SurrChgOnDecr       ;
    database_entity Has1035ExchCharge   ;
    database_entity SnflQTable          ;
    database_entity CoiSnflIsGuar       ;
    database_entity SurrChgByFormula    ;
    database_entity SurrChgPeriod       ;
    database_entity SurrChgZeroDur      ;
    database_entity SurrChgNlpMult      ;
    database_entity SurrChgNlpMax       ;
    database_entity SurrChgEaMax        ;
    database_entity SurrChgAmort        ;
    database_entity AllowSpecAmtIncr    ;
    database_entity MinSpecAmtIncr      ;
    database_entity EnforceNaarLimit    ;
    database_entity MinSpecAmt          ;
    database_entity MinIssSpecAmt       ;
    database_entity MinIssBaseSpecAmt   ;
    database_entity MinRenlSpecAmt      ;
    database_entity MinRenlBaseSpecAmt  ;
    database_entity MaxIssSpecAmt       ;
    database_entity MaxRenlSpecAmt      ;
    database_entity AllowDbo1           ;
    database_entity AllowDbo2           ;
    database_entity AllowDbo3           ;
    database_entity AllowChangeToDbo2   ;
    database_entity DboChgCanIncrSpecAmt;
    database_entity DboChgCanDecrSpecAmt;
    database_entity AllowExtEndt        ;
    database_entity AllowTerm           ;
    database_entity GuarTermTable       ;
    database_entity TermTable           ;
    database_entity TermMinIssAge       ;
    database_entity TermMaxIssAge       ;
    database_entity TermForcedConvAge   ;
    database_entity MaxTermProportion   ;
    database_entity TermCoiRate         ;
    database_entity TermPremRate        ;
    database_entity TermIsDbFor7702     ;
    database_entity TermIsDbFor7702A    ;
    database_entity AllowWp             ;
    database_entity WpTable             ;
    database_entity WpMinIssAge         ;
    database_entity WpMaxIssAge         ;
    database_entity WpMax               ;
    database_entity WpCoiRate           ;
    database_entity WpPremRate          ;
    database_entity WpChargeMethod      ;
    database_entity AllowAdb            ;
    database_entity AdbTable            ;
    database_entity AdbMinIssAge        ;
    database_entity AdbMaxIssAge        ;
    database_entity AdbLimit            ;
    database_entity AdbCoiRate          ;
    database_entity AdbPremRate         ;
    database_entity AllowSpouseRider    ;
    database_entity SpouseRiderGuarTable;
    database_entity SpouseRiderTable    ;
    database_entity AllowChildRider     ;
    database_entity ChildRiderTable     ;
    database_entity AllowWd             ;
    database_entity WdFee               ;
    database_entity WdFeeRate           ;
    database_entity FreeWdProportion    ;
    database_entity MinWd               ;
    database_entity MaxWdAcctValMult    ;
    database_entity MaxWdDed            ;
    database_entity WdCanDecrSpecAmtDbo1;
    database_entity WdCanDecrSpecAmtDbo2;
    database_entity WdCanDecrSpecAmtDbo3;
    database_entity FirstWdYear         ;
    database_entity AllowLoan           ;
    database_entity AllowPrefLoan       ;
    database_entity AllowFixedLoan      ;
    database_entity AllowVlr            ;
    database_entity FixedLoanRate       ;
    database_entity MaxVlrRate          ;
    database_entity MaxLoanAcctValMult  ;
    database_entity MaxLoanDed          ;
    database_entity GuarPrefLoanSpread  ;
    database_entity GuarRegLoanSpread   ;
    database_entity CurrPrefLoanSpread  ;
    database_entity CurrRegLoanSpread   ;
    database_entity FirstLoanYear       ;
    database_entity MinPremType         ;
    database_entity MinPremIntSpread    ;
    database_entity TgtPremType         ;
    database_entity TgtPremTable        ;
    database_entity TgtPremFixedAtIssue ;
    database_entity TgtPremIgnoreSubstd ;
    database_entity MinPmt              ;
    database_entity NoLapseMinDur       ;
    database_entity NoLapseMinAge       ;
    database_entity NoLapseUnratedOnly  ;
    database_entity NoLapseDbo1Only     ;
    database_entity NoLapseAlwaysActive ;
    database_entity AllowHoneymoon      ;
    database_entity DeductionMethod     ;
    database_entity DeductionAcct       ;
    database_entity DistributionMethod  ;
    database_entity DistributionAcct    ;
    database_entity EePremMethod        ;
    database_entity EePremAcct          ;
    database_entity ErPremMethod        ;
    database_entity ErPremAcct          ;
    database_entity CompTarget          ;
    database_entity CompExcess          ;
    database_entity CompChargeBack      ;
    database_entity AssetComp           ;
    database_entity AllowExtraAssetComp ;
    database_entity AllowExtraPremComp  ;
    database_entity AllowExpRating      ;
    database_entity ExpRatStdDevMult    ;
    database_entity ExpRatIbnrMult      ;
    database_entity ExpRatCoiRetention  ;
    database_entity ExpRatRiskCoiMult   ;
    database_entity ExpRatAmortPeriod   ;
    database_entity LedgerType          ;
    database_entity AgeLastOrNearest    ;
    database_entity MaxIllusAge         ;
    database_entity MaturityAge         ;
    database_entity LapseIgnoresSurrChg ;
    database_entity DefaultProcessOrder ;
    database_entity NominallyPar        ;
    database_entity TableYTable         ;
    database_entity Gam83Table          ;
    database_entity WeightClass         ;
    database_entity WeightGender        ;
    database_entity WeightSmoking       ;
    database_entity WeightAge           ;
    database_entity WeightSpecAmt       ;
    database_entity WeightState         ;
    database_entity FullExpPol          ;
    database_entity FullExpPrem         ;
    database_entity FullExpDumpin       ;
    database_entity FullExpSpecAmt      ;
    database_entity VarExpPol           ;
    database_entity VarExpPrem          ;
    database_entity VarExpDumpin        ;
    database_entity VarExpSpecAmt       ;
    database_entity ExpSpecAmtLimit     ;
    database_entity MedicalProportion   ;
    database_entity UwTestCost          ;
    database_entity VxBasicQTable       ;
    database_entity VxDeficQTable       ;
    database_entity VxTaxQTable         ;
    database_entity StatVxInt           ;
    database_entity TaxVxInt            ;
    database_entity StatVxQ             ;
    database_entity TaxVxQ              ;
    database_entity DefVxQ              ;
    database_entity SnflQ               ;
    database_entity LapseRate           ;
    database_entity ReqSurpNaar         ;
    database_entity ReqSurpVx           ;
    database_entity LicFitRate          ;
    database_entity LicDacTaxRate       ;
    database_entity GdbVxMethod         ;
    database_entity PrimaryHurdle       ;
    database_entity SecondaryHurdle     ;
};

void LMI_SO print_databases();

#endif // dbdict_hpp

