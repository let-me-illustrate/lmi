// Product data representable as strings.
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

#ifndef product_data_hpp
#define product_data_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <string>

/// A single product datum: a string with an optional gloss.
///
/// For example, the principal string datum might be the full name of
/// the issuing company, whereas the gloss might explain under what
/// conditions that company is used instead of a sibling.
///
/// Implicitly-declared special member functions do the right thing.

class glossed_string final
{
    friend class PolicyDocument;

  public:
    glossed_string() = default;
    explicit glossed_string
        (std::string const& datum
        ,std::string const& gloss = std::string()
        );
    ~glossed_string() = default;

    glossed_string& operator=(std::string const&);

    bool operator==(glossed_string const&) const;

    std::string const& datum() const;
    std::string const& gloss() const;

  private:
    std::string datum_;
    std::string gloss_;
};

class product_data;

template<> struct deserialized<product_data>
{
    typedef glossed_string value_type;
};

/// Product data representable as strings, including filenames.
///
/// This is the "master" product file: it includes the filenames of
/// all other product files.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO product_data
    :public xml_serializable  <product_data>
    ,public MemberSymbolTable <product_data>
{
    friend class BasicValues; // For antediluvian fork only.
    friend class PolicyDocument;
    friend class product_file_test;

    typedef deserialized<product_data>::value_type value_type;

  public:
    explicit product_data(std::string const& product_name);
    ~product_data() override;

    std::string const& datum(std::string const& name) const;

    // Legacy functions to support creating product files programmatically.
    static void write_policy_files();
    static void write_proprietary_policy_files();

  protected:
    product_data();
    glossed_string& item(std::string const& name);

  private:
    product_data(product_data const&);
    product_data& operator=(product_data const&) = delete;

    void ascribe_members();

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    value_type fetch_element
        (xml::element const& e
        ) const override;
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
    bool is_detritus(std::string const&) const override;
    void redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,value_type       & value
        ) const override;
    void redintegrate_ex_post
        (int                                     file_version
        ,std::map<std::string,value_type> const& detritus_map
        ,std::list<std::string>           const& residuary_names
        ) override;

    // Names of files that contain other product data.
    glossed_string DatabaseFilename;
    glossed_string FundFilename;
    glossed_string LingoFilename;
    glossed_string RoundingFilename;
    glossed_string TierFilename;

    // Names of rate-table files.
    glossed_string CvatCorridorFilename;
    glossed_string Irc7702NspFilename;
    glossed_string CurrCOIFilename;
    glossed_string GuarCOIFilename;
    glossed_string WPFilename;
    glossed_string ADDFilename;
    glossed_string ChildRiderFilename;
    glossed_string CurrSpouseRiderFilename;
    glossed_string GuarSpouseRiderFilename;
    glossed_string CurrTermFilename;
    glossed_string GuarTermFilename;
    glossed_string GroupProxyFilename;
    glossed_string SevenPayFilename;
    glossed_string MinPremFilename;
    glossed_string TgtPremFilename;
    glossed_string Irc7702QFilename;
    glossed_string PartialMortalityFilename;
    glossed_string SubstdTblMultFilename;
    glossed_string CurrSpecAmtLoadFilename;
    glossed_string GuarSpecAmtLoadFilename;

    // Essential strings describing the policy and company.
    glossed_string PolicyMktgName;
    glossed_string PolicyLegalName;
    glossed_string InsCoShortName;
    glossed_string InsCoName;
    glossed_string InsCoAddr;
    glossed_string InsCoStreet;
    glossed_string InsCoPhone;
    glossed_string InsCoDomicile;
    glossed_string MainUnderwriter;
    glossed_string MainUnderwriterAddress;
    glossed_string CoUnderwriter;
    glossed_string CoUnderwriterAddress;

    // Terms defined in the contract, which must be used for column
    // headers, footnotes, etc. according to the illustration reg.
    glossed_string AvName;
    glossed_string CsvName;
    glossed_string CsvHeaderName;
    glossed_string NoLapseProvisionName;
    glossed_string ContractName;
    glossed_string DboName;
    glossed_string DboNameLevel;
    glossed_string DboNameIncreasing;
    glossed_string DboNameReturnOfPremium;
    glossed_string DboNameMinDeathBenefit;
    glossed_string GenAcctName;
    glossed_string GenAcctNameElaborated;
    glossed_string SepAcctName;
    glossed_string SpecAmtName;
    glossed_string SpecAmtNameElaborated;

    // Underwriting terms.
    glossed_string UwBasisMedical;
    glossed_string UwBasisParamedical;
    glossed_string UwBasisNonmedical;
    glossed_string UwBasisSimplified;
    glossed_string UwBasisGuaranteed;
    glossed_string UwClassPreferred;
    glossed_string UwClassStandard;
    glossed_string UwClassRated;
    glossed_string UwClassUltra;

    // Ledger column definitions.
    glossed_string AccountValueFootnote;
    glossed_string AttainedAgeFootnote;
    glossed_string CashSurrValueFootnote;
    glossed_string DeathBenefitFootnote;
    glossed_string InitialPremiumFootnote;
    glossed_string NetPremiumFootnote;
    glossed_string GrossPremiumFootnote;
    glossed_string OutlayFootnote;
    glossed_string PolicyYearFootnote;

    // Terse rider names.
    glossed_string ADDTerseName;
    glossed_string InsurabilityTerseName;
    glossed_string ChildTerseName;
    glossed_string SpouseTerseName;
    glossed_string TermTerseName;
    glossed_string WaiverTerseName;
    glossed_string AccelBftRiderTerseName;
    glossed_string OverloanRiderTerseName;

    // Rider footnotes.
    glossed_string ADDFootnote;
    glossed_string ChildFootnote;
    glossed_string SpouseFootnote;
    glossed_string TermFootnote;
    glossed_string WaiverFootnote;
    glossed_string AccelBftRiderFootnote;
    glossed_string OverloanRiderFootnote;

    // Group quotes.
    glossed_string GroupQuoteShortProductName;
    glossed_string GroupQuoteIsNotAnOffer;
    glossed_string GroupQuoteRidersFooter;
    glossed_string GroupQuotePolicyFormId;
    glossed_string GroupQuoteStateVariations;
    glossed_string GroupQuoteProspectus;
    glossed_string GroupQuoteUnderwriter;
    glossed_string GroupQuoteBrokerDealer;
    // Group plan type is one of:
    //   -Mandatory: no individual selection of amounts; typically,
    //     the employer pays the entire premium
    //   -Voluntary: individual selection of amounts; typically, the
    //      employee pays the premium; may be called "supplemental"
    //      when it complements a (separate) "mandatory" plan
    //   -Fusion: mandatory and supplemental combined; typically, the
    //      employer and employee pay their respective premiums
    glossed_string GroupQuoteRubricMandatory;
    glossed_string GroupQuoteRubricVoluntary;
    glossed_string GroupQuoteRubricFusion;
    glossed_string GroupQuoteFooterMandatory;
    glossed_string GroupQuoteFooterVoluntary;
    glossed_string GroupQuoteFooterFusion;

    // Premium-specific footnotes.
    glossed_string MinimumPremiumFootnote;
    glossed_string PremAllocationFootnote;

    // Miscellaneous other footnotes.
    glossed_string InterestDisclaimer;
    glossed_string GuarMortalityFootnote;
    glossed_string ProductDescription;
    glossed_string StableValueFootnote;
    glossed_string NoVanishPremiumFootnote;
    glossed_string RejectPremiumFootnote;
    glossed_string ExpRatingFootnote;
    glossed_string MortalityBlendFootnote;
    glossed_string HypotheticalRatesFootnote;
    glossed_string SalesLoadRefundFootnote;
    glossed_string NoLapseEverFootnote;
    glossed_string NoLapseFootnote;
    glossed_string CurrentValuesFootnote;
    glossed_string DBOption1Footnote;
    glossed_string DBOption2Footnote;
    glossed_string DBOption3Footnote;
    glossed_string MinDeathBenefitFootnote;
    glossed_string ExpRatRiskChargeFootnote;
    glossed_string ExchangeChargeFootnote1;
    glossed_string FlexiblePremiumFootnote;
    glossed_string GuaranteedValuesFootnote;
    glossed_string CreditingRateFootnote;
    glossed_string GrossRateFootnote;
    glossed_string NetRateFootnote;
    glossed_string MecFootnote;
    glossed_string GptFootnote;
    glossed_string MidpointValuesFootnote;
    glossed_string SinglePremiumFootnote;
    glossed_string MonthlyChargesFootnote;
    glossed_string UltCreditingRateFootnote;
    glossed_string UltCreditingRateHeader;
    glossed_string MaxNaarFootnote;
    glossed_string PremTaxSurrChgFootnote;
    glossed_string PolicyFeeFootnote;
    glossed_string AssetChargeFootnote;
    glossed_string InvestmentIncomeFootnote;
    glossed_string IrrDbFootnote;
    glossed_string IrrCsvFootnote;
    glossed_string MortalityChargesFootnote;
    glossed_string LoanAndWithdrawalFootnote;
    glossed_string LoanFootnote;
    glossed_string ImprimaturPresale;
    glossed_string ImprimaturPresaleComposite;
    glossed_string ImprimaturInforce;
    glossed_string ImprimaturInforceComposite;
    glossed_string StateMarketingImprimatur;
    glossed_string NonGuaranteedFootnote;
    glossed_string NonGuaranteedFootnote1;
    glossed_string NonGuaranteedFootnote1Tx;
    glossed_string FnMonthlyDeductions;
    glossed_string SurrenderFootnote;
    glossed_string PortabilityFootnote;
    glossed_string FundRateFootnote;
    glossed_string IssuingCompanyFootnote;
    glossed_string SubsidiaryFootnote;
    glossed_string PlacementAgentFootnote;
    glossed_string MarketingNameFootnote;
    glossed_string GuarIssueDisclaimerNcSc;
    glossed_string GuarIssueDisclaimerMd;
    glossed_string GuarIssueDisclaimerTx;
    glossed_string IllRegCertAgent;
    glossed_string IllRegCertAgentIl;
    glossed_string IllRegCertAgentTx;
    glossed_string IllRegCertClient;
    glossed_string IllRegCertClientIl;
    glossed_string IllRegCertClientTx;
    glossed_string FnMaturityAge;
    glossed_string FnPartialMortality;
    glossed_string FnProspectus;
    glossed_string FnInitialSpecAmt;
    glossed_string FnInforceAcctVal;
    glossed_string FnInforceTaxBasis;
    glossed_string Fn1035Charge;
    glossed_string FnMecExtraWarning;
    glossed_string FnNotTaxAdvice;
    glossed_string FnNotTaxAdvice2;
    glossed_string FnImf;
    glossed_string FnCensus;
    glossed_string FnDacTax;
    glossed_string FnDefnLifeIns;
    glossed_string FnBoyEoy;
    glossed_string FnGeneralAccount;
    glossed_string FnPpMemorandum;
    glossed_string FnPpAccreditedInvestor;
    glossed_string FnPpLoads;
    glossed_string FnProposalUnderwriting;
    glossed_string FnGuaranteedPremium;
    glossed_string FnOmnibusDisclaimer;
    glossed_string FnInitialDbo;
    glossed_string DefnGuarGenAcctRate;
    glossed_string DefnAV;
    glossed_string DefnCSV;
    glossed_string DefnMec;
    glossed_string DefnOutlay;
    glossed_string DefnSpecAmt;
};

LMI_SO void load(product_data      &, fs::path const&);
LMI_SO void save(product_data const&, fs::path const&);

#endif // product_data_hpp
