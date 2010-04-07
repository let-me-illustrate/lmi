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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "product_data.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp" // AddDataDir()
#include "miscellany.hpp"     // lmi_array_size()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>          // std::find()
#include <vector>

template class xml_serializable<product_data>;

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

/// Enregister certain data members for access via any_member<>[].

void product_data::ascribe_members()
{
    ascribe("DatabaseFilename"              , &product_data::DatabaseFilename              );
    ascribe("FundFilename"                  , &product_data::FundFilename                  );
    ascribe("RoundingFilename"              , &product_data::RoundingFilename              );
    ascribe("TierFilename"                  , &product_data::TierFilename                  );
    ascribe("CorridorFilename"              , &product_data::CorridorFilename              );
    ascribe("CurrCOIFilename"               , &product_data::CurrCOIFilename               );
    ascribe("GuarCOIFilename"               , &product_data::GuarCOIFilename               );
    ascribe("WPFilename"                    , &product_data::WPFilename                    );
    ascribe("ADDFilename"                   , &product_data::ADDFilename                   );
    ascribe("ChildRiderFilename"            , &product_data::ChildRiderFilename            );
    ascribe("CurrSpouseRiderFilename"       , &product_data::CurrSpouseRiderFilename       );
    ascribe("GuarSpouseRiderFilename"       , &product_data::GuarSpouseRiderFilename       );
    ascribe("CurrTermFilename"              , &product_data::CurrTermFilename              );
    ascribe("GuarTermFilename"              , &product_data::GuarTermFilename              );
    ascribe("TableYFilename"                , &product_data::TableYFilename                );
    ascribe("PremTaxFilename"               , &product_data::PremTaxFilename               );
    ascribe("TAMRA7PayFilename"             , &product_data::TAMRA7PayFilename             );
    ascribe("TgtPremFilename"               , &product_data::TgtPremFilename               );
    ascribe("IRC7702Filename"               , &product_data::IRC7702Filename               );
    ascribe("Gam83Filename"                 , &product_data::Gam83Filename                 );
    ascribe("SubstdTblMultFilename"         , &product_data::SubstdTblMultFilename         );
    ascribe("CurrSpecAmtLoadFilename"       , &product_data::CurrSpecAmtLoadFilename       );
    ascribe("GuarSpecAmtLoadFilename"       , &product_data::GuarSpecAmtLoadFilename       );
    ascribe("PolicyForm"                    , &product_data::PolicyForm                    );
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
    ascribe("AccountValueFootnote"          , &product_data::AccountValueFootnote          );
    ascribe("AttainedAgeFootnote"           , &product_data::AttainedAgeFootnote           );
    ascribe("CashSurrValueFootnote"         , &product_data::CashSurrValueFootnote         );
    ascribe("DeathBenefitFootnote"          , &product_data::DeathBenefitFootnote          );
    ascribe("InitialPremiumFootnote"        , &product_data::InitialPremiumFootnote        );
    ascribe("NetPremiumFootnote"            , &product_data::NetPremiumFootnote            );
    ascribe("OutlayFootnote"                , &product_data::OutlayFootnote                );
    ascribe("PolicyYearFootnote"            , &product_data::PolicyYearFootnote            );
    ascribe("ADDFootnote"                   , &product_data::ADDFootnote                   );
    ascribe("ChildFootnote"                 , &product_data::ChildFootnote                 );
    ascribe("SpouseFootnote"                , &product_data::SpouseFootnote                );
    ascribe("TermFootnote"                  , &product_data::TermFootnote                  );
    ascribe("WaiverFootnote"                , &product_data::WaiverFootnote                );
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
    ascribe("MecFootnote"                   , &product_data::MecFootnote                   );
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
    ascribe("PresaleTrackingNumber"         , &product_data::PresaleTrackingNumber         );
    ascribe("CompositeTrackingNumber"       , &product_data::CompositeTrackingNumber       );
    ascribe("InforceTrackingNumber"         , &product_data::InforceTrackingNumber         );
    ascribe("InforceCompositeTrackingNumber", &product_data::InforceCompositeTrackingNumber);
    ascribe("InforceNonGuaranteedFootnote0" , &product_data::InforceNonGuaranteedFootnote0 );
    ascribe("InforceNonGuaranteedFootnote1" , &product_data::InforceNonGuaranteedFootnote1 );
    ascribe("InforceNonGuaranteedFootnote2" , &product_data::InforceNonGuaranteedFootnote2 );
    ascribe("InforceNonGuaranteedFootnote3" , &product_data::InforceNonGuaranteedFootnote3 );
    ascribe("NonGuaranteedFootnote"         , &product_data::NonGuaranteedFootnote         );
    ascribe("MonthlyChargesPaymentFootnote" , &product_data::MonthlyChargesPaymentFootnote );
}

