// Ledger formatter.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: ledger_formatter.cpp,v 1.7 2007-01-24 00:53:20 chicares Exp $

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
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>

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
LedgerFormatterFactory& LedgerFormatterFactory::Instance()
{
    static LedgerFormatterFactory factory;
    return factory;
}

//=============================================================================
LedgerFormatterFactory::LedgerFormatterFactory()
{
}

//=============================================================================
LedgerFormatter LedgerFormatterFactory::CreateFormatter
    (Ledger const& ledger_values
    )
{
    return LedgerFormatter(ledger_values);
}

//=============================================================================
xslt_lmi::Stylesheet const& LedgerFormatterFactory::GetStylesheet
    (std::string const& filename
    )
{
    XmlStylesheets::const_iterator it = stylesheets_.find(filename);
    if(it != stylesheets_.end())
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

//=============================================================================
LedgerFormatter::LedgerFormatter()
    :ledger_values_(NULL)
{
}

//=============================================================================
LedgerFormatter::LedgerFormatter(Ledger const& ledger_values)
    :ledger_values_(&ledger_values)
{
}

//=============================================================================
LedgerFormatter::LedgerFormatter(LedgerFormatter const& rhs)
    :ledger_values_(rhs.ledger_values_)
{
}

//=============================================================================
LedgerFormatter& LedgerFormatter::operator=(LedgerFormatter const& rhs)
{
    if(this != &rhs && ledger_values_ != rhs.ledger_values_)
        {
        ledger_values_ = rhs.ledger_values_;
        }
    return *this;
}

//=============================================================================
xslt_lmi::Stylesheet const& LedgerFormatter::GetStylesheet
    (std::string const& filename
    ) const
{
    return LedgerFormatterFactory::Instance().GetStylesheet(filename);
}

//=============================================================================
xml_lmi::xml_document const& LedgerFormatter::GetXmlDoc
    (enum_xml_version xml_version
    ) const
{
    if(!ledger_values_)
        {
        throw std::runtime_error("Can't generate xml for a NULL ledger.");
        }

    boost::shared_ptr<xml_lmi::xml_document> document
        (new xml_lmi::xml_document(ledger_values_->xml_root_name())
        );
    xml::element& root = document->root_node();

    ledger_values_->write_excerpt(root, xml_version);

    cached_xml_docs_[xml_version] = document;

    return *document;
}

//=============================================================================
void LedgerFormatter::FormatAsHtml(std::ostream& os) const
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_html_filename()
            );

        stylesheet.transform
            (GetXmlDoc(e_xml_calculation_summary).document()
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
void LedgerFormatter::FormatAsLightTSV(std::ostream& os) const
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance()
                .xslt_light_tab_delimited_filename()
            );

        std::map<std::string,std::string> params;
        stylesheet.transform
            (GetXmlDoc(e_xml_calculation_summary).document()
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
void LedgerFormatter::FormatAsTabDelimited(std::ostream& os) const
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_tab_delimited_filename()
            );

        stylesheet.transform
            (GetXmlDoc(e_xml_detailed).document()
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
void LedgerFormatter::FormatAsXslFo(std::ostream& os) const
{
    try
        {
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (ledger_values_->GetLedgerType().str() + ".xsl"
            );

        stylesheet.transform
            (GetXmlDoc(e_xml_full).document()
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
    LedgerFormatter formatter
        (LedgerFormatterFactory::Instance().CreateFormatter(ledger_values)
        );

    std::ofstream ofs(file_name.c_str(), ios_out_trunc_binary());
    formatter.FormatAsHtml(ofs);
}

