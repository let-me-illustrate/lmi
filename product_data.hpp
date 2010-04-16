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
#include "xml_serializable.hpp"

#include <boost/utility.hpp>

#include <string>

/// Product data representable as strings, including filenames.
///
/// This is the "master" product file: it includes the filenames of
/// all other product files.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO product_data
    :        private boost::noncopyable
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
    virtual int         class_version() const;
    virtual std::string xml_root_name() const;

    // xml_serializable overrides.
    virtual bool is_detritus(std::string const&) const;

    // Names of files that contain other product data.
    std::string DatabaseFilename;
    std::string FundFilename;
    std::string RoundingFilename;
    std::string TierFilename;

    // Names of rate-table files.
    std::string CorridorFilename;
    std::string CurrCOIFilename;
    std::string GuarCOIFilename;
    std::string WPFilename;
    std::string ADDFilename;
    std::string ChildRiderFilename;
    std::string CurrSpouseRiderFilename;
    std::string GuarSpouseRiderFilename;
    std::string CurrTermFilename;
    std::string GuarTermFilename;
    std::string TableYFilename;
    std::string PremTaxFilename;
    std::string TAMRA7PayFilename;
    std::string TgtPremFilename;
    std::string IRC7702Filename;
    std::string Gam83Filename;
    std::string SubstdTblMultFilename;
    std::string CurrSpecAmtLoadFilename;
    std::string GuarSpecAmtLoadFilename;

    // Essential strings describing the policy and company.
    std::string PolicyForm;
    std::string PolicyMktgName;
    std::string PolicyLegalName;
    std::string InsCoShortName;
    std::string InsCoName;
    std::string InsCoAddr;
    std::string InsCoStreet;
    std::string InsCoPhone;
    std::string InsCoDomicile;
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

    // Most of the following are missing from the GUI.

    // Footnotes that describe various ledger columns.
    std::string AccountValueFootnote;
    std::string AttainedAgeFootnote;
    std::string CashSurrValueFootnote;
    std::string DeathBenefitFootnote;
    std::string InitialPremiumFootnote;
    std::string NetPremiumFootnote;
    std::string OutlayFootnote;
    std::string PolicyYearFootnote;

    // Rider footnotes.
    std::string ADDFootnote;
    std::string ChildFootnote;
    std::string SpouseFootnote;
    std::string TermFootnote;
    std::string WaiverFootnote;

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
    std::string NoLapseFootnote;
    std::string MarketValueAdjFootnote;
    std::string ExchangeChargeFootnote0;
    std::string CurrentValuesFootnote;
    std::string DBOption1Footnote;
    std::string DBOption2Footnote;
    std::string ExpRatRiskChargeFootnote;
    std::string ExchangeChargeFootnote1;
    std::string FlexiblePremiumFootnote;
    std::string GuaranteedValuesFootnote;
    std::string CreditingRateFootnote;
    std::string MecFootnote;
    std::string MidpointValuesFootnote;
    std::string SinglePremiumFootnote;
    std::string MonthlyChargesFootnote;
    std::string UltCreditingRateFootnote;
    std::string MaxNaarFootnote;
    std::string PremTaxSurrChgFootnote;
    std::string PolicyFeeFootnote;
    std::string AssetChargeFootnote;
    std::string InvestmentIncomeFootnote;
    std::string IrrDbFootnote;
    std::string IrrCsvFootnote;
    std::string MortalityChargesFootnote;
    std::string LoanAndWithdrawalFootnote;
    std::string PresaleTrackingNumber;
    std::string CompositeTrackingNumber;
    std::string InforceTrackingNumber;
    std::string InforceCompositeTrackingNumber;
    std::string InforceNonGuaranteedFootnote0;
    std::string InforceNonGuaranteedFootnote1;
    std::string InforceNonGuaranteedFootnote2;
    std::string InforceNonGuaranteedFootnote3;
    std::string NonGuaranteedFootnote;
    std::string MonthlyChargesPaymentFootnote;
};

#endif // product_data_hpp