/// Serial number of this class's xml version.
///
/// version 0: 20100402T1123Z

int product_data::class_version() const
{
    return 0;
}

std::string product_data::xml_root_name() const
{
    return "policy";
}

bool product_data::is_detritus(std::string const& s) const
{
    static std::string const a[] =
        {"Remove this string when adding the first removed entity."
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return v.end() != std::find(v.begin(), v.end(), s);
}

std::string product_data::redintegrate_ex_ante
    (int                file_version
    ,std::string const& // name
    ,std::string const& value
    ) const
{
    if(class_version() == file_version)
        {
        return value;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        return value; // Stifle compiler warning.
        }
}

void product_data::redintegrate_ex_post
    (int                                file_version
    ,std::map<std::string, std::string> // detritus_map
    ,std::list<std::string>             // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        }
}

void product_data::redintegrate_ad_terminum()
{
}

/// Create a product file for the 'sample' product.
///
/// Only the most crucial members are explicitly initialized. For the
/// rest, default (empty) strings are good enough for 'sample'.

void product_data::WritePolFiles()
{
    product_data z;

    z.DatabaseFilename               = "sample.db4";
    z.FundFilename                   = "sample.funds";
    z.RoundingFilename               = "sample.rounding";
    z.TierFilename                   = "sample.strata";

    z.CorridorFilename               = "sample";
    z.CurrCOIFilename                = "qx_cso";
    z.GuarCOIFilename                = "qx_cso";
    z.WPFilename                     = "sample";
    z.ADDFilename                    = "qx_ins";
    z.ChildRiderFilename             = "qx_ins";
    z.CurrSpouseRiderFilename        = "qx_ins";
    z.GuarSpouseRiderFilename        = "qx_ins";
    z.CurrTermFilename               = "qx_cso";
    z.GuarTermFilename               = "qx_cso";
    z.TableYFilename                 = "qx_ins";
    z.PremTaxFilename                = "sample";
    z.TAMRA7PayFilename              = "sample";
    z.TgtPremFilename                = "sample";
    z.IRC7702Filename                = "qx_cso";
    z.Gam83Filename                  = "qx_ann";
    z.SubstdTblMultFilename          = "sample";
    z.CurrSpecAmtLoadFilename        = "sample";
    z.GuarSpecAmtLoadFilename        = "sample";
    z.PolicyForm                     = "UL32768-NY";
    z.PolicyMktgName                 = "UL Supreme";
    z.PolicyLegalName = "Flexible Premium Adjustable Life Insurance Policy";
    z.InsCoShortName                 = "Superior Life";
    z.InsCoName                      = "Superior Life Insurance Company";
    z.InsCoAddr                      = "Superior, WI 12345";
    z.InsCoStreet                    = "246 Main Street";
    z.InsCoPhone                     = "(800) 555-1212";
    z.InsCoDomicile                  = "WI";
    z.MainUnderwriter                = "Superior Securities";
    z.MainUnderwriterAddress         = "246-M Main Street, Superior, WI 12345";
    z.CoUnderwriter                  = "Superior Investors";
    z.CoUnderwriterAddress           = "246-C Main Street, Superior, WI 12345";
    z.AvName                         = "Account";
    z.CsvName                        = "Cash Surrender";
    z.CsvHeaderName                  = "Cash Surr";
    z.NoLapseProvisionName           = "No-lapse Provision";

    z.save(AddDataDir("sample.policy"));
}

