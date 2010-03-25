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
    path = fs::change_extension(path, ".policy");
    Read(AddDataDir(path.string()));
}

namespace
{
std::string xml_root_name()
{
    return "policy";
}
} // Unnamed namespace.

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

    xml_lmi::dom_parser parser(a_Filename);
    xml::element const& root = parser.root_node(xml_root_name());

#   define GET_ELEMENT(name) xml_serialize::get_element(root, #name, name)

    GET_ELEMENT(DatabaseFilename              );
    GET_ELEMENT(FundFilename                  );
    GET_ELEMENT(CorridorFilename              );
    GET_ELEMENT(CurrCOIFilename               );
    GET_ELEMENT(GuarCOIFilename               );
    GET_ELEMENT(WPFilename                    );
    GET_ELEMENT(ADDFilename                   );
    GET_ELEMENT(ChildRiderFilename            );
    GET_ELEMENT(CurrSpouseRiderFilename       );
    GET_ELEMENT(GuarSpouseRiderFilename       );
    GET_ELEMENT(CurrTermFilename              );
    GET_ELEMENT(GuarTermFilename              );
    GET_ELEMENT(TableYFilename                );
    GET_ELEMENT(PremTaxFilename               );
    GET_ELEMENT(TAMRA7PayFilename             );
    GET_ELEMENT(TgtPremFilename               );
    GET_ELEMENT(IRC7702Filename               );
    GET_ELEMENT(Gam83Filename                 );
    GET_ELEMENT(SubstdTblMultFilename         );
    GET_ELEMENT(CurrSpecAmtLoadFilename       );
    GET_ELEMENT(GuarSpecAmtLoadFilename       );
    GET_ELEMENT(RoundingFilename              );
    GET_ELEMENT(TierFilename                  );
    GET_ELEMENT(PolicyForm                    );
    GET_ELEMENT(PolicyMktgName                );
    GET_ELEMENT(PolicyLegalName               );
    GET_ELEMENT(InsCoShortName                );
    GET_ELEMENT(InsCoName                     );
    GET_ELEMENT(InsCoAddr                     );
    GET_ELEMENT(InsCoStreet                   );
    GET_ELEMENT(InsCoPhone                    );
    GET_ELEMENT(InsCoDomicile                 );
    GET_ELEMENT(MainUnderwriter               );
    GET_ELEMENT(MainUnderwriterAddress        );
    GET_ELEMENT(CoUnderwriter                 );
    GET_ELEMENT(CoUnderwriterAddress          );
    GET_ELEMENT(AvName                        );
    GET_ELEMENT(CsvName                       );
    GET_ELEMENT(CsvHeaderName                 );
    GET_ELEMENT(NoLapseProvisionName          );
    GET_ELEMENT(InterestDisclaimer            );
    GET_ELEMENT(GuarMortalityFootnote         );
    GET_ELEMENT(AccountValueFootnote          );
    GET_ELEMENT(AttainedAgeFootnote           );
    GET_ELEMENT(CashSurrValueFootnote         );
    GET_ELEMENT(DeathBenefitFootnote          );
    GET_ELEMENT(InitialPremiumFootnote        );
    GET_ELEMENT(NetPremiumFootnote            );
    GET_ELEMENT(OutlayFootnote                );
    GET_ELEMENT(PolicyYearFootnote            );
    GET_ELEMENT(ADDFootnote                   );
    GET_ELEMENT(ChildFootnote                 );
    GET_ELEMENT(SpouseFootnote                );
    GET_ELEMENT(TermFootnote                  );
    GET_ELEMENT(WaiverFootnote                );
    GET_ELEMENT(MinimumPremiumFootnote        );
    GET_ELEMENT(PremAllocationFootnote        );
    GET_ELEMENT(ProductDescription            );
    GET_ELEMENT(StableValueFootnote           );
    GET_ELEMENT(NoVanishPremiumFootnote       );
    GET_ELEMENT(RejectPremiumFootnote         );
    GET_ELEMENT(ExpRatingFootnote             );
    GET_ELEMENT(MortalityBlendFootnote        );
    GET_ELEMENT(HypotheticalRatesFootnote     );
    GET_ELEMENT(SalesLoadRefundFootnote       );
    GET_ELEMENT(NoLapseFootnote               );
    GET_ELEMENT(MarketValueAdjFootnote        );
    GET_ELEMENT(ExchangeChargeFootnote0       );
    GET_ELEMENT(CurrentValuesFootnote         );
    GET_ELEMENT(DBOption1Footnote             );
    GET_ELEMENT(DBOption2Footnote             );
    GET_ELEMENT(ExpRatRiskChargeFootnote      );
    GET_ELEMENT(ExchangeChargeFootnote1       );
    GET_ELEMENT(FlexiblePremiumFootnote       );
    GET_ELEMENT(GuaranteedValuesFootnote      );
    GET_ELEMENT(CreditingRateFootnote         );
    GET_ELEMENT(MecFootnote                   );
    GET_ELEMENT(MidpointValuesFootnote        );
    GET_ELEMENT(SinglePremiumFootnote         );
    GET_ELEMENT(MonthlyChargesFootnote        );
    GET_ELEMENT(UltCreditingRateFootnote      );
    GET_ELEMENT(MaxNaarFootnote               );
    GET_ELEMENT(PremTaxSurrChgFootnote        );
    GET_ELEMENT(PolicyFeeFootnote             );
    GET_ELEMENT(AssetChargeFootnote           );
    GET_ELEMENT(InvestmentIncomeFootnote      );
    GET_ELEMENT(IrrDbFootnote                 );
    GET_ELEMENT(IrrCsvFootnote                );
    GET_ELEMENT(MortalityChargesFootnote      );
    GET_ELEMENT(LoanAndWithdrawalFootnote     );
    GET_ELEMENT(PresaleTrackingNumber         );
    GET_ELEMENT(CompositeTrackingNumber       );
    GET_ELEMENT(InforceTrackingNumber         );
    GET_ELEMENT(InforceCompositeTrackingNumber);
    GET_ELEMENT(InforceNonGuaranteedFootnote0 );
    GET_ELEMENT(InforceNonGuaranteedFootnote1 );
    GET_ELEMENT(InforceNonGuaranteedFootnote2 );
    GET_ELEMENT(InforceNonGuaranteedFootnote3 );
    GET_ELEMENT(NonGuaranteedFootnote         );
    GET_ELEMENT(MonthlyChargesPaymentFootnote );

