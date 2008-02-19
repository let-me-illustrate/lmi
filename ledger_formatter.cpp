// Ledger formatter.
//
// Copyright (C) 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger_formatter.cpp,v 1.13 2008-02-19 00:46:28 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_formatter.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "ledger.hpp"
#include "ledger_xsl.hpp"
#include "miscellany.hpp" // ios_out_trunc_binary()
#include "xml_lmi.hpp"

#include <exception>
#include <fstream>
#include <ostream>

namespace
{
// This abomination rips-off the const-protection coat from xml_document
// and accesses its internal document member in a non-const way.
xml_lmi::Document& get_non_const_document(xml_lmi::xml_document const& xml_doc)
{
    return const_cast<xml_lmi::Document&>(xml_doc.document());
}

} // unnamed namespace

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
    catch(std::exception const& e)
        {
        fatal_error()
            << "Invalid file name '"
            << filename
            << "': "
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
        fatal_error() << "Can't generate xml for a NULL ledger." << LMI_FLUSH;
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
        fatal_error()
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
        fatal_error()
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
        fatal_error()
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
    // This function is controversial because it uses different data formats
    // that are not supposed to work together.
    //
    // XML input data in the new format is converted back into the old format
    // via "xml2to1.xsl" template to match the needs of the templates
    // that generate xsl-fo output.
    try
        {
        xml_lmi::xml_document const& input_v2 = GetXmlDoc(e_xml_full);
        xml_lmi::xml_document        input_v1("dummy");

        // First transformation reduces the new XML input format to the old one.
        xslt_lmi::Stylesheet const& converter = GetStylesheet("xml2to1.xsl");

        converter.transform
            (input_v2.document()
            ,get_non_const_document(input_v1)
            );

        // Second transformation produces xsl-fo output from the old format XML.
        //
        // INELEGANT !! It's goofy to get a complete filepath, then
        // discard parts of it, when they'll presumably need to be
        // added back later. Apparently there's some sort of caching
        // going on; whether it has any benefit that could justify
        // the attendant complexity is not evident.

        LMI_ASSERT(ledger_values_);
        xslt_lmi::Stylesheet const& stylesheet = GetStylesheet
            (xsl_filepath(*ledger_values_).leaf()
            );

        stylesheet.transform
            (input_v1.document()
            ,os
            ,xslt_lmi::Stylesheet::e_output_xml
            );
        }
    catch(std::exception const& e)
        {
        fatal_error()
            << "Error formatting ledger values as xsl-fo: '"
            << e.what()
            << "'."
            << LMI_FLUSH
            ;
        }
}

//=============================================================================
void LedgerFormatter::FormatAsXml
    (std::ostream& os
    ,enum_xml_version xml_version
    ) const
{
    try
        {
        os << GetXmlDoc(xml_version);
        }
    catch(std::exception const& e)
        {
        fatal_error()
            << "Error generating xml for ledger: '"
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

