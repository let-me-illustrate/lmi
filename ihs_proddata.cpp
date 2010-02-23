// Product data.
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
#include "xml_serialize.hpp"

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
    path = fs::change_extension(path, ".xpol");
    Read(AddDataDir(path.string()));
}

//============================================================================
#ifndef LMI_NO_LEGACY_FORMATS
void TProductData::ReadLegacy(std::string const& a_Filename)
{
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
    std::getline(is, InforceNonGuaranteedFootnote0, '\n');
    std::getline(is, InforceNonGuaranteedFootnote1, '\n');
    std::getline(is, InforceNonGuaranteedFootnote2, '\n');
    std::getline(is, InforceNonGuaranteedFootnote3, '\n');
    std::getline(is, NonGuaranteedFootnote,         '\n');
    std::getline(is, MonthlyChargesPaymentFootnote, '\n');

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
#endif // !LMI_NO_LEGACY_FORMATS

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

#ifndef LMI_NO_LEGACY_FORMATS
    // We temporarily support reading both XML and the old file formats.
    if(".pol" == fs::extension(a_Filename))
        {
        ReadLegacy(a_Filename);
        return;
        }
#endif // !LMI_NO_LEGACY_FORMATS

    xml_lmi::dom_parser doc(a_Filename);
    xml::element const& root = doc.root_node("product");

    #define GET_PROP(name) xml_serialize::get_property(root, #name, name)

    GET_PROP(DatabaseFilename);
    GET_PROP(FundFilename);
    GET_PROP(CorridorFilename);
    GET_PROP(CurrCOIFilename);
    GET_PROP(GuarCOIFilename);
    GET_PROP(WPFilename);
    GET_PROP(ADDFilename);
    GET_PROP(ChildRiderFilename);
    GET_PROP(CurrSpouseRiderFilename);
    GET_PROP(GuarSpouseRiderFilename);
    GET_PROP(CurrTermFilename);
    GET_PROP(GuarTermFilename);
    GET_PROP(TableYFilename);
    GET_PROP(PremTaxFilename);
    GET_PROP(TAMRA7PayFilename);
    GET_PROP(TgtPremFilename);
    GET_PROP(IRC7702Filename);
    GET_PROP(Gam83Filename);
    GET_PROP(SubstdTblMultFilename);
    GET_PROP(CurrSpecAmtLoadFilename);
    GET_PROP(GuarSpecAmtLoadFilename);
    GET_PROP(RoundingFilename);
    GET_PROP(TierFilename);
    GET_PROP(PolicyForm);
    GET_PROP(PolicyMktgName);
    GET_PROP(PolicyLegalName);
    GET_PROP(InsCoShortName);
    GET_PROP(InsCoName);
    GET_PROP(InsCoAddr);
    GET_PROP(InsCoStreet);
    GET_PROP(InsCoPhone);
    GET_PROP(InsCoDomicile);
    GET_PROP(MainUnderwriter);
    GET_PROP(MainUnderwriterAddress);
    GET_PROP(CoUnderwriter);
    GET_PROP(CoUnderwriterAddress);
    GET_PROP(AvName);
    GET_PROP(CsvName);
    GET_PROP(CsvHeaderName);
    GET_PROP(NoLapseProvisionName);
    GET_PROP(InterestDisclaimer);
    GET_PROP(GuarMortalityFootnote);
    GET_PROP(AccountValueFootnote);
    GET_PROP(AttainedAgeFootnote);
    GET_PROP(CashSurrValueFootnote);
    GET_PROP(DeathBenefitFootnote);
    GET_PROP(InitialPremiumFootnote);
    GET_PROP(NetPremiumFootnote);
    GET_PROP(OutlayFootnote);
    GET_PROP(PolicyYearFootnote);
    GET_PROP(ADDFootnote);
    GET_PROP(ChildFootnote);
    GET_PROP(SpouseFootnote);
    GET_PROP(TermFootnote);
    GET_PROP(WaiverFootnote);
    GET_PROP(MinimumPremiumFootnote);
    GET_PROP(PremAllocationFootnote);
    GET_PROP(ProductDescription);
    GET_PROP(StableValueFootnote);
    GET_PROP(NoVanishPremiumFootnote);
    GET_PROP(RejectPremiumFootnote);
    GET_PROP(ExpRatingFootnote);
    GET_PROP(MortalityBlendFootnote);
    GET_PROP(HypotheticalRatesFootnote);
    GET_PROP(SalesLoadRefundFootnote);
    GET_PROP(NoLapseFootnote);
    GET_PROP(MarketValueAdjFootnote);
    GET_PROP(ExchangeChargeFootnote0);
    GET_PROP(CurrentValuesFootnote);
    GET_PROP(DBOption1Footnote);
    GET_PROP(DBOption2Footnote);
    GET_PROP(ExpRatRiskChargeFootnote);
    GET_PROP(ExchangeChargeFootnote1);
    GET_PROP(FlexiblePremiumFootnote);
    GET_PROP(GuaranteedValuesFootnote);
    GET_PROP(CreditingRateFootnote);
    GET_PROP(MecFootnote);
    GET_PROP(MidpointValuesFootnote);
    GET_PROP(SinglePremiumFootnote);
    GET_PROP(MonthlyChargesFootnote);
    GET_PROP(UltCreditingRateFootnote);
    GET_PROP(MaxNaarFootnote);
    GET_PROP(PremTaxSurrChgFootnote);
    GET_PROP(PolicyFeeFootnote);
    GET_PROP(AssetChargeFootnote);
    GET_PROP(InvestmentIncomeFootnote);
    GET_PROP(IrrDbFootnote);
    GET_PROP(IrrCsvFootnote);
    GET_PROP(MortalityChargesFootnote);
    GET_PROP(LoanAndWithdrawalFootnote);
    GET_PROP(PresaleTrackingNumber);
    GET_PROP(CompositeTrackingNumber);
    GET_PROP(InforceTrackingNumber);
    GET_PROP(InforceCompositeTrackingNumber);
    GET_PROP(InforceNonGuaranteedFootnote0);
    GET_PROP(InforceNonGuaranteedFootnote1);
    GET_PROP(InforceNonGuaranteedFootnote2);
    GET_PROP(InforceNonGuaranteedFootnote3);
    GET_PROP(NonGuaranteedFootnote);
    GET_PROP(MonthlyChargesPaymentFootnote);

    #undef GET_PROP
}

