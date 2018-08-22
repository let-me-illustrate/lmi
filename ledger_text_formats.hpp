// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#ifndef ledger_text_formats_hpp
#define ledger_text_formats_hpp

#include "config.hpp"

#include "oecumenic_enumerations.hpp"
#include "so_attributes.hpp"

#include <iosfwd>
#include <string>
#include <utility>                      // pair
#include <vector>

class Ledger;

std::string LMI_SO FormatSelectedValuesAsHtml(Ledger const&);
std::string LMI_SO FormatSelectedValuesAsTsv (Ledger const&);

void LMI_SO PrintCellTabDelimited  (Ledger const&, std::string const& file_name);

void LMI_SO PrintRosterHeaders     (               std::string const& file_name);
void LMI_SO PrintRosterTabDelimited(Ledger const&, std::string const& file_name);

void LMI_SO PrintLedgerFlatText    (Ledger const&, std::ostream&);

std::string LMI_SO ledger_format
    (double                            d
    ,std::pair<int,oenum_format_style> f
    );

std::vector<std::string> LMI_SO ledger_format
    (std::vector<double> const&        dv
    ,std::pair<int,oenum_format_style> f
    );

/// Metadata for presentation of ledger entities.
///
/// Implicitly-declared special member functions do the right thing.

struct ledger_metadata
{
    ledger_metadata()
        :decimals_ {0}
        ,style_    {oe_format_normal}
        ,legend_   {""}
        {}

    ledger_metadata
        (int                decimals
        ,oenum_format_style style
        ,std::string        legend
        )
        :decimals_ {decimals}
        ,style_    {style}
        ,legend_   {legend}
        {}

    int                decimals_;
    oenum_format_style style_;
    std::string        legend_;
};

#endif // ledger_text_formats_hpp

