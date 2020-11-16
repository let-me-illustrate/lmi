// Ledger data that do not vary by basis.
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

#ifndef ledger_invariant_hpp
#define ledger_invariant_hpp

#include "config.hpp"

#include "ledger_base.hpp"

#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "so_attributes.hpp"

#include <iosfwd>
#include <string>
#include <vector>

// This class holds all the output values calculated by class AccountValue
// that don't vary by basis.

class BasicValues;
class Ledger;

class LMI_SO LedgerInvariant final
    :public LedgerBase
{
  public:
    explicit LedgerInvariant(int len);
    LedgerInvariant(LedgerInvariant const&);
    LedgerInvariant& operator=(LedgerInvariant const&);
    ~LedgerInvariant() override;

    void Init(BasicValues const*);
    void ReInit(BasicValues const*);

    LedgerInvariant& PlusEq(LedgerInvariant const& a_Addend);

    bool                       is_irr_initialized()    const;
    bool                       IsFullyInitialized()    const;
    int                        GetLength()             const override;
    std::vector<double> const& GetInforceLives()       const;

    void CalculateIrrs(Ledger const&);

    void UpdateCRC(CRC& a_crc) const override;
    void Spew(std::ostream& os) const override;

// TODO ?? Make data private. Provide const accessors. Some values
// (e.g., outlay) could be calculated dynamically instead of stored.

    // BOY vectors.
    std::vector<double> GrossPmt;   // TODO ?? Why "gross"?
    std::vector<double> EeGrossPmt;
    std::vector<double> ErGrossPmt;
    std::vector<double> NetWD;      // TODO ?? Unclear whether this is truly net.
    std::vector<double> NewCashLoan;
    std::vector<double> Outlay;
    std::vector<double> GptForceout;
    std::vector<double> NaarForceout;
    std::vector<double> ModalMinimumPremium;
    std::vector<double> EeModalMinimumPremium;
    std::vector<double> ErModalMinimumPremium;
    std::vector<double> AddonMonthlyFee;

    // EOY vectors.
    std::vector<double> TermSpecAmt;
    std::vector<double> SpecAmt;

    // Forborne vectors.
    std::vector<double> Salary;

    // Nonscalable vectors.
    std::vector<double> IndvTaxBracket;
    std::vector<double> CorpTaxBracket;
    std::vector<double> AnnualFlatExtra;
    std::vector<double> HoneymoonValueSpread;
    std::vector<double> PartMortTableMult;
    std::vector<double> AddonCompOnAssets;
    std::vector<double> AddonCompOnPremium;
    std::vector<double> CorridorFactor;
    std::vector<double> AnnLoanDueRate;
    // TODO ?? M&E varies by basis (cf. 'GuarMaxMandE' below), so it
    // belongs in the variant ledger.
    std::vector<double> CurrMandE;
    // TODO ?? This is misbegotten. It's supposed to mean IMF including
    // any tiered component, but in that case it would vary by basis.
    std::vector<double> TotalIMF;
    std::vector<double> RefundableSalesLoad;

    // Scalable scalars.
    double      InitBaseSpecAmt;
    double      InitTermSpecAmt;
    double      ChildRiderAmount;
    double      SpouseRiderAmount;
    double      InitPrem;
    double      GuarPrem;
    double      InitSevenPayPrem;
    double      InitGSP;
    double      InitGLP;
    double      InitTgtPrem;
    double      ListBillPremium;
    double      EeListBillPremium;
    double      ErListBillPremium;
    double      Dumpin;
    double      External1035Amount;
    double      Internal1035Amount;
    double      InforceTotalAV;
    double      InforceTaxBasis;

    // Nonscalable scalars.
    double      MaleProportion;
    double      NonsmokerProportion;
    double      GuarMaxMandE;
    double      InitDacTaxRate;
    double      InitPremTaxRate;
    double      GenderBlended;
    double      SmokerBlended;
    double      Age;
    double      RetAge;
    double      EndtAge;
    double      GroupIndivSelection;
    double      NoLongerIssued;
    double      AllowGroupQuote;
    double      TxCallsGuarUwSubstd;
    double      AllowExperienceRating;
    double      UseExperienceRating;
    double      UsePartialMort;
    double      SurviveToExpectancy;
    double      SurviveToYear;
    double      SurviveToAge;
    double      SurvivalMaxYear;
    double      SurvivalMaxAge;
    double      AvgFund;
    double      CustomFund;
    double      IsMec;
    double      InforceIsMec;
    double      InforceYear;
    double      InforceMonth;
    double      MecYear;
    double      MecMonth;
    double      HasWP;
    double      HasADD;
    double      HasTerm;
    double      HasSupplSpecAmt;
    double      HasChildRider;
    double      HasSpouseRider;
    double      SpouseIssueAge;
    double      HasHoneymoon;
    double      PostHoneymoonSpread;
    double      SplitMinPrem;
    double      ErNotionallyPaysTerm;
    double      IsSinglePremium;
    double      MaxAnnGuarLoanSpread;
    double      MaxAnnCurrLoanDueRate;
    double      IsInforce;
    double      CurrentCoiMultiplier;
    double      NoLapseAlwaysActive;
    double      NoLapseMinDur;
    double      NoLapseMinAge;
    double      Has1035ExchCharge;
    double      EffDateJdn;
    double      DateOfBirthJdn;
    double      LastCoiReentryDateJdn;
    double      ListBillDateJdn;
    double      InforceAsOfDateJdn;
    double      GenAcctAllocation;
    double      SplitFundAllocation;
    double      WriteTsvFile;
    double      SupplementalReport;

    // Strings.

    // Essential strings describing the policy and company.
    std::string PolicyForm;
    // Ledger needs no member like product_data::PolicyFormAlternative; // LINGO !! expunge
    std::string PolicyMktgName;
    std::string PolicyLegalName;
    std::string CsoEra;
    std::string InsCoShortName;
    std::string InsCoName;
    std::string InsCoAddr;
    std::string InsCoStreet;
    std::string InsCoPhone;
    // Ledger needs no member like product_data::InsCoDomicile;
    std::string MainUnderwriter;
    std::string MainUnderwriterAddress;
    std::string CoUnderwriter;
    std::string CoUnderwriterAddress;

    // Terms defined in the contract, which must be used for column
    // headers according to the illustration reg.
    std::string AvName;
    std::string CsvName;
    std::string CsvHeaderName;
    std::string NoLapseProvisionName;
    std::string ContractName;
    std::string DboName;
    // PDF !! 'DboName+' variables are used only to assemble DBO
    // definitions for mce_ill_reg PDFs. Probably those definitions
    // should be moved into '.policy' files instead, in which case
    // the 'DboName*' variables here can be expunged. Incidentally,
    // 'DboNameReturnOfPremium' is omitted here only because the
    // mce_ill_reg products that lmi supports happen not to use a
    // definition for it today.
    std::string DboNameLevel;
    std::string DboNameIncreasing;
//  std::string DboNameReturnOfPremium;
    std::string DboNameMinDeathBenefit;
    std::string GenAcctName;
    std::string GenAcctNameElaborated;
    std::string SepAcctName;
    std::string SpecAmtName;
    std::string SpecAmtNameElaborated;
    std::string UwBasisMedical;
    std::string UwBasisParamedical;
    std::string UwBasisNonmedical;
    std::string UwBasisSimplified;
    std::string UwBasisGuaranteed;
    std::string UwClassPreferred;
    std::string UwClassStandard;
    std::string UwClassRated;
    std::string UwClassUltra;

    // Ledger column definitions.
    std::string AccountValueFootnote;
    std::string AttainedAgeFootnote;
    std::string CashSurrValueFootnote;
    std::string DeathBenefitFootnote;
    std::string InitialPremiumFootnote;
    std::string NetPremiumFootnote;
    std::string GrossPremiumFootnote;
    std::string OutlayFootnote;
    std::string PolicyYearFootnote;

    // Terse rider names.
    std::string ADDTerseName;
    std::string InsurabilityTerseName;
    std::string ChildTerseName;
    std::string SpouseTerseName;
    std::string TermTerseName;
    std::string WaiverTerseName;
    std::string AccelBftRiderTerseName;
    std::string OverloanRiderTerseName;

    // Rider footnotes.
    std::string ADDFootnote;
    std::string ChildFootnote;
    std::string SpouseFootnote;
    std::string TermFootnote;
    std::string WaiverFootnote;
    std::string AccelBftRiderFootnote;
    std::string OverloanRiderFootnote;

    // Group quote footnotes.
    std::string GroupQuoteShortProductName;
    std::string GroupQuoteIsNotAnOffer;
    std::string GroupQuoteRidersFooter;
    std::string GroupQuotePolicyFormId;
    std::string GroupQuoteStateVariations;
    std::string GroupQuoteProspectus;
    std::string GroupQuoteUnderwriter;
    std::string GroupQuoteBrokerDealer;
    std::string GroupQuoteRubricMandatory;
    std::string GroupQuoteRubricVoluntary;
    std::string GroupQuoteRubricFusion;
    std::string GroupQuoteFooterMandatory;
    std::string GroupQuoteFooterVoluntary;
    std::string GroupQuoteFooterFusion;

    // Premium-specific footnotes.
    std::string MinimumPremiumFootnote;
    std::string PremAllocationFootnote;

    // Miscellaneous other footnotes.
    std::string InterestDisclaimer;
    std::string GuarMortalityFootnote;
    std::string ProductDescription;
    std::string StableValueFootnote;
    std::string NoVanishPremiumFootnote;
    std::string RejectPremiumFootnote;
    std::string ExpRatingFootnote;
    std::string MortalityBlendFootnote;
    std::string HypotheticalRatesFootnote;
    std::string SalesLoadRefundFootnote;
    std::string NoLapseEverFootnote;
    std::string NoLapseFootnote;
    std::string CurrentValuesFootnote;
    std::string DBOption1Footnote;
    std::string DBOption2Footnote;
    std::string DBOption3Footnote;
    std::string MinDeathBenefitFootnote;
    std::string ExpRatRiskChargeFootnote;
    std::string ExchangeChargeFootnote1;
    std::string FlexiblePremiumFootnote;
    std::string GuaranteedValuesFootnote;
    std::string CreditingRateFootnote;
    std::string DefnGuarGenAcctRate;
    std::string GrossRateFootnote;
    std::string NetRateFootnote;
    std::string MecFootnote;
    std::string GptFootnote;
    std::string MidpointValuesFootnote;
    std::string SinglePremiumFootnote;
    std::string MonthlyChargesFootnote;
    std::string UltCreditingRateFootnote;
    std::string UltCreditingRateHeader;
    std::string MaxNaarFootnote;
    std::string PremTaxSurrChgFootnote;
    std::string PolicyFeeFootnote;
    std::string AssetChargeFootnote;
    std::string InvestmentIncomeFootnote;
    std::string IrrDbFootnote;
    std::string IrrCsvFootnote;
    std::string MortalityChargesFootnote;
    std::string LoanAndWithdrawalFootnote;
    std::string LoanFootnote;
    std::string ImprimaturPresale;
    std::string ImprimaturPresaleComposite;
    std::string ImprimaturInforce;
    std::string ImprimaturInforceComposite;
    std::string StateMarketingImprimatur;
    std::string NonGuaranteedFootnote;
    std::string NonGuaranteedFootnote1;
    std::string NonGuaranteedFootnote1Tx;
    std::string FnMonthlyDeductions;
    std::string SurrenderFootnote;
    std::string PortabilityFootnote;
    std::string FundRateFootnote;
    std::string IssuingCompanyFootnote;
    std::string SubsidiaryFootnote;
    std::string PlacementAgentFootnote;
    std::string MarketingNameFootnote;
    std::string GuarIssueDisclaimerNcSc;
    std::string GuarIssueDisclaimerMd;
    std::string GuarIssueDisclaimerTx;
    std::string IllRegCertAgent;
    std::string IllRegCertAgentIl;
    std::string IllRegCertAgentTx;
    std::string IllRegCertClient;
    std::string IllRegCertClientIl;
    std::string IllRegCertClientTx;
    std::string FnMaturityAge;
    std::string FnPartialMortality;
    std::string FnProspectus;
    std::string FnInitialSpecAmt;
    std::string FnInforceAcctVal;
    std::string FnInforceTaxBasis;
    std::string Fn1035Charge;
    std::string FnMecExtraWarning;
    std::string FnNotTaxAdvice;
    std::string FnNotTaxAdvice2;
    std::string FnImf;
    std::string FnCensus;
    std::string FnDacTax;
    std::string FnDefnLifeIns;
    std::string FnBoyEoy;
    std::string FnGeneralAccount;
    std::string FnPpMemorandum;
    std::string FnPpAccreditedInvestor;
    std::string FnPpLoads;
    std::string FnProposalUnderwriting;
    std::string FnGuaranteedPremium;
    std::string FnOmnibusDisclaimer;
    std::string FnInitialDbo;
    std::string DefnAV;
    std::string DefnCSV;
    std::string DefnMec;
    std::string DefnOutlay;
    std::string DefnSpecAmt;

    // Strings from class Input.

    std::string ProductName;
    std::string ProducerName;
    std::string ProducerStreet;
    std::string ProducerCityEtc;
    std::string ProducerPhone;
    std::string ProducerId;
    std::string CorpName;
    std::string MasterContractNumber;
    std::string ContractNumber;
    std::string Insured1;
    std::string Gender;
    std::string UWType;
    std::string Smoker;
    std::string UWClass;
    std::string SubstandardTable;
    std::string DefnLifeIns;
    std::string DefnMaterialChange;
    std::string PartMortTableName;

    std::string StateOfJurisdiction;
    std::string PremiumTaxState;
    std::string CountryIso3166Abbrev;
    std::string Comments;

    std::string SupplementalReportColumn00;
    std::string SupplementalReportColumn01;
    std::string SupplementalReportColumn02;
    std::string SupplementalReportColumn03;
    std::string SupplementalReportColumn04;
    std::string SupplementalReportColumn05;
    std::string SupplementalReportColumn06;
    std::string SupplementalReportColumn07;
    std::string SupplementalReportColumn08;
    std::string SupplementalReportColumn09;
    std::string SupplementalReportColumn10;
    std::string SupplementalReportColumn11;

    // Arguably enumerative members such as 'ErMode' should be of type
    // std::string instead, because strings would be more useful for
    // generating reports from this class's data, and mc_enum::str()
    // is not visible across a shared-library boundary. For now, this
    // kludge solves an immediate problem for premium quotes.
    std::string InitDBOpt;
    std::string InitEeMode;
    std::string InitErMode;

    // Public data members not included in any of the maps that are
    // handled by the base class are "special".

    // Special-case vectors.

    // Vectors of type other than <double>.
    std::vector<mce_dbopt>DBOpt;
    std::vector<mce_mode> EeMode;
    std::vector<mce_mode> ErMode;

    // Different length than other vectors.
    std::vector<double> InforceLives;

    std::vector<double>      FundNumbers;
    std::vector<std::string> FundNames;
    std::vector<int>         FundAllocs; // Obsolete--spreadsheet only.
    std::vector<double>      FundAllocations;

    // IRRs that we need to think about some more before deciding
    // where to put them.

    std::vector<double> IrrCsvGuar0;
    std::vector<double> IrrDbGuar0;
    std::vector<double> IrrCsvCurr0;
    std::vector<double> IrrDbCurr0;
    std::vector<double> IrrCsvGuarInput;
    std::vector<double> IrrDbGuarInput;
    std::vector<double> IrrCsvCurrInput;
    std::vector<double> IrrDbCurrInput;

    // Special-case strings.

    // Calendar dates are special because date formatting might be
    // customized or treated differently by different platforms.
    // Therefore, they are represented elsewhere as JDNs, and only the
    // JDNs are used in UpdateCRC() and Spew().
    std::string     EffDate;
    std::string     DateOfBirth;
    std::string     LastCoiReentryDate;
    std::string     ListBillDate;
    std::string     InforceAsOfDate;

  private:
    void Alloc(int len);
    void Copy(LedgerInvariant const&);
    void Destroy();
    void Init();

    // Special cases.
    int  Length;
    int  irr_precision_;
    bool irr_initialized_ {false}; // CalculateIrrs() succeeded
    bool FullyInitialized {false}; // Init(BasicValues const*) succeeded
};

inline bool LedgerInvariant::is_irr_initialized() const
{
    return irr_initialized_;
}

inline bool LedgerInvariant::IsFullyInitialized() const
{
    return FullyInitialized;
}

inline int LedgerInvariant::GetLength() const
{
    return Length;
}

inline std::vector<double> const& LedgerInvariant::GetInforceLives() const
{
    return InforceLives;
}

#endif // ledger_invariant_hpp
