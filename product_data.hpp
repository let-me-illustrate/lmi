// Product data representable as strings.
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

#ifndef product_data_hpp
#define product_data_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "xml_serializable.hpp"

#include <string>

/// A single product datum: a string with an optional gloss.
///
/// For example, the principal string datum might be the full name of
/// the issuing company, whereas the gloss might explain under what
/// conditions that company is used instead of a sibling.
///
/// Implicitly-declared special member functions do the right thing.

class glossed_string
    :virtual private obstruct_slicing<glossed_string>
{
    friend class PolicyDocument;
    friend class product_data;

  public:
    explicit glossed_string
        (std::string const& datum
        ,std::string const& gloss = std::string()
        );
    ~glossed_string();

    bool operator==(glossed_string const&) const;

    std::string const& datum() const;
    std::string const& gloss() const;

  private:
    glossed_string();
    glossed_string& operator=(std::string const&);

    std::string datum_;
    std::string gloss_;
};

/// Product data representable as strings, including filenames.
///
/// This is the "master" product file: it includes the filenames of
/// all other product files.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO product_data
    :        private lmi::uncopyable   <product_data>
    ,virtual private obstruct_slicing  <product_data>
    ,        public  xml_serializable  <product_data>
    ,        public  MemberSymbolTable <product_data>
{
    friend class PolicyDocument;

  public:
    explicit product_data(std::string const& product_name);
    ~product_data();

    std::string const& datum(std::string const& name) const;

    // Legacy functions to support creating product files programmatically.
    static void WritePolFiles();
    static void WriteProprietaryPolFiles();

  private:
    product_data();

    void ascribe_members();

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
    virtual bool is_detritus(std::string const&) const;

    // Names of files that contain other product data.
    glossed_string DatabaseFilename;
    glossed_string FundFilename;
    glossed_string RoundingFilename;
    glossed_string TierFilename;

    // Names of rate-table files.
    glossed_string CorridorFilename;
    glossed_string CurrCOIFilename;
    glossed_string GuarCOIFilename;
    glossed_string WPFilename;
    glossed_string ADDFilename;
    glossed_string ChildRiderFilename;
    glossed_string CurrSpouseRiderFilename;
    glossed_string GuarSpouseRiderFilename;
    glossed_string CurrTermFilename;
    glossed_string GuarTermFilename;
    glossed_string TableYFilename;
    glossed_string PremTaxFilename;
    glossed_string TAMRA7PayFilename;
    glossed_string TgtPremFilename;
    glossed_string IRC7702Filename;
    glossed_string Gam83Filename;
    glossed_string SubstdTblMultFilename;
    glossed_string CurrSpecAmtLoadFilename;
    glossed_string GuarSpecAmtLoadFilename;

    // Essential strings describing the policy and company.
    glossed_string PolicyForm;
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
    // headers according to the illustration reg.
    glossed_string AvName;
    glossed_string CsvName;
    glossed_string CsvHeaderName;
    glossed_string NoLapseProvisionName;

    // Most of the following are missing from the GUI.

    // Footnotes that describe various ledger columns.
    glossed_string AccountValueFootnote;
    glossed_string AttainedAgeFootnote;
    glossed_string CashSurrValueFootnote;
    glossed_string DeathBenefitFootnote;
    glossed_string InitialPremiumFootnote;
    glossed_string NetPremiumFootnote;
    glossed_string OutlayFootnote;
    glossed_string PolicyYearFootnote;

    // Rider footnotes.
    glossed_string ADDFootnote;
    glossed_string ChildFootnote;
    glossed_string SpouseFootnote;
    glossed_string TermFootnote;
    glossed_string WaiverFootnote;

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
    glossed_string NoLapseFootnote;
    glossed_string MarketValueAdjFootnote;
    glossed_string ExchangeChargeFootnote0;
    glossed_string CurrentValuesFootnote;
    glossed_string DBOption1Footnote;
    glossed_string DBOption2Footnote;
    glossed_string ExpRatRiskChargeFootnote;
    glossed_string ExchangeChargeFootnote1;
    glossed_string FlexiblePremiumFootnote;
    glossed_string GuaranteedValuesFootnote;
    glossed_string CreditingRateFootnote;
    glossed_string MecFootnote;
    glossed_string MidpointValuesFootnote;
    glossed_string SinglePremiumFootnote;
    glossed_string MonthlyChargesFootnote;
    glossed_string UltCreditingRateFootnote;
    glossed_string MaxNaarFootnote;
    glossed_string PremTaxSurrChgFootnote;
    glossed_string PolicyFeeFootnote;
    glossed_string AssetChargeFootnote;
    glossed_string InvestmentIncomeFootnote;
    glossed_string IrrDbFootnote;
    glossed_string IrrCsvFootnote;
    glossed_string MortalityChargesFootnote;
    glossed_string LoanAndWithdrawalFootnote;
    glossed_string PresaleTrackingNumber;
    glossed_string CompositeTrackingNumber;
    glossed_string InforceTrackingNumber;
    glossed_string InforceCompositeTrackingNumber;
    glossed_string InforceNonGuaranteedFootnote0;
    glossed_string InforceNonGuaranteedFootnote1;
    glossed_string InforceNonGuaranteedFootnote2;
    glossed_string InforceNonGuaranteedFootnote3;
    glossed_string NonGuaranteedFootnote;
    glossed_string MonthlyChargesPaymentFootnote;
    glossed_string SurrenderFootnote;
    glossed_string FundRateFootnote;
    glossed_string FundRateFootnote0;
    glossed_string FundRateFootnote1;
};

#endif // product_data_hpp

