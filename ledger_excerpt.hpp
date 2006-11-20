// Ledger excerption for various reports.
//
// Copyright (C) 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ledger_excerpt.hpp,v 1.1.2.1 2006-11-20 12:18:08 etarassov Exp $

#ifndef ledger_excerpt_hpp
#define ledger_excerpt_hpp

#include "config.hpp"

/// We need to generate different versions of xml data:
/// - e_xml_calculation_summary
///     a light version, when speed is crucial (calculation summary)
/// - e_xml_spreadsheet
///     a detailed version, the data subset for TSV output (to a spreadsheet)
/// - e_xml_full
///     a full version, when we need all the information

enum enum_xml_version
    {e_xml_calculation_summary
    ,e_xml_spreadsheet
    ,e_xml_full
    };

#endif // ledger_excerpt_hpp

