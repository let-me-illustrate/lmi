// Product-specific string data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "product_data.hpp"

#include "data_directory.hpp"           // AddDataDir()

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
/// This is intentionally a minimal clone of write_policy_files(), and
/// is intended only as an examplar for customization to implement
/// proprietary real-world products.

void product_data::write_proprietary_policy_files()
{
    product_data z;

    // Generic data: a partial clone of the 'sample' product.

    z.DatabaseFilename           = glossed_string("sample.database");
    z.FundFilename               = glossed_string("sample.funds");
    z.LingoFilename              = glossed_string("sample.lingo");
    z.RoundingFilename           = glossed_string("sample.rounding");
    z.TierFilename               = glossed_string("sample.strata");

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
    z.PolicyForm                 = glossed_string("UL32768-NY"); // LINGO !! expunge
    z.PolicyFormAlternative      = glossed_string("UL32768-X"); // LINGO !! expunge
    z.PolicyMktgName             = glossed_string("UL Supreme");
    z.PolicyLegalName            = glossed_string("Flexible Premium Adjustable Life Insurance Policy");
    z.InsCoShortName             = glossed_string("Superior Life");
    z.InsCoName                  = glossed_string("Superior Life Insurance Company");
    z.InsCoAddr                  = glossed_string("Superior, WI 12345");
    z.InsCoStreet                = glossed_string("246 Main Street");
    z.InsCoPhone                 = glossed_string("(800) 555-1212");
    z.InsCoDomicile              = glossed_string("WI");
    z.MainUnderwriter            = glossed_string("Superior Securities");
    z.MainUnderwriterAddress     = glossed_string("246-M Main Street, Superior, WI 12345");
    z.CoUnderwriter              = glossed_string("Superior Investors");
    z.CoUnderwriterAddress       = glossed_string("246-C Main Street, Superior, WI 12345");
    z.AvName                     = glossed_string("Account");
    z.CsvName                    = glossed_string("Cash Surrender");
    z.CsvHeaderName              = glossed_string("Cash Surr");
    z.NoLapseProvisionName       = glossed_string("No-lapse Provision");

// Not necessary for 'sample' product only, because it's built in.
//    z.save(AddDataDir("sample.policy"));

    // Copy the template above for other policy forms, e.g.:
//  z.DatabaseFilename        = glossed_string("another.database");
//  ...
}
