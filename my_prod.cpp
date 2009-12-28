// Product-specific fund data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: my_prod.cpp,v 1.11 2009-02-18 22:12:12 wboutin Exp $

// This file is a template for embedding product-specific data. Doing
// that creates a derived work covered by the GPL. But you may prefer
// not to publish your data, for instance because it is proprietary.
// In that case, the GPL does not permit you to distribute the derived
// work at all. But read the second paragraph of section 0 of the GPL
// carefully: it permits you to run your modified version of the
// program--and to distribute its output, which is not a derived work
// because it's merely your data, trivially cast in a format suitable
// for use with lmi. You can therefore distribute the files created by
// your modified version of this program, but not that program itself.
// Those files are all you need: distributing the program itself isn't
// necessary anyway.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_proddata.hpp"

#include "data_directory.hpp"

// TODO ?? It would be better to call TProductData::WritePolFiles()
// here than to duplicate (more or less) its contents. However, it
// would be better still to use xml for all product data files.

//============================================================================
void TProductData::WriteProprietaryPolFiles()
{
    TProductData foo;

    // Generic data.
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

    // Sample policy form.
    foo.DatabaseFilename  = "sample.db4";
    foo.FundFilename      = "sample.fnd";
    foo.RoundingFilename  = "sample.rnd";
    foo.TierFilename      = "sample.tir";

    foo.Write(AddDataDir("sample.pol"));

    // Another policy form....
}