//============================================================================
void TProductData::Write(std::string const& a_Filename) const
{
    xml::document doc("product");
    xml::node& root = doc.get_root_node();

    #define ADD_PROP(name) xml_serialize::add_property(root, #name, name)

    ADD_PROP(DatabaseFilename);
    ADD_PROP(FundFilename);
    ADD_PROP(CorridorFilename);
    ADD_PROP(CurrCOIFilename);
    ADD_PROP(GuarCOIFilename);
    ADD_PROP(WPFilename);
    ADD_PROP(ADDFilename);
    ADD_PROP(ChildRiderFilename);
    ADD_PROP(CurrSpouseRiderFilename);
    ADD_PROP(GuarSpouseRiderFilename);
    ADD_PROP(CurrTermFilename);
    ADD_PROP(GuarTermFilename);
    ADD_PROP(TableYFilename);
    ADD_PROP(PremTaxFilename);
    ADD_PROP(TAMRA7PayFilename);
    ADD_PROP(TgtPremFilename);
    ADD_PROP(IRC7702Filename);
    ADD_PROP(Gam83Filename);
    ADD_PROP(SubstdTblMultFilename);
    ADD_PROP(CurrSpecAmtLoadFilename);
    ADD_PROP(GuarSpecAmtLoadFilename);
    ADD_PROP(RoundingFilename);
    ADD_PROP(TierFilename);
    ADD_PROP(PolicyForm);
    ADD_PROP(PolicyMktgName);
    ADD_PROP(PolicyLegalName);
    ADD_PROP(InsCoShortName);
    ADD_PROP(InsCoName);
    ADD_PROP(InsCoAddr);
    ADD_PROP(InsCoStreet);
    ADD_PROP(InsCoPhone);
    ADD_PROP(InsCoDomicile);
    ADD_PROP(MainUnderwriter);
    ADD_PROP(MainUnderwriterAddress);
    ADD_PROP(CoUnderwriter);
    ADD_PROP(CoUnderwriterAddress);
    ADD_PROP(AvName);
    ADD_PROP(CsvName);
    ADD_PROP(CsvHeaderName);
    ADD_PROP(NoLapseProvisionName);
    ADD_PROP(InterestDisclaimer);
    ADD_PROP(GuarMortalityFootnote);
    ADD_PROP(AccountValueFootnote);
    ADD_PROP(AttainedAgeFootnote);
    ADD_PROP(CashSurrValueFootnote);
    ADD_PROP(DeathBenefitFootnote);
    ADD_PROP(InitialPremiumFootnote);
    ADD_PROP(NetPremiumFootnote);
    ADD_PROP(OutlayFootnote);
    ADD_PROP(PolicyYearFootnote);
    ADD_PROP(ADDFootnote);
    ADD_PROP(ChildFootnote);
    ADD_PROP(SpouseFootnote);
    ADD_PROP(TermFootnote);
    ADD_PROP(WaiverFootnote);
    ADD_PROP(MinimumPremiumFootnote);
    ADD_PROP(PremAllocationFootnote);
    ADD_PROP(ProductDescription);
    ADD_PROP(StableValueFootnote);
    ADD_PROP(NoVanishPremiumFootnote);
    ADD_PROP(RejectPremiumFootnote);
    ADD_PROP(ExpRatingFootnote);
    ADD_PROP(MortalityBlendFootnote);
    ADD_PROP(HypotheticalRatesFootnote);
    ADD_PROP(SalesLoadRefundFootnote);
    ADD_PROP(NoLapseFootnote);
    ADD_PROP(MarketValueAdjFootnote);
    ADD_PROP(ExchangeChargeFootnote0);
    ADD_PROP(CurrentValuesFootnote);
    ADD_PROP(DBOption1Footnote);
    ADD_PROP(DBOption2Footnote);
    ADD_PROP(ExpRatRiskChargeFootnote);
    ADD_PROP(ExchangeChargeFootnote1);
    ADD_PROP(FlexiblePremiumFootnote);
    ADD_PROP(GuaranteedValuesFootnote);
    ADD_PROP(CreditingRateFootnote);
    ADD_PROP(MecFootnote);
    ADD_PROP(MidpointValuesFootnote);
    ADD_PROP(SinglePremiumFootnote);
    ADD_PROP(MonthlyChargesFootnote);
    ADD_PROP(UltCreditingRateFootnote);
    ADD_PROP(MaxNaarFootnote);
    ADD_PROP(PremTaxSurrChgFootnote);
    ADD_PROP(PolicyFeeFootnote);
    ADD_PROP(AssetChargeFootnote);
    ADD_PROP(InvestmentIncomeFootnote);
    ADD_PROP(IrrDbFootnote);
    ADD_PROP(IrrCsvFootnote);
    ADD_PROP(MortalityChargesFootnote);
    ADD_PROP(LoanAndWithdrawalFootnote);
    ADD_PROP(PresaleTrackingNumber);
    ADD_PROP(CompositeTrackingNumber);
    ADD_PROP(InforceTrackingNumber);
    ADD_PROP(InforceCompositeTrackingNumber);
    ADD_PROP(InforceNonGuaranteedFootnote0);
    ADD_PROP(InforceNonGuaranteedFootnote1);
    ADD_PROP(InforceNonGuaranteedFootnote2);
    ADD_PROP(InforceNonGuaranteedFootnote3);
    ADD_PROP(NonGuaranteedFootnote);
    ADD_PROP(MonthlyChargesPaymentFootnote);

    #undef ADD_PROP

    if(!doc.save_to_file(a_Filename.c_str()))
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
    foo.InforceNonGuaranteedFootnote0  = "";
    foo.InforceNonGuaranteedFootnote1  = "";
    foo.InforceNonGuaranteedFootnote2  = "";
    foo.InforceNonGuaranteedFootnote3  = "";
    foo.NonGuaranteedFootnote          = "";
    foo.MonthlyChargesPaymentFootnote  = "";

    foo.DatabaseFilename = "sample.xdb4";
    foo.FundFilename     = "sample.xfnd";
    foo.RoundingFilename = "sample.xrnd";
    foo.TierFilename     = "sample.xtir";

    foo.Write(AddDataDir("sample.xpol"));
}

