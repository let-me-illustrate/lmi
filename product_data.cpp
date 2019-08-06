// Product data representable as strings.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "product_data.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "map_lookup.hpp"
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
        alarum()
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

glossed_string::glossed_string
    (std::string const& datum
    ,std::string const& gloss
    )
    :datum_ {datum}
    ,gloss_ {gloss}
{}

glossed_string& glossed_string::operator=(std::string const& s)
{
    datum_ = s;
    gloss_ = "";
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

/// Default ctor, used only by derived classes and friends.

product_data::product_data()
{
    ascribe_members();
}

/// Construct from product name.
///
/// The argument is a string (typically Input::ProductName) such as
/// 'sample'. The appropriate extension and path are added here to
/// produce a filepath.
///
/// Somewhat arbitrarily, forbid '.' in product names. There's no real
/// need to allow that, and it would complicate the code. A product
/// name like "ul.with.variable.funds" could too easily be mistaken
/// for a '.funds' file. The boost filesystem portability guidelines
/// suggest "Do not use more that one period in a file name", and
/// extensions are added to product names to create file names.

product_data::product_data(std::string const& product_name)
{
    ascribe_members();

    fs::path path(product_name);
    LMI_ASSERT(product_name == fs::basename(path));
    path = fs::change_extension(path, ".policy");
    load(AddDataDir(path.string()));
}

product_data::product_data(product_data const& z)
    :xml_serializable  <product_data> {}
    ,MemberSymbolTable <product_data> {}
{
    ascribe_members();
    MemberSymbolTable<product_data>::assign(z);
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.

product_data::~product_data() = default;

/// Principal string datum (without gloss) for named member.

std::string const& product_data::datum(std::string const& name) const
{
    return member_cast<glossed_string>(operator[](name))->datum();
}

/// Reference to named member.

glossed_string& product_data::item(std::string const& name)
{
    return *exact_cast<glossed_string>(operator[](name));
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
    ascribe("DboName"                       , &product_data::DboName                       );
    ascribe("DboNameLevel"                  , &product_data::DboNameLevel                  );
    ascribe("DboNameIncreasing"             , &product_data::DboNameIncreasing             );
    ascribe("DboNameReturnOfPremium"        , &product_data::DboNameReturnOfPremium        );
    ascribe("DboNameMinDeathBenefit"        , &product_data::DboNameMinDeathBenefit        );
    ascribe("GenAcctName"                   , &product_data::GenAcctName                   );
    ascribe("GenAcctNameElaborated"         , &product_data::GenAcctNameElaborated         );
    ascribe("SepAcctName"                   , &product_data::SepAcctName                   );
    ascribe("SpecAmtName"                   , &product_data::SpecAmtName                   );
    ascribe("SpecAmtNameElaborated"         , &product_data::SpecAmtNameElaborated         );
    ascribe("UwBasisMedical"                , &product_data::UwBasisMedical                );
    ascribe("UwBasisParamedical"            , &product_data::UwBasisParamedical            );
    ascribe("UwBasisNonmedical"             , &product_data::UwBasisNonmedical             );
    ascribe("UwBasisSimplified"             , &product_data::UwBasisSimplified             );
    ascribe("UwBasisGuaranteed"             , &product_data::UwBasisGuaranteed             );
    ascribe("UwClassPreferred"              , &product_data::UwClassPreferred              );
    ascribe("UwClassStandard"               , &product_data::UwClassStandard               );
    ascribe("UwClassRated"                  , &product_data::UwClassRated                  );
    ascribe("UwClassUltra"                  , &product_data::UwClassUltra                  );
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
    ascribe("DBOption3Footnote"             , &product_data::DBOption3Footnote             );
    ascribe("MinDeathBenefitFootnote"       , &product_data::MinDeathBenefitFootnote       );
    ascribe("ExpRatRiskChargeFootnote"      , &product_data::ExpRatRiskChargeFootnote      );
    ascribe("ExchangeChargeFootnote1"       , &product_data::ExchangeChargeFootnote1       );
    ascribe("FlexiblePremiumFootnote"       , &product_data::FlexiblePremiumFootnote       );
    ascribe("GuaranteedValuesFootnote"      , &product_data::GuaranteedValuesFootnote      );
    ascribe("CreditingRateFootnote"         , &product_data::CreditingRateFootnote         );
    ascribe("GuaranteedCreditRateFootnote"  , &product_data::GuaranteedCreditRateFootnote  );
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
    static std::vector<std::string> const v
        {"PresaleTrackingNumber"          // renamed to ImprimaturPresale
        ,"CompositeTrackingNumber"        // renamed to ImprimaturPresaleComposite
        ,"InforceTrackingNumber"          // renamed to ImprimaturInforce
        ,"InforceCompositeTrackingNumber" // renamed to ImprimaturInforceComposite
        };
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

namespace
{
class sample : public product_data {public: sample();};

class sample2 : public sample {public: sample2();};

class sample2naic  : public sample2 {public: sample2naic ();};
class sample2finra : public sample2 {public: sample2finra();};
class sample2prosp : public sample2 {public: sample2prosp();};
class sample2gpp   : public sample2 {public: sample2gpp  ();};
class sample2ipp   : public sample2 {public: sample2ipp  ();};
class sample2xyz   : public sample2 {public: sample2xyz  ();};

/// The 'sample' product DWISOTT. Its values, where specified at all
/// (rather than defaulted to empty strings), are intended to be
/// plausible, if perhaps whimsical.

sample::sample()
{
    // Names of lmi product files.
    item("DatabaseFilename")           = glossed_string("sample.database");
    item("FundFilename")               = glossed_string("sample.funds");
    item("RoundingFilename")           = glossed_string("sample.rounding");
    item("TierFilename")               = glossed_string("sample.strata");

    // Base names of mortality-table databases.
    item("CvatCorridorFilename")       = glossed_string("sample");
    item("Irc7702NspFilename")         = glossed_string("sample");
    item("CurrCOIFilename")            = glossed_string("qx_cso");
    item("GuarCOIFilename")            = glossed_string("qx_cso");
    item("WPFilename")                 = glossed_string("sample");
    item("ADDFilename")                = glossed_string("qx_ins", "Specimen gloss.");
    item("ChildRiderFilename")         = glossed_string("qx_ins");
    item("CurrSpouseRiderFilename")    = glossed_string("qx_ins");
    item("GuarSpouseRiderFilename")    = glossed_string("qx_ins");
    item("CurrTermFilename")           = glossed_string("sample");
    item("GuarTermFilename")           = glossed_string("sample");
    item("GroupProxyFilename")         = glossed_string("qx_ins");
    item("SevenPayFilename")           = glossed_string("sample");
    item("TgtPremFilename")            = glossed_string("sample");
    item("Irc7702QFilename")           = glossed_string("qx_cso");
    item("PartialMortalityFilename")   = glossed_string("qx_ann");
    item("SubstdTblMultFilename")      = glossed_string("sample");
    item("CurrSpecAmtLoadFilename")    = glossed_string("sample");
    item("GuarSpecAmtLoadFilename")    = glossed_string("sample");

    // Other data that affect calculations.
    item("InsCoDomicile")              = glossed_string("WI");

    // Substitutable strings.
    item("PolicyForm")                 = glossed_string("UL32768-NY");
    item("PolicyFormAlternative")      = glossed_string("UL32768-X");
    item("PolicyMktgName")             = glossed_string("UL Supreme");
    item("PolicyLegalName")            = glossed_string("Flexible Premium Adjustable Life Insurance Policy");
    item("InsCoShortName")             = glossed_string("Superior Life");
    item("InsCoName")                  = glossed_string("Superior Life Insurance Company");
    item("InsCoAddr")                  = glossed_string("Superior, WI 12345");
    item("InsCoStreet")                = glossed_string("246 Main Street");
    item("InsCoPhone")                 = glossed_string("(800) 555-1212");
    item("MainUnderwriter")            = glossed_string("Superior Securities");
    item("MainUnderwriterAddress")     = glossed_string("246-M Main Street, Superior, WI 12345");
    item("CoUnderwriter")              = glossed_string("Superior Investors");
    item("CoUnderwriterAddress")       = glossed_string("246-C Main Street, Superior, WI 12345");
    item("AvName")                     = glossed_string("Account");
    item("CsvName")                    = glossed_string("Cash Surrender");
    item("CsvHeaderName")              = glossed_string("Cash Surr");
    item("NoLapseProvisionName")       = glossed_string("No-lapse Provision");
    item("ContractName")               = glossed_string("contract"); // Alternatively, "policy" or "certificate".
    item("DboName")                    = glossed_string("Death Benefit Option");
    item("DboNameLevel")               = glossed_string("A");
    item("DboNameIncreasing")          = glossed_string("B");
    item("DboNameReturnOfPremium")     = glossed_string("ROP");
    item("DboNameMinDeathBenefit")     = glossed_string("MDB");
    item("GenAcctName")                = glossed_string("General Account");
    item("GenAcctNameElaborated")      = glossed_string("General Account (GA)");
    item("SepAcctName")                = glossed_string("Separate Account");
    item("SpecAmtName")                = glossed_string("Specified Amount");
    item("SpecAmtNameElaborated")      = glossed_string("Specified (Face) Amount");
    item("UwBasisMedical")             = glossed_string("Medical");
    item("UwBasisParamedical")         = glossed_string("Paramedical");
    item("UwBasisNonmedical")          = glossed_string("Nonmedical");
    item("UwBasisSimplified")          = glossed_string("Simplified Issue");
    item("UwBasisGuaranteed")          = glossed_string("Guaranteed Issue");
    item("UwClassPreferred")           = glossed_string("Preferred");
    item("UwClassStandard")            = glossed_string("Standard");
    item("UwClassRated")               = glossed_string("Rated");
    item("UwClassUltra")               = glossed_string("Ultrapreferred");
    item("MarketingNameFootnote")      = glossed_string("Policy form UL32768-NY is marketed as 'UL Supreme'.");

    item("ADDTerseName")               = glossed_string("Accident");
    item("InsurabilityTerseName")      = glossed_string("Insurability");
    item("ChildTerseName")             = glossed_string("Child");
    item("SpouseTerseName")            = glossed_string("Spouse");
    item("TermTerseName")              = glossed_string("Term");
    item("WaiverTerseName")            = glossed_string("Waiver");
    item("AccelBftRiderTerseName")     = glossed_string("Acceleration");
    item("OverloanRiderTerseName")     = glossed_string("Overloan");

    item("GroupQuoteShortProductName") = glossed_string("UL SUPREME®");
    item("GroupQuoteIsNotAnOffer")     = glossed_string("This is not an offer of insurance.");
    item("GroupQuoteRidersFooter")     = glossed_string("Available riders: accident and waiver.");
    item("GroupQuotePolicyFormId")     = glossed_string("Policy form UL32768-NY is a flexible premium contract.");
    item("GroupQuoteStateVariations")  = glossed_string("Not available in all states.");
    item("GroupQuoteProspectus")       = glossed_string("Read the prospectus carefully.");
    item("GroupQuoteUnderwriter")      = glossed_string("Securities underwritten by Superior Securities.");
    item("GroupQuoteBrokerDealer")     = glossed_string("Securities offered through Superior Brokerage.");
    item("GroupQuoteRubricMandatory")  = glossed_string("Mandatory");
    item("GroupQuoteRubricVoluntary")  = glossed_string("Voluntary");
    item("GroupQuoteRubricFusion")     = glossed_string("Fusion");
    item("GroupQuoteFooterMandatory")  = glossed_string("The employer pays all premiums.");
    item("GroupQuoteFooterVoluntary")  = glossed_string("The employee pays all premiums.");
    item("GroupQuoteFooterFusion")     = glossed_string("The employer and employee pay their respective premiums.");
}

/// The 'sample2*' products are designed to facilitate testing.
/// There is one for each supported ledger type:
///   sample2naic  mce_ill_reg
///   sample2finra mce_finra
///   sample2prosp mce_prospectus_abeyed ['emit_test_data' only]
///   sample2gpp   mce_group_private_placement
///   sample2ipp   mce_individual_private_placement
/// and one for exotica:
///   sample2xyz   mce_finra
///
/// "*Filename" members are names of actual lmi product files, or
/// basenames of mortality-table databases, and their values must
/// nominate actual files. Member 'InsCoDomicile' is used to
/// determine retaliatory premium-tax rates, and must be a two-letter
/// USPS abbreviation. All other members represent text that is used
/// for report formatting; in order to make 'sample2*' more useful for
/// developing and testing reports, each has a nonempty value that is
/// its member name enclosed in braces ("{}"). Braces aren't otherwise
/// used in values, so any output substring like "{contract}" here:
///   "This {contract} provides valuable protection"
/// necessarily represents a substitutable value, while everything
/// else in a report is just literal text.

sample2::sample2()
{
    for(auto const& i : member_names())
        {
        operator[](i) = '{' + i + '}';
        }

    // Names of lmi product files.
    item("DatabaseFilename")           = glossed_string("sample.database");
    item("FundFilename")               = glossed_string("sample.funds");
    item("RoundingFilename")           = glossed_string("sample.rounding");
    item("TierFilename")               = glossed_string("sample.strata");

    // Base names of mortality-table databases.
    item("CvatCorridorFilename")       = glossed_string("sample");
    item("Irc7702NspFilename")         = glossed_string("sample");
    item("CurrCOIFilename")            = glossed_string("qx_cso");
    item("GuarCOIFilename")            = glossed_string("qx_cso");
    item("WPFilename")                 = glossed_string("sample");
    item("ADDFilename")                = glossed_string("qx_ins", "Specimen gloss.");
    item("ChildRiderFilename")         = glossed_string("qx_ins");
    item("CurrSpouseRiderFilename")    = glossed_string("qx_ins");
    item("GuarSpouseRiderFilename")    = glossed_string("qx_ins");
    item("CurrTermFilename")           = glossed_string("sample");
    item("GuarTermFilename")           = glossed_string("sample");
    item("GroupProxyFilename")         = glossed_string("qx_ins");
    item("SevenPayFilename")           = glossed_string("sample");
    item("TgtPremFilename")            = glossed_string("sample");
    item("Irc7702QFilename")           = glossed_string("qx_cso");
    item("PartialMortalityFilename")   = glossed_string("qx_ann");
    item("SubstdTblMultFilename")      = glossed_string("sample");
    item("CurrSpecAmtLoadFilename")    = glossed_string("sample");
    item("GuarSpecAmtLoadFilename")    = glossed_string("sample");

    // Other data that affect calculations.
    item("InsCoDomicile")              = glossed_string("WI");
}

sample2naic::sample2naic()
{
}

sample2finra::sample2finra()
{
    item("DatabaseFilename")           = glossed_string("sample2finra.database");
}

sample2prosp::sample2prosp()
{
    item("DatabaseFilename")           = glossed_string("sample2prosp.database");
}

sample2gpp::sample2gpp()
{
    item("DatabaseFilename")           = glossed_string("sample2gpp.database");
}

/// This specimen product
///   https://lists.nongnu.org/archive/html/lmi/2018-09/msg00039.html
/// | has deliberately overlong footnotes
/// for pagination testing.

sample2ipp::sample2ipp()
{
    item("DatabaseFilename")           = glossed_string("sample2ipp.database");
    item("IrrDbFootnote") = glossed_string
        ("The \"Red Death\" had long devastated the country. No pestilence"
         " had ever been so fatal, or so hideous. Blood was its Avatar and"
         " its seal--the redness and the horror of blood. There were sharp"
         " pains, and sudden dizziness, and then profuse bleeding at the"
         " pores, with dissolution. The scarlet stains upon the body and"
         " especially upon the face of the victim, were the pest ban which"
         " shut him out from the aid and from the sympathy of his fellow-men."
         " And the whole seizure, progress and termination of the disease,"
         " were the incidents of half an hour."
        );
    item("IrrCsvFootnote") = glossed_string
        ("But the Prince Prospero was happy and dauntless and sagacious. When"
         " his dominions were half depopulated, he summoned to his presence a"
         " thousand hale and light-hearted friends from among the knights and"
         " dames of his court, and with these retired to the deep seclusion"
         " of one of his castellated abbeys. This was an extensive and"
         " magnificent structure, the creation of the prince's own eccentric"
         " yet august taste. A strong and lofty wall girdled it in. This wall"
         " had gates of iron. The courtiers, having entered, brought furnaces"
         " and massy hammers and welded the bolts. They resolved to leave"
         " means neither of ingress nor egress to the sudden impulses of"
         " despair or of frenzy from within. The abbey was amply provisioned."
         " With such precautions the courtiers might bid defiance to contagion."
         " The external world could take care of itself. In the meantime it"
         " was folly to grieve, or to think. The prince had provided all the"
         " appliances of pleasure. There were buffoons, there were"
         " improvisatori, there were ballet-dancers, there were musicians,"
         " there was Beauty, there was wine. All these and security were"
         " within. Without was the \"Red Death\"."
        );
    item("MortalityChargesFootnote") = glossed_string
        ("It was towards the close of the fifth or sixth month of his"
         " seclusion, and while the pestilence raged most furiously abroad,"
         " that the Prince Prospero entertained his thousand friends at a"
         " masked ball of the most unusual magnificence."
        );
    item("PolicyYearFootnote") = glossed_string
        ("It was a voluptuous scene, that masquerade. But first let me tell"
         " of the rooms in which it was held. These were seven--an imperial"
         " suite. In many palaces, however, such suites form a long and"
         " straight vista, while the folding doors slide back nearly to the"
         " walls on either hand, so that the view of the whole extent is"
         " scarcely impeded. Here the case was very different, as might have"
         " been expected from the duke's love of the _bizarre_. The apartments"
         " were so irregularly disposed that the vision embraced but little"
         " more than one at a time. There was a sharp turn at every twenty or"
         " thirty yards, and at each turn a novel effect. To the right and"
         " left, in the middle of each wall, a tall and narrow Gothic window"
         " looked out upon a closed corridor which pursued the windings of the"
         " suite. These windows were of stained glass whose color varied in"
         " accordance with the prevailing hue of the decorations of the"
         " chamber into which it opened. That at the eastern extremity was"
         " hung, for example in blue--and vividly blue were its windows. The"
         " second chamber was purple in its ornaments and tapestries, and here"
         " the panes were purple. The third was green throughout, and so were"
         " the casements. The fourth was furnished and lighted with orange--the"
         " fifth with white--the sixth with violet. The seventh apartment was"
         " closely shrouded in black velvet tapestries that hung all over the"
         " ceiling and down the walls, falling in heavy folds upon a carpet of"
         " the same material and hue. But in this chamber only, the color of"
         " the windows failed to correspond with the decorations. The panes"
         " here were scarlet--a deep blood color. Now in no one of the seven"
         " apartments was there any lamp or candelabrum, amid the profusion of"
         " golden ornaments that lay scattered to and fro or depended from the"
         " roof. There was no light of any kind emanating from lamp or candle"
         " within the suite of chambers. But in the corridors that followed the"
         " suite, there stood, opposite to each window, a heavy tripod, bearing"
         " a brazier of fire, that projected its rays through the tinted glass"
         " and so glaringly illumined the room. And thus were produced a"
         " multitude of gaudy and fantastic appearances. But in the western or"
         " black chamber the effect of the fire-light that streamed upon the"
         " dark hangings through the blood-tinted panes, was ghastly in the"
         " extreme, and produced so wild a look upon the countenances of those"
         " who entered, that there were few of the company bold enough to set"
         " foot within its precincts at all."
        );
}

sample2xyz::sample2xyz()
{
    item("DatabaseFilename")           = glossed_string("sample2xyz.database");
}
} // Unnamed namespace.

/// Create product files for 'sample' products.

void product_data::write_policy_files()
{
    sample      ().save(AddDataDir("sample.policy"));
    sample2naic ().save(AddDataDir("sample2naic.policy"));
    sample2finra().save(AddDataDir("sample2finra.policy"));
    sample2prosp().save(AddDataDir("sample2prosp.policy"));
    sample2gpp  ().save(AddDataDir("sample2gpp.policy"));
    sample2ipp  ().save(AddDataDir("sample2ipp.policy"));
    sample2xyz  ().save(AddDataDir("sample2xyz.policy"));
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
