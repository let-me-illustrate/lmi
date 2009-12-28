// Product data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_proddata.cpp,v 1.21 2009-02-18 22:12:11 wboutin Exp $

// This class contains names of files containing a product's tables as well
// as strings that are the same for all instances of that product.

// The tables are in SOA table manager binary format.

// The (SOA) table numbers are in the database. To avoid circularity, first
// instantiate this class, then instantiate the database before reading any
// tables with the default methods that look to the database for table numbers.
// This is handled automatically in class BasicValues, where those methods are.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_proddata.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>
#include <string>

//============================================================================
TProductData::TProductData()
{
}

//============================================================================
TProductData::~TProductData()
{
}

//============================================================================
TProductData::TProductData(std::string const& a_Filename)
{
    Init(a_Filename);
}

//============================================================================
void TProductData::Init(std::string const& a_Filename)
{
    fs::path path(a_Filename);
    LMI_ASSERT(a_Filename == path.leaf());
    path = fs::change_extension(path, ".pol");
    Read(AddDataDir(path.string()));
}

//============================================================================
void TProductData::Read(std::string const& a_Filename)
{
    if(access(a_Filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << a_Filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }
    std::ifstream is(a_Filename.c_str());

    std::getline(is, DatabaseFilename,              '\n');
    std::getline(is, FundFilename,                  '\n');
    std::getline(is, CorridorFilename,              '\n');
    std::getline(is, CurrCOIFilename,               '\n');
    std::getline(is, GuarCOIFilename,               '\n');
    std::getline(is, WPFilename,                    '\n');
    std::getline(is, ADDFilename,                   '\n');
    std::getline(is, ChildRiderFilename,            '\n');
    std::getline(is, CurrSpouseRiderFilename,       '\n');
    std::getline(is, GuarSpouseRiderFilename,       '\n');
    std::getline(is, CurrTermFilename,              '\n');
    std::getline(is, GuarTermFilename,              '\n');
    std::getline(is, TableYFilename,                '\n');
    std::getline(is, PremTaxFilename,               '\n');
    std::getline(is, TAMRA7PayFilename,             '\n');
    std::getline(is, TgtPremFilename,               '\n');
    std::getline(is, IRC7702Filename,               '\n');
    std::getline(is, Gam83Filename,                 '\n');
    std::getline(is, SubstdTblMultFilename,         '\n');
    std::getline(is, CurrSpecAmtLoadFilename,       '\n');
    std::getline(is, GuarSpecAmtLoadFilename,       '\n');
    std::getline(is, RoundingFilename,              '\n');
    std::getline(is, TierFilename,                  '\n');
    std::getline(is, PolicyForm,                    '\n');
    std::getline(is, PolicyMktgName,                '\n');
    std::getline(is, PolicyLegalName,               '\n');
    std::getline(is, InsCoShortName,                '\n');
    std::getline(is, InsCoName,                     '\n');
    std::getline(is, InsCoAddr,                     '\n');
    std::getline(is, InsCoStreet,                   '\n');
    std::getline(is, InsCoPhone,                    '\n');
    std::getline(is, InsCoDomicile,                 '\n');
    std::getline(is, MainUnderwriter,               '\n');
    std::getline(is, MainUnderwriterAddress,        '\n');
    std::getline(is, CoUnderwriter,                 '\n');
    std::getline(is, CoUnderwriterAddress,          '\n');
    std::getline(is, AvName,                        '\n');
    std::getline(is, CsvName,                       '\n');
    std::getline(is, CsvHeaderName,                 '\n');
    std::getline(is, NoLapseProvisionName,          '\n');
    std::getline(is, InterestDisclaimer,            '\n');
    std::getline(is, GuarMortalityFootnote,         '\n');
    std::getline(is, AccountValueFootnote,          '\n');
    std::getline(is, AttainedAgeFootnote,           '\n');
    std::getline(is, CashSurrValueFootnote,         '\n');
    std::getline(is, DeathBenefitFootnote,          '\n');
    std::getline(is, InitialPremiumFootnote,        '\n');
    std::getline(is, NetPremiumFootnote,            '\n');
    std::getline(is, OutlayFootnote,                '\n');
    std::getline(is, PolicyYearFootnote,            '\n');
    std::getline(is, ADDFootnote,                   '\n');
    std::getline(is, ChildFootnote,                 '\n');
    std::getline(is, SpouseFootnote,                '\n');
    std::getline(is, TermFootnote,                  '\n');
    std::getline(is, WaiverFootnote,                '\n');
    std::getline(is, MinimumPremiumFootnote,        '\n');
    std::getline(is, PremAllocationFootnote,        '\n');
    std::getline(is, ProductDescription,            '\n');
    std::getline(is, StableValueFootnote,           '\n');
    std::getline(is, NoVanishPremiumFootnote,       '\n');
    std::getline(is, RejectPremiumFootnote,         '\n');
    std::getline(is, ExpRatingFootnote,             '\n');
    std::getline(is, MortalityBlendFootnote,        '\n');
    std::getline(is, HypotheticalRatesFootnote,     '\n');
    std::getline(is, SalesLoadRefundFootnote,       '\n');
    std::getline(is, NoLapseFootnote,               '\n');
    std::getline(is, MarketValueAdjFootnote,        '\n');
    std::getline(is, ExchangeChargeFootnote0,       '\n');
    std::getline(is, CurrentValuesFootnote,         '\n');
    std::getline(is, DBOption1Footnote,             '\n');
    std::getline(is, DBOption2Footnote,             '\n');
    std::getline(is, ExpRatRiskChargeFootnote,      '\n');
    std::getline(is, ExchangeChargeFootnote1,       '\n');
    std::getline(is, FlexiblePremiumFootnote,       '\n');
    std::getline(is, GuaranteedValuesFootnote,      '\n');
    std::getline(is, CreditingRateFootnote,         '\n');
    std::getline(is, MecFootnote,                   '\n');
    std::getline(is, MidpointValuesFootnote,        '\n');
    std::getline(is, SinglePremiumFootnote,         '\n');
    std::getline(is, MonthlyChargesFootnote,        '\n');
    std::getline(is, UltCreditingRateFootnote,      '\n');
    std::getline(is, MaxNaarFootnote,               '\n');
    std::getline(is, PremTaxSurrChgFootnote,        '\n');
    std::getline(is, PolicyFeeFootnote,             '\n');
    std::getline(is, AssetChargeFootnote,           '\n');
    std::getline(is, InvestmentIncomeFootnote,      '\n');
    std::getline(is, IrrDbFootnote,                 '\n');
    std::getline(is, IrrCsvFootnote,                '\n');
    std::getline(is, MortalityChargesFootnote,      '\n');
    std::getline(is, LoanAndWithdrawalFootnote,     '\n');
    std::getline(is, PresaleTrackingNumber,         '\n');
    std::getline(is, CompositeTrackingNumber,       '\n');
    std::getline(is, InforceTrackingNumber,         '\n');
    std::getline(is, InforceCompositeTrackingNumber,'\n');

    bool okay = is.good();
    if(!okay)
        {
        fatal_error()
            << "Unexpected end of product data file '"
            << a_Filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
    std::string dummy;
    is >> dummy;
    okay = is.eof();
    if(!okay)
        {
        fatal_error()
            << "Data past expected end of product data file '"
            << a_Filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void TProductData::Write(std::string const& a_Filename) const
{
    std::ofstream os(a_Filename.c_str());

    os << DatabaseFilename              << '\n';
    os << FundFilename                  << '\n';
    os << CorridorFilename              << '\n';
    os << CurrCOIFilename               << '\n';
    os << GuarCOIFilename               << '\n';
    os << WPFilename                    << '\n';
    os << ADDFilename                   << '\n';
    os << ChildRiderFilename            << '\n';
    os << CurrSpouseRiderFilename       << '\n';
    os << GuarSpouseRiderFilename       << '\n';
    os << CurrTermFilename              << '\n';
    os << GuarTermFilename              << '\n';
    os << TableYFilename                << '\n';
    os << PremTaxFilename               << '\n';
    os << TAMRA7PayFilename             << '\n';
    os << TgtPremFilename               << '\n';
    os << IRC7702Filename               << '\n';
    os << Gam83Filename                 << '\n';
    os << SubstdTblMultFilename         << '\n';
    os << CurrSpecAmtLoadFilename       << '\n';
    os << GuarSpecAmtLoadFilename       << '\n';
    os << RoundingFilename              << '\n';
    os << TierFilename                  << '\n';
    os << PolicyForm                    << '\n';
    os << PolicyMktgName                << '\n';
    os << PolicyLegalName               << '\n';
    os << InsCoShortName                << '\n';
    os << InsCoName                     << '\n';
    os << InsCoAddr                     << '\n';
    os << InsCoStreet                   << '\n';
    os << InsCoPhone                    << '\n';
    os << InsCoDomicile                 << '\n';
    os << MainUnderwriter               << '\n';
    os << MainUnderwriterAddress        << '\n';
    os << CoUnderwriter                 << '\n';
    os << CoUnderwriterAddress          << '\n';
    os << AvName                        << '\n';
    os << CsvName                       << '\n';
    os << CsvHeaderName                 << '\n';
    os << NoLapseProvisionName          << '\n';
    os << InterestDisclaimer            << '\n';
    os << GuarMortalityFootnote         << '\n';
    os << AccountValueFootnote          << '\n';
    os << AttainedAgeFootnote           << '\n';
    os << CashSurrValueFootnote         << '\n';
    os << DeathBenefitFootnote          << '\n';
    os << InitialPremiumFootnote        << '\n';
    os << NetPremiumFootnote            << '\n';
    os << OutlayFootnote                << '\n';
    os << PolicyYearFootnote            << '\n';
    os << ADDFootnote                   << '\n';
    os << ChildFootnote                 << '\n';
    os << SpouseFootnote                << '\n';
    os << TermFootnote                  << '\n';
    os << WaiverFootnote                << '\n';
    os << MinimumPremiumFootnote        << '\n';
    os << PremAllocationFootnote        << '\n';
    os << ProductDescription            << '\n';
    os << StableValueFootnote           << '\n';
    os << NoVanishPremiumFootnote       << '\n';
    os << RejectPremiumFootnote         << '\n';
    os << ExpRatingFootnote             << '\n';
    os << MortalityBlendFootnote        << '\n';
    os << HypotheticalRatesFootnote     << '\n';
    os << SalesLoadRefundFootnote       << '\n';
    os << NoLapseFootnote               << '\n';
    os << MarketValueAdjFootnote        << '\n';
    os << ExchangeChargeFootnote0       << '\n';
    os << CurrentValuesFootnote         << '\n';
    os << DBOption1Footnote             << '\n';
    os << DBOption2Footnote             << '\n';
    os << ExpRatRiskChargeFootnote      << '\n';
    os << ExchangeChargeFootnote1       << '\n';
    os << FlexiblePremiumFootnote       << '\n';
    os << GuaranteedValuesFootnote      << '\n';
    os << CreditingRateFootnote         << '\n';
    os << MecFootnote                   << '\n';
    os << MidpointValuesFootnote        << '\n';
    os << SinglePremiumFootnote         << '\n';
    os << MonthlyChargesFootnote        << '\n';
    os << UltCreditingRateFootnote      << '\n';
    os << MaxNaarFootnote               << '\n';
    os << PremTaxSurrChgFootnote        << '\n';
    os << PolicyFeeFootnote             << '\n';
    os << AssetChargeFootnote           << '\n';
    os << InvestmentIncomeFootnote      << '\n';
    os << IrrDbFootnote                 << '\n';
    os << IrrCsvFootnote                << '\n';
    os << MortalityChargesFootnote      << '\n';
    os << LoanAndWithdrawalFootnote     << '\n';
    os << PresaleTrackingNumber         << '\n';
    os << CompositeTrackingNumber       << '\n';
    os << InforceTrackingNumber         << '\n';
    os << InforceCompositeTrackingNumber<< '\n';

    if(!os.good())
        {
        fatal_error()
            << "Unable to write product data file '"
            << a_Filename
            << "'."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void TProductData::WritePolFiles()
{
    TProductData foo;

    foo.CorridorFilename               = "sample";
    foo.CurrCOIFilename                = "qx_cso";
    foo.GuarCOIFilename                = "qx_cso";
    foo.WPFilename                     = "sample";
    foo.ADDFilename                    = "qx_ins";
    foo.ChildRiderFilename             = "qx_ins";
    foo.CurrSpouseRiderFilename        = "qx_ins";
    foo.GuarSpouseRiderFilename        = "qx_ins";
    foo.CurrTermFilename               = "qx_cso";
    foo.GuarTermFilename               = "qx_cso";
    foo.TableYFilename                 = "qx_ins";
    foo.PremTaxFilename                = "sample";
    foo.TAMRA7PayFilename              = "sample";
    foo.TgtPremFilename                = "sample";
    foo.IRC7702Filename                = "qx_cso";
    foo.Gam83Filename                  = "qx_ann";
    foo.SubstdTblMultFilename          = "sample";
    foo.CurrSpecAmtLoadFilename        = "sample";
    foo.GuarSpecAmtLoadFilename        = "sample";
    foo.PolicyForm                     = "UL32768-NY";
    foo.PolicyMktgName                 = "UL Supreme";
    foo.PolicyLegalName = "Flexible Premium Adjustable Life Insurance Policy";
    foo.InsCoShortName                 = "Superior Life";
    foo.InsCoName                      = "Superior Life Insurance Company";
    foo.InsCoAddr                      = "Superior, WI 12345";
    foo.InsCoStreet                    = "246 Main Street";
    foo.InsCoPhone                     = "(800) 555-1212";
    foo.InsCoDomicile                  = "WI";
    foo.MainUnderwriter                = "Superior Securities";
    foo.MainUnderwriterAddress         = "246-M Main Street, Superior, WI 12345";
    foo.CoUnderwriter                  = "Superior Investors";
    foo.CoUnderwriterAddress           = "246-C Main Street, Superior, WI 12345";
    foo.AvName                         = "Account";
    foo.CsvName                        = "Cash Surrender";
    foo.CsvHeaderName                  = "Cash Surr";
    foo.NoLapseProvisionName           = "No-lapse Provision";
    foo.InterestDisclaimer             = "";
    foo.GuarMortalityFootnote          = "";
    foo.AccountValueFootnote           = "";
    foo.AttainedAgeFootnote            = "";
    foo.CashSurrValueFootnote          = "";
    foo.DeathBenefitFootnote           = "";
    foo.InitialPremiumFootnote         = "";
    foo.NetPremiumFootnote             = "";
    foo.OutlayFootnote                 = "";
    foo.PolicyYearFootnote             = "";
    foo.ADDFootnote                    = "";
    foo.ChildFootnote                  = "";
    foo.SpouseFootnote                 = "";
    foo.TermFootnote                   = "";
    foo.WaiverFootnote                 = "";
    foo.MinimumPremiumFootnote         = "";
    foo.PremAllocationFootnote         = "";
    foo.ProductDescription             = "";
    foo.StableValueFootnote            = "";
    foo.NoVanishPremiumFootnote        = "";
    foo.RejectPremiumFootnote          = "";
    foo.ExpRatingFootnote              = "";
    foo.MortalityBlendFootnote         = "";
    foo.HypotheticalRatesFootnote      = "";
    foo.SalesLoadRefundFootnote        = "";
    foo.NoLapseFootnote                = "";
    foo.MarketValueAdjFootnote         = "";
    foo.ExchangeChargeFootnote0        = "";
    foo.CurrentValuesFootnote          = "";
    foo.DBOption1Footnote              = "";
    foo.DBOption2Footnote              = "";
    foo.ExpRatRiskChargeFootnote       = "";
    foo.ExchangeChargeFootnote1        = "";
    foo.FlexiblePremiumFootnote        = "";
    foo.GuaranteedValuesFootnote       = "";
    foo.CreditingRateFootnote          = "";
    foo.MecFootnote                    = "";
    foo.MidpointValuesFootnote         = "";
    foo.SinglePremiumFootnote          = "";
    foo.MonthlyChargesFootnote         = "";
    foo.UltCreditingRateFootnote       = "";
    foo.MaxNaarFootnote                = "";
    foo.PremTaxSurrChgFootnote         = "";
    foo.PolicyFeeFootnote              = "";
    foo.AssetChargeFootnote            = "";
    foo.InvestmentIncomeFootnote       = "";
    foo.IrrDbFootnote                  = "";
    foo.IrrCsvFootnote                 = "";
    foo.MortalityChargesFootnote       = "";
    foo.LoanAndWithdrawalFootnote      = "";
    foo.PresaleTrackingNumber          = "";
    foo.CompositeTrackingNumber        = "";
    foo.InforceTrackingNumber          = "";
    foo.InforceCompositeTrackingNumber = "";

    foo.DatabaseFilename = "sample.db4";
    foo.FundFilename     = "sample.fnd";
    foo.RoundingFilename = "sample.rnd";
    foo.TierFilename     = "sample.tir";

    foo.Write(AddDataDir("sample.pol"));
}

