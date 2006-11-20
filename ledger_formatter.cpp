// Ledger formatter.
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

// $Id: ledger_formatter.cpp,v 1.1.2.2 2006-11-20 13:17:34 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_formatter.hpp"

#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "financial.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "security.hpp"
#include "value_cast.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_array.hpp>

#include <libxml++/libxml++.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iterator>
#include <locale>
#include <sstream>
#include <vector>

//=============================================================================
LedgerFormatter& LedgerFormatter::instance()
{
    static LedgerFormatter formatter;
    return formatter;
}

//=============================================================================
LedgerFormatter::LedgerFormatter()
{
}

//=============================================================================
xslt_lmi::Stylesheet const& LedgerFormatter::GetStylesheet
    (std::string const& filename
    )
{
    XmlStylesheets::const_iterator it = stylesheets_.find(filename);
    if (it != stylesheets_.end())
        {
        XmlStylesheetPtr ptr = it->second;
        return *ptr.get();
        }

    std::string full_name;
    try
        {
        full_name = AddDataDir(filename);
        }
    catch(fs::filesystem_error const& e)
        {
        fatal_error()
            << "Invalid file name '"
            << filename
            << "'. "
            << e.what()
            << LMI_FLUSH
            ;
        }

    XmlStylesheetPtr stylesheet_ptr(new xslt_lmi::Stylesheet(full_name));

    stylesheets_[filename] = stylesheet_ptr;

    return *stylesheet_ptr.get();
}

/// Returns a non-null pointer to a newly created xml_lmi::Document containing
/// the xml data requested

LedgerFormatter::XmlDocumentPtr LedgerFormatter::GenerateXmlData
    (Ledger const& ledger_values
    ,enum_xml_version xml_version
    )
{
    XmlDocumentPtr document_ptr(new xml_lmi::Document);
    xml_lmi::Element& root
        = *(document_ptr->create_root_node(ledger_values.xml_root_name()));

    ledger_values.write_excerpt(root, xml_version);

    return document_ptr;
}

//=============================================================================
void LedgerFormatter::FormatAsHtml
    (Ledger const& ledger_values
    ,std::ostream& os
    )
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_html_filename()
            );

        stylesheet.transform
            (*GenerateXmlData(ledger_values, e_xml_calculation_summary)
            ,os
            ,xslt_lmi::Stylesheet::e_output_html
            );
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error formatting ledger values as html: '"
            << e.what()
            << "'."
            << LMI_FLUSH
            ;
        }
}

//=============================================================================
void LedgerFormatter::FormatAsLightTSV
    (Ledger const& ledger_values
    ,std::ostream& os
    )
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance()
                .xslt_light_tab_delimited_filename()
            );

        stylesheet.transform
            (*GenerateXmlData(ledger_values, e_xml_calculation_summary)
            ,os
            ,xslt_lmi::Stylesheet::e_output_text
            );
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error formatting ledger calculation summary as tsv: '"
            << e.what()
            << "'."
            << LMI_FLUSH
            ;
        }
}

//=============================================================================
void LedgerFormatter::FormatAsTabDelimited
    (Ledger const& ledger_values
    ,std::ostream& os
    )
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_tab_delimited_filename()
            );

        stylesheet.transform
            (*GenerateXmlData(ledger_values, e_xml_spreadsheet)
            ,os
            ,xslt_lmi::Stylesheet::e_output_text
            );
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error formatting ledger values as tsv: '"
            << e.what()
            << "'."
            << LMI_FLUSH
            ;
        }
}

//=============================================================================
void LedgerFormatter::FormatAsXslFo
    (Ledger const& ledger_values
    ,std::ostream& os
    )
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (ledger_values.GetLedgerType().str() + ".xsl"
            );

        stylesheet.transform
            (*GenerateXmlData(ledger_values, e_xml_full)
            ,os
            ,xslt_lmi::Stylesheet::e_output_xml
            );
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error formatting ledger values as xsl-fo: '"
            << e.what()
            << "'."
            << LMI_FLUSH
            ;
        }
}

//=============================================================================
void LMI_SO PrintFormTabDelimitedXXX
    (Ledger const&      ledger_values
    ,std::string const& file_name
    )
{
    std::ofstream ofs
        (file_name.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );

    LedgerFormatter::instance().FormatAsHtml(ledger_values, ofs);
}