#   undef GET_ELEMENT
}

//============================================================================
void TProductData::Write(std::string const& a_Filename) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    xml_lmi::set_attr(root, "version", "0");

#   define SET_ELEMENT(name) xml_serialize::set_element(root, #name, name)

    SET_ELEMENT(DatabaseFilename              );
    SET_ELEMENT(FundFilename                  );
    SET_ELEMENT(CorridorFilename              );
    SET_ELEMENT(CurrCOIFilename               );
    SET_ELEMENT(GuarCOIFilename               );
    SET_ELEMENT(WPFilename                    );
    SET_ELEMENT(ADDFilename                   );
    SET_ELEMENT(ChildRiderFilename            );
    SET_ELEMENT(CurrSpouseRiderFilename       );
    SET_ELEMENT(GuarSpouseRiderFilename       );
    SET_ELEMENT(CurrTermFilename              );
    SET_ELEMENT(GuarTermFilename              );
    SET_ELEMENT(TableYFilename                );
    SET_ELEMENT(PremTaxFilename               );
    SET_ELEMENT(TAMRA7PayFilename             );
    SET_ELEMENT(TgtPremFilename               );
    SET_ELEMENT(IRC7702Filename               );
    SET_ELEMENT(Gam83Filename                 );
    SET_ELEMENT(SubstdTblMultFilename         );
    SET_ELEMENT(CurrSpecAmtLoadFilename       );
    SET_ELEMENT(GuarSpecAmtLoadFilename       );
    SET_ELEMENT(RoundingFilename              );
    SET_ELEMENT(TierFilename                  );
    SET_ELEMENT(PolicyForm                    );
    SET_ELEMENT(PolicyMktgName                );
    SET_ELEMENT(PolicyLegalName               );
    SET_ELEMENT(InsCoShortName                );
    SET_ELEMENT(InsCoName                     );
    SET_ELEMENT(InsCoAddr                     );
    SET_ELEMENT(InsCoStreet                   );
    SET_ELEMENT(InsCoPhone                    );
    SET_ELEMENT(InsCoDomicile                 );
    SET_ELEMENT(MainUnderwriter               );
    SET_ELEMENT(MainUnderwriterAddress        );
    SET_ELEMENT(CoUnderwriter                 );
    SET_ELEMENT(CoUnderwriterAddress          );
    SET_ELEMENT(AvName                        );
    SET_ELEMENT(CsvName                       );
    SET_ELEMENT(CsvHeaderName                 );
    SET_ELEMENT(NoLapseProvisionName          );
    SET_ELEMENT(InterestDisclaimer            );
    SET_ELEMENT(GuarMortalityFootnote         );
    SET_ELEMENT(AccountValueFootnote          );
    SET_ELEMENT(AttainedAgeFootnote           );
    SET_ELEMENT(CashSurrValueFootnote         );
    SET_ELEMENT(DeathBenefitFootnote          );
    SET_ELEMENT(InitialPremiumFootnote        );
    SET_ELEMENT(NetPremiumFootnote            );
    SET_ELEMENT(OutlayFootnote                );
    SET_ELEMENT(PolicyYearFootnote            );
    SET_ELEMENT(ADDFootnote                   );
    SET_ELEMENT(ChildFootnote                 );
    SET_ELEMENT(SpouseFootnote                );
    SET_ELEMENT(TermFootnote                  );
    SET_ELEMENT(WaiverFootnote                );
    SET_ELEMENT(MinimumPremiumFootnote        );
    SET_ELEMENT(PremAllocationFootnote        );
    SET_ELEMENT(ProductDescription            );
    SET_ELEMENT(StableValueFootnote           );
    SET_ELEMENT(NoVanishPremiumFootnote       );
    SET_ELEMENT(RejectPremiumFootnote         );
    SET_ELEMENT(ExpRatingFootnote             );
    SET_ELEMENT(MortalityBlendFootnote        );
    SET_ELEMENT(HypotheticalRatesFootnote     );
    SET_ELEMENT(SalesLoadRefundFootnote       );
    SET_ELEMENT(NoLapseFootnote               );
    SET_ELEMENT(MarketValueAdjFootnote        );
    SET_ELEMENT(ExchangeChargeFootnote0       );
    SET_ELEMENT(CurrentValuesFootnote         );
    SET_ELEMENT(DBOption1Footnote             );
    SET_ELEMENT(DBOption2Footnote             );
    SET_ELEMENT(ExpRatRiskChargeFootnote      );
    SET_ELEMENT(ExchangeChargeFootnote1       );
    SET_ELEMENT(FlexiblePremiumFootnote       );
    SET_ELEMENT(GuaranteedValuesFootnote      );
    SET_ELEMENT(CreditingRateFootnote         );
    SET_ELEMENT(MecFootnote                   );
    SET_ELEMENT(MidpointValuesFootnote        );
    SET_ELEMENT(SinglePremiumFootnote         );
    SET_ELEMENT(MonthlyChargesFootnote        );
    SET_ELEMENT(UltCreditingRateFootnote      );
    SET_ELEMENT(MaxNaarFootnote               );
    SET_ELEMENT(PremTaxSurrChgFootnote        );
    SET_ELEMENT(PolicyFeeFootnote             );
    SET_ELEMENT(AssetChargeFootnote           );
    SET_ELEMENT(InvestmentIncomeFootnote      );
    SET_ELEMENT(IrrDbFootnote                 );
    SET_ELEMENT(IrrCsvFootnote                );
    SET_ELEMENT(MortalityChargesFootnote      );
    SET_ELEMENT(LoanAndWithdrawalFootnote     );
    SET_ELEMENT(PresaleTrackingNumber         );
    SET_ELEMENT(CompositeTrackingNumber       );
    SET_ELEMENT(InforceTrackingNumber         );
    SET_ELEMENT(InforceCompositeTrackingNumber);
    SET_ELEMENT(InforceNonGuaranteedFootnote0 );
    SET_ELEMENT(InforceNonGuaranteedFootnote1 );
    SET_ELEMENT(InforceNonGuaranteedFootnote2 );
    SET_ELEMENT(InforceNonGuaranteedFootnote3 );
    SET_ELEMENT(NonGuaranteedFootnote         );
    SET_ELEMENT(MonthlyChargesPaymentFootnote );

#   undef SET_ELEMENT

    // Instead of this:
//    document.save(a_Filename);
    // for the nonce, explicitly change the extension, in order to
    // force external product-file code to use the new extension.
    fs::path path(a_Filename, fs::native);
    path = fs::change_extension(path, ".policy");
    document.save(path.string());
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

    foo.DatabaseFilename = "sample.db4";
    foo.FundFilename     = "sample.fnd";
    foo.RoundingFilename = "sample.rnd";
    foo.TierFilename     = "sample.tir";

    foo.Write(AddDataDir("sample.policy"));
}

