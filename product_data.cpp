// Product data representable as strings.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "product_data.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "map_lookup.hpp"
#include "miscellany.hpp"               // lmi_array_size()
#include "my_proem.hpp"                 // ::write_proem()
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <vector>

template class xml_serializable<product_data>;

namespace xml_serialize
{
template<> struct xml_io<glossed_string>
{
    static void to_xml(xml::element& e, glossed_string const& t)
    {
        set_element(e, "datum", t.datum());
        set_element(e, "gloss", t.gloss());
    }

    static void from_xml(xml::element const& e, glossed_string& t)
    {
        std::string datum;
        std::string gloss;
        get_element(e, "datum", datum);
        get_element(e, "gloss", gloss);
        t = glossed_string(datum, gloss);
    }
};
} // namespace xml_serialize

/// Specialize: needed for any_member::str().

template<> std::string value_cast<std::string>(glossed_string const& z)
{
    if(z.gloss().empty())
        {
        return z.datum();
        }
    else
        {
        fatal_error()
            << "Invalid function call. Context:"
            << '\n' << z.datum()
            << '\n' << z.gloss()
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Specialize: needed for any_member::operator=(std::string const&).

template<> glossed_string value_cast<glossed_string>(std::string const& z)
{
    return glossed_string(z);
}

glossed_string::glossed_string()
{}

glossed_string::glossed_string
    (std::string const& datum
    ,std::string const& gloss
    )
    :datum_(datum)
    ,gloss_(gloss)
{}

glossed_string::~glossed_string()
{}

glossed_string& glossed_string::operator=(std::string const& s)
{
    datum_ = s;
    return *this;
}

bool glossed_string::operator==(glossed_string const& z) const
{
    return z.datum_ == datum_ && z.gloss_ == gloss_;
}

std::string const& glossed_string::datum() const
{
    return datum_;
}

std::string const& glossed_string::gloss() const
{
    return gloss_;
}

/// Private default ctor.
///
/// Used only by friend class PolicyDocument and 'my_prod.cpp'.

product_data::product_data()
{
    ascribe_members();
}

/// Construct from product name.
///
/// The argument is a string (typically Input::ProductName) such as
/// 'sample'. The appropriate extension and path are added here to
/// produce a filepath.

product_data::product_data(std::string const& product_name)
{
    ascribe_members();

    fs::path path(product_name);
    LMI_ASSERT(product_name == path.leaf());
    path = fs::change_extension(path, ".policy");
    load(AddDataDir(path.string()));
}

product_data::~product_data()
{
}

/// Principal string datum (without gloss) for named member.

std::string const& product_data::datum(std::string const& name) const
{
    return member_cast<glossed_string>(operator[](name))->datum();
}

/// Enregister certain data members for access via any_member<>[].

void product_data::ascribe_members()
{
    ascribe("DatabaseFilename"              , &product_data::DatabaseFilename              );
    ascribe("FundFilename"                  , &product_data::FundFilename                  );
    ascribe("RoundingFilename"              , &product_data::RoundingFilename              );
    ascribe("TierFilename"                  , &product_data::TierFilename                  );
    ascribe("CvatCorridorFilename"          , &product_data::CvatCorridorFilename          );
    ascribe("Irc7702NspFilename"            , &product_data::Irc7702NspFilename            );
    ascribe("CurrCOIFilename"               , &product_data::CurrCOIFilename               );
    ascribe("GuarCOIFilename"               , &product_data::GuarCOIFilename               );
    ascribe("WPFilename"                    , &product_data::WPFilename                    );
    ascribe("ADDFilename"                   , &product_data::ADDFilename                   );
    ascribe("ChildRiderFilename"            , &product_data::ChildRiderFilename            );
    ascribe("CurrSpouseRiderFilename"       , &product_data::CurrSpouseRiderFilename       );
    ascribe("GuarSpouseRiderFilename"       , &product_data::GuarSpouseRiderFilename       );
    ascribe("CurrTermFilename"              , &product_data::CurrTermFilename              );
    ascribe("GuarTermFilename"              , &product_data::GuarTermFilename              );
    ascribe("GroupProxyFilename"            , &product_data::GroupProxyFilename            );
    ascribe("SevenPayFilename"              , &product_data::SevenPayFilename              );
    ascribe("TgtPremFilename"               , &product_data::TgtPremFilename               );
    ascribe("Irc7702QFilename"              , &product_data::Irc7702QFilename              );
    ascribe("PartialMortalityFilename"      , &product_data::PartialMortalityFilename      );
    ascribe("SubstdTblMultFilename"         , &product_data::SubstdTblMultFilename         );
    ascribe("CurrSpecAmtLoadFilename"       , &product_data::CurrSpecAmtLoadFilename       );
    ascribe("GuarSpecAmtLoadFilename"       , &product_data::GuarSpecAmtLoadFilename       );
    ascribe("PolicyForm"                    , &product_data::PolicyForm                    );
    ascribe("PolicyFormAlternative"         , &product_data::PolicyFormAlternative         );
    ascribe("PolicyMktgName"                , &product_data::PolicyMktgName                );
    ascribe("PolicyLegalName"               , &product_data::PolicyLegalName               );
    ascribe("InsCoShortName"                , &product_data::InsCoShortName                );
    ascribe("InsCoName"                     , &product_data::InsCoName                     );
    ascribe("InsCoAddr"                     , &product_data::InsCoAddr                     );
    ascribe("InsCoStreet"                   , &product_data::InsCoStreet                   );
    ascribe("InsCoPhone"                    , &product_data::InsCoPhone                    );
    ascribe("InsCoDomicile"                 , &product_data::InsCoDomicile                 );
    ascribe("MainUnderwriter"               , &product_data::MainUnderwriter               );
    ascribe("MainUnderwriterAddress"        , &product_data::MainUnderwriterAddress        );
    ascribe("CoUnderwriter"                 , &product_data::CoUnderwriter                 );
    ascribe("CoUnderwriterAddress"          , &product_data::CoUnderwriterAddress          );
    ascribe("AvName"                        , &product_data::AvName                        );
    ascribe("CsvName"                       , &product_data::CsvName                       );
    ascribe("CsvHeaderName"                 , &product_data::CsvHeaderName                 );
    ascribe("NoLapseProvisionName"          , &product_data::NoLapseProvisionName          );
    ascribe("ContractName"                  , &product_data::ContractName                  );
    ascribe("DboNameLevel"                  , &product_data::DboNameLevel                  );
    ascribe("DboNameIncreasing"             , &product_data::DboNameIncreasing             );
    ascribe("DboNameReturnOfPremium"        , &product_data::DboNameReturnOfPremium        );
    ascribe("AccountValueFootnote"          , &product_data::AccountValueFootnote          );
    ascribe("AttainedAgeFootnote"           , &product_data::AttainedAgeFootnote           );
    ascribe("CashSurrValueFootnote"         , &product_data::CashSurrValueFootnote         );
    ascribe("DeathBenefitFootnote"          , &product_data::DeathBenefitFootnote          );
    ascribe("InitialPremiumFootnote"        , &product_data::InitialPremiumFootnote        );
    ascribe("NetPremiumFootnote"            , &product_data::NetPremiumFootnote            );
    ascribe("GrossPremiumFootnote"          , &product_data::GrossPremiumFootnote          );
    ascribe("OutlayFootnote"                , &product_data::OutlayFootnote                );
    ascribe("PolicyYearFootnote"            , &product_data::PolicyYearFootnote            );
    ascribe("ADDTerseName"                  , &product_data::ADDTerseName                  );
    ascribe("InsurabilityTerseName"         , &product_data::InsurabilityTerseName         );
    ascribe("ChildTerseName"                , &product_data::ChildTerseName                );
    ascribe("SpouseTerseName"               , &product_data::SpouseTerseName               );
    ascribe("TermTerseName"                 , &product_data::TermTerseName                 );
    ascribe("WaiverTerseName"               , &product_data::WaiverTerseName               );
    ascribe("AccelBftRiderTerseName"        , &product_data::AccelBftRiderTerseName        );
    ascribe("OverloanRiderTerseName"        , &product_data::OverloanRiderTerseName        );
    ascribe("ADDFootnote"                   , &product_data::ADDFootnote                   );
    ascribe("ChildFootnote"                 , &product_data::ChildFootnote                 );
    ascribe("SpouseFootnote"                , &product_data::SpouseFootnote                );
    ascribe("TermFootnote"                  , &product_data::TermFootnote                  );
    ascribe("WaiverFootnote"                , &product_data::WaiverFootnote                );
    ascribe("AccelBftRiderFootnote"         , &product_data::AccelBftRiderFootnote         );
    ascribe("OverloanRiderFootnote"         , &product_data::OverloanRiderFootnote         );
    ascribe("GroupQuoteShortProductName"    , &product_data::GroupQuoteShortProductName    );
    ascribe("GroupQuoteIsNotAnOffer"        , &product_data::GroupQuoteIsNotAnOffer        );
    ascribe("GroupQuoteRidersFooter"        , &product_data::GroupQuoteRidersFooter        );
    ascribe("GroupQuotePolicyFormId"        , &product_data::GroupQuotePolicyFormId        );
    ascribe("GroupQuoteStateVariations"     , &product_data::GroupQuoteStateVariations     );
    ascribe("GroupQuoteProspectus"          , &product_data::GroupQuoteProspectus          );
    ascribe("GroupQuoteUnderwriter"         , &product_data::GroupQuoteUnderwriter         );
    ascribe("GroupQuoteBrokerDealer"        , &product_data::GroupQuoteBrokerDealer        );
    ascribe("GroupQuoteRubricMandatory"     , &product_data::GroupQuoteRubricMandatory     );
    ascribe("GroupQuoteRubricVoluntary"     , &product_data::GroupQuoteRubricVoluntary     );
    ascribe("GroupQuoteRubricFusion"        , &product_data::GroupQuoteRubricFusion        );
    ascribe("GroupQuoteFooterMandatory"     , &product_data::GroupQuoteFooterMandatory     );
    ascribe("GroupQuoteFooterVoluntary"     , &product_data::GroupQuoteFooterVoluntary     );
    ascribe("GroupQuoteFooterFusion"        , &product_data::GroupQuoteFooterFusion        );
    ascribe("MinimumPremiumFootnote"        , &product_data::MinimumPremiumFootnote        );
    ascribe("PremAllocationFootnote"        , &product_data::PremAllocationFootnote        );
    ascribe("InterestDisclaimer"            , &product_data::InterestDisclaimer            );
    ascribe("GuarMortalityFootnote"         , &product_data::GuarMortalityFootnote         );
    ascribe("ProductDescription"            , &product_data::ProductDescription            );
    ascribe("StableValueFootnote"           , &product_data::StableValueFootnote           );
    ascribe("NoVanishPremiumFootnote"       , &product_data::NoVanishPremiumFootnote       );
    ascribe("RejectPremiumFootnote"         , &product_data::RejectPremiumFootnote         );
    ascribe("ExpRatingFootnote"             , &product_data::ExpRatingFootnote             );
    ascribe("MortalityBlendFootnote"        , &product_data::MortalityBlendFootnote        );
    ascribe("HypotheticalRatesFootnote"     , &product_data::HypotheticalRatesFootnote     );
    ascribe("SalesLoadRefundFootnote"       , &product_data::SalesLoadRefundFootnote       );
    ascribe("NoLapseFootnote"               , &product_data::NoLapseFootnote               );
    ascribe("MarketValueAdjFootnote"        , &product_data::MarketValueAdjFootnote        );
    ascribe("ExchangeChargeFootnote0"       , &product_data::ExchangeChargeFootnote0       );
    ascribe("CurrentValuesFootnote"         , &product_data::CurrentValuesFootnote         );
    ascribe("DBOption1Footnote"             , &product_data::DBOption1Footnote             );
    ascribe("DBOption2Footnote"             , &product_data::DBOption2Footnote             );
    ascribe("ExpRatRiskChargeFootnote"      , &product_data::ExpRatRiskChargeFootnote      );
    ascribe("ExchangeChargeFootnote1"       , &product_data::ExchangeChargeFootnote1       );
    ascribe("FlexiblePremiumFootnote"       , &product_data::FlexiblePremiumFootnote       );
    ascribe("GuaranteedValuesFootnote"      , &product_data::GuaranteedValuesFootnote      );
    ascribe("CreditingRateFootnote"         , &product_data::CreditingRateFootnote         );
    ascribe("GrossRateFootnote"             , &product_data::GrossRateFootnote             );
    ascribe("NetRateFootnote"               , &product_data::NetRateFootnote               );
    ascribe("MecFootnote"                   , &product_data::MecFootnote                   );
    ascribe("GptFootnote"                   , &product_data::GptFootnote                   );
    ascribe("MidpointValuesFootnote"        , &product_data::MidpointValuesFootnote        );
    ascribe("SinglePremiumFootnote"         , &product_data::SinglePremiumFootnote         );
    ascribe("MonthlyChargesFootnote"        , &product_data::MonthlyChargesFootnote        );
    ascribe("UltCreditingRateFootnote"      , &product_data::UltCreditingRateFootnote      );
    ascribe("MaxNaarFootnote"               , &product_data::MaxNaarFootnote               );
    ascribe("PremTaxSurrChgFootnote"        , &product_data::PremTaxSurrChgFootnote        );
    ascribe("PolicyFeeFootnote"             , &product_data::PolicyFeeFootnote             );
    ascribe("AssetChargeFootnote"           , &product_data::AssetChargeFootnote           );
    ascribe("InvestmentIncomeFootnote"      , &product_data::InvestmentIncomeFootnote      );
    ascribe("IrrDbFootnote"                 , &product_data::IrrDbFootnote                 );
    ascribe("IrrCsvFootnote"                , &product_data::IrrCsvFootnote                );
    ascribe("MortalityChargesFootnote"      , &product_data::MortalityChargesFootnote      );
    ascribe("LoanAndWithdrawalFootnote"     , &product_data::LoanAndWithdrawalFootnote     );
    ascribe("LoanFootnote"                  , &product_data::LoanFootnote                  );
    ascribe("ImprimaturPresale"             , &product_data::ImprimaturPresale             );
    ascribe("ImprimaturPresaleComposite"    , &product_data::ImprimaturPresaleComposite    );
    ascribe("ImprimaturInforce"             , &product_data::ImprimaturInforce             );
    ascribe("ImprimaturInforceComposite"    , &product_data::ImprimaturInforceComposite    );
    ascribe("StateMarketingImprimatur"      , &product_data::StateMarketingImprimatur      );
    ascribe("InforceNonGuaranteedFootnote0" , &product_data::InforceNonGuaranteedFootnote0 );
    ascribe("InforceNonGuaranteedFootnote1" , &product_data::InforceNonGuaranteedFootnote1 );
    ascribe("InforceNonGuaranteedFootnote2" , &product_data::InforceNonGuaranteedFootnote2 );
    ascribe("InforceNonGuaranteedFootnote3" , &product_data::InforceNonGuaranteedFootnote3 );
    ascribe("NonGuaranteedFootnote"         , &product_data::NonGuaranteedFootnote         );
    ascribe("MonthlyChargesPaymentFootnote" , &product_data::MonthlyChargesPaymentFootnote );
    ascribe("SurrenderFootnote"             , &product_data::SurrenderFootnote             );
    ascribe("PortabilityFootnote"           , &product_data::PortabilityFootnote           );
    ascribe("FundRateFootnote"              , &product_data::FundRateFootnote              );
    ascribe("FundRateFootnote0"             , &product_data::FundRateFootnote0             );
    ascribe("FundRateFootnote1"             , &product_data::FundRateFootnote1             );
    ascribe("IssuingCompanyFootnote"        , &product_data::IssuingCompanyFootnote        );
    ascribe("SubsidiaryFootnote"            , &product_data::SubsidiaryFootnote            );
    ascribe("PlacementAgentFootnote"        , &product_data::PlacementAgentFootnote        );
    ascribe("MarketingNameFootnote"         , &product_data::MarketingNameFootnote         );
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20100402T1123Z
/// version 1: 20120616T1210Z

int product_data::class_version() const
{
    return 1;
}

std::string const& product_data::xml_root_name() const
{
    static std::string const s("policy");
    return s;
}

product_data::value_type product_data::fetch_element
    (xml::element const& e
    ) const
{
    value_type r;
    xml_serialize::from_xml(e, r);
    return r;
}

/// This override doesn't call redintegrate_ex_ante(); that wouldn't
/// make sense, because the underlying datatype is just a doublet of
/// strings, and strings can legitimately contain anything.

void product_data::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 // file_version
    )
{
    glossed_string& r = *member_cast<glossed_string>(operator[](name));
    xml_serialize::from_xml(e, r);
}

void product_data::write_element
    (xml::element&       parent
    ,std::string const&  name
    ) const
{
    glossed_string const& r = *member_cast<glossed_string>(operator[](name));
    xml_serialize::set_element(parent, name, r);
}

void product_data::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_leaf_name
    ) const
{
    ::write_proem(document, file_leaf_name);
}

bool product_data::is_detritus(std::string const& s) const
{
    static std::string const a[] =
        {"PresaleTrackingNumber"          // renamed to ImprimaturPresale
        ,"CompositeTrackingNumber"        // renamed to ImprimaturPresaleComposite
        ,"InforceTrackingNumber"          // renamed to ImprimaturInforce
        ,"InforceCompositeTrackingNumber" // renamed to ImprimaturInforceComposite
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return contains(v, s);
}

void product_data::redintegrate_ex_ante
    (int                file_version
    ,std::string const& // name
    ,value_type       & // value
    ) const
{
    if(class_version() == file_version)
        {
        return;
        }

    if(file_version < 2)
        {
        return;
        }
}

void product_data::redintegrate_ex_post
    (int                                     file_version
    ,std::map<std::string,value_type> const& detritus_map
    ,std::list<std::string>           const& residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    if(file_version < 1)
        {
        // Version 1 renamed these members:
        LMI_ASSERT(contains(residuary_names, "ImprimaturPresale"));
        LMI_ASSERT(contains(residuary_names, "ImprimaturPresaleComposite"));
        LMI_ASSERT(contains(residuary_names, "ImprimaturInforce"));
        LMI_ASSERT(contains(residuary_names, "ImprimaturInforceComposite"));
        ImprimaturPresale          = map_lookup(detritus_map, "PresaleTrackingNumber");
        ImprimaturPresaleComposite = map_lookup(detritus_map, "CompositeTrackingNumber");
        ImprimaturInforce          = map_lookup(detritus_map, "InforceTrackingNumber");
        ImprimaturInforceComposite = map_lookup(detritus_map, "InforceCompositeTrackingNumber");
        }
}

/// Create a product file for 'sample' products.
///
/// The 'sample' product DWISOTT. Its values, where specified at all
/// (rather than defaulted to empty strings), are intended to be
/// plausible, if perhaps whimsical.
///
/// The 'sample2' product is designed to facilitate replacement of
/// XSL with wxPdfDoc; it may be removed when that task is complete.
/// "*Filename" members are names of actual lmi product files, or
/// basenames of mortality-table databases, and their values must
/// nominate actual files. Member 'InsCoDomicile' is used to
/// determine retaliatory premium-tax rates, and must be a two-letter
/// USPS abbreviation. All other members represent text that is used
/// for formatting reports; in order to make 'sample2' more useful for
/// developing and testing reports, each has a nonempty value that is
/// its member name enclosed in braces ("{}"). Braces aren't otherwise
/// used in values, so any output substring like "{contract}" here:
///   "This {contract} provides valuable protection"
/// necessarily represents a substitutable value, while everything
/// else in a report is just literal text.

void product_data::WritePolFiles()
{
    product_data z;

    // 'sample' product

    // Names of lmi product files.
    z.DatabaseFilename           = glossed_string("sample.database");
    z.FundFilename               = glossed_string("sample.funds");
    z.RoundingFilename           = glossed_string("sample.rounding");
    z.TierFilename               = glossed_string("sample.strata");

    // Base names of mortality-table databases.
    z.CvatCorridorFilename       = glossed_string("sample");
    z.Irc7702NspFilename         = glossed_string("sample");
    z.CurrCOIFilename            = glossed_string("qx_cso");
    z.GuarCOIFilename            = glossed_string("qx_cso");
    z.WPFilename                 = glossed_string("sample");
    z.ADDFilename                = glossed_string("qx_ins", "Specimen gloss.");
    z.ChildRiderFilename         = glossed_string("qx_ins");
    z.CurrSpouseRiderFilename    = glossed_string("qx_ins");
    z.GuarSpouseRiderFilename    = glossed_string("qx_ins");
    z.CurrTermFilename           = glossed_string("sample");
    z.GuarTermFilename           = glossed_string("sample");
    z.GroupProxyFilename         = glossed_string("qx_ins");
    z.SevenPayFilename           = glossed_string("sample");
    z.TgtPremFilename            = glossed_string("sample");
    z.Irc7702QFilename           = glossed_string("qx_cso");
    z.PartialMortalityFilename   = glossed_string("qx_ann");
    z.SubstdTblMultFilename      = glossed_string("sample");
    z.CurrSpecAmtLoadFilename    = glossed_string("sample");
    z.GuarSpecAmtLoadFilename    = glossed_string("sample");

    // Other data that affect calculations.
    z.InsCoDomicile              = glossed_string("WI");

    // Substitutable strings.
    z.PolicyForm                 = glossed_string("UL32768-NY");
    z.PolicyFormAlternative      = glossed_string("UL32768-X");
    z.PolicyMktgName             = glossed_string("UL Supreme");
    z.PolicyLegalName            = glossed_string("Flexible Premium Adjustable Life Insurance Policy");
    z.InsCoShortName             = glossed_string("Superior Life");
    z.InsCoName                  = glossed_string("Superior Life Insurance Company");
    z.InsCoAddr                  = glossed_string("Superior, WI 12345");
    z.InsCoStreet                = glossed_string("246 Main Street");
    z.InsCoPhone                 = glossed_string("(800) 555-1212");
    z.MainUnderwriter            = glossed_string("Superior Securities");
    z.MainUnderwriterAddress     = glossed_string("246-M Main Street, Superior, WI 12345");
    z.CoUnderwriter              = glossed_string("Superior Investors");
    z.CoUnderwriterAddress       = glossed_string("246-C Main Street, Superior, WI 12345");
    z.AvName                     = glossed_string("Account");
    z.CsvName                    = glossed_string("Cash Surrender");
    z.CsvHeaderName              = glossed_string("Cash Surr");
    z.NoLapseProvisionName       = glossed_string("No-lapse Provision");
    z.ContractName               = glossed_string("contract"); // Alternatively, "policy" or "certificate".
    z.DboNameLevel               = glossed_string("A");
    z.DboNameIncreasing          = glossed_string("B");
    z.DboNameReturnOfPremium     = glossed_string("ROP");
    z.MarketingNameFootnote      = glossed_string("Policy form UL32768-NY is marketed as 'UL Supreme'.");

    z.ADDTerseName               = glossed_string("Accident");
    z.InsurabilityTerseName      = glossed_string("Insurability");
    z.ChildTerseName             = glossed_string("Child");
    z.SpouseTerseName            = glossed_string("Spouse");
    z.TermTerseName              = glossed_string("Term");
    z.WaiverTerseName            = glossed_string("Waiver");
    z.AccelBftRiderTerseName     = glossed_string("Acceleration");
    z.OverloanRiderTerseName     = glossed_string("Overloan");

    z.GroupQuoteShortProductName = glossed_string("UL SUPREME®");
    z.GroupQuoteIsNotAnOffer     = glossed_string("This is not an offer of insurance.");
    z.GroupQuoteRidersFooter     = glossed_string("Available riders: accident and waiver.");
    z.GroupQuotePolicyFormId     = glossed_string("Policy form UL32768-NY is a flexible premium contract.");
    z.GroupQuoteStateVariations  = glossed_string("Not available in all states.");
    z.GroupQuoteProspectus       = glossed_string("Read the prospectus carefully.");
    z.GroupQuoteUnderwriter      = glossed_string("Securities underwritten by Superior Securities.");
    z.GroupQuoteBrokerDealer     = glossed_string("Securities offered through Superior Brokerage.");
    z.GroupQuoteRubricMandatory  = glossed_string("Mandatory");
    z.GroupQuoteRubricVoluntary  = glossed_string("Voluntary");
    z.GroupQuoteRubricFusion     = glossed_string("Fusion");
    z.GroupQuoteFooterMandatory  = glossed_string("The employer pays all premiums.");
    z.GroupQuoteFooterVoluntary  = glossed_string("The employee pays all premiums.");
    z.GroupQuoteFooterFusion     = glossed_string("The employer and employee pay their respective premiums.");

    z.save(AddDataDir("sample.policy"));

    // 'sample2' product

    typedef std::vector<std::string>::const_iterator svci;
    for(svci i = z.member_names().begin(); i != z.member_names().end(); ++i)
        {
        z[*i] = '{' + *i + '}';
        }

    // Names of lmi product files.
    z.DatabaseFilename           = glossed_string("sample.database");
    z.FundFilename               = glossed_string("sample.funds");
    z.RoundingFilename           = glossed_string("sample.rounding");
    z.TierFilename               = glossed_string("sample.strata");

    // Base names of mortality-table databases.
    z.CvatCorridorFilename       = glossed_string("sample");
    z.Irc7702NspFilename         = glossed_string("sample");
    z.CurrCOIFilename            = glossed_string("qx_cso");
    z.GuarCOIFilename            = glossed_string("qx_cso");
    z.WPFilename                 = glossed_string("sample");
    z.ADDFilename                = glossed_string("qx_ins", "Specimen gloss.");
    z.ChildRiderFilename         = glossed_string("qx_ins");
    z.CurrSpouseRiderFilename    = glossed_string("qx_ins");
    z.GuarSpouseRiderFilename    = glossed_string("qx_ins");
    z.CurrTermFilename           = glossed_string("sample");
    z.GuarTermFilename           = glossed_string("sample");
    z.GroupProxyFilename         = glossed_string("qx_ins");
    z.SevenPayFilename           = glossed_string("sample");
    z.TgtPremFilename            = glossed_string("sample");
    z.Irc7702QFilename           = glossed_string("qx_cso");
    z.PartialMortalityFilename   = glossed_string("qx_ann");
    z.SubstdTblMultFilename      = glossed_string("sample");
    z.CurrSpecAmtLoadFilename    = glossed_string("sample");
    z.GuarSpecAmtLoadFilename    = glossed_string("sample");

    // Other data that affect calculations.
    z.InsCoDomicile              = glossed_string("WI");

    z.save(AddDataDir("sample2.policy"));
}

/// Load from file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void load(product_data& z, fs::path const& path)
{
    z.xml_serializable<product_data>::load(path);
}

/// Save to file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void save(product_data const& z, fs::path const& path)
{
    z.xml_serializable<product_data>::save(path);
}

