// Product-specific string data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "data_directory.hpp" // AddDataDir()

/// Create string-data files for a particular product.
///
/// It is possible to maintain string data by writing code to create
/// these xml files, but editing the xml directly is probably easier.
///
/// Using lmi code in a program that embodies proprietary data creates
/// a derived work covered by the GNU General Public License (GPL). If
/// you distribute such a program, the GPL requires you to provide its
/// source code, which would disclose proprietary data. If you're not
/// willing to disclose it, then you are not permitted to distribute
/// the derived work at all.
///
/// However, read the second paragraph of GPL section 0 carefully. It
/// permits you to run your modified version of the program--and to
/// distribute its output, which is not a derived work because it's
/// merely your data. Casting it in a format suitable for use with lmi
/// does not subject it to the GPL. You can therefore distribute the
/// files created by your modified version of this program, but not
/// that program itself. Those files are all you need: distributing
/// the program itself isn't necessary anyway.
///
/// This function is intentionally more or less a duplicate of
/// product_data::WritePolFiles(), but this one is meant to be
/// customized.

void product_data::WriteProprietaryPolFiles()
{
    product_data z;

    // Generic data for the 'sample' product.

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

    // Copy the template above for other policy forms, e.g.:
//  z.DatabaseFilename               = "another.db4";
//  ...
}

