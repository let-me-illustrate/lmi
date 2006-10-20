// Ledger formatting as text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: ledger_text_formats.cpp,v 1.22.2.1 2006-10-20 17:46:02 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_text_formats.hpp"

#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
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
#include <libxml/globals.h>
#include <libxml/HTMLtree.h>
#include <libxml/parser.h>
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>

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
LedgerFormatterFactory & LedgerFormatterFactory::Instance()
{
    static LedgerFormatterFactory factory;
    return factory;
}

//=============================================================================
LedgerFormatterFactory::LedgerFormatterFactory()
{
}

//=============================================================================
LedgerFormatter LedgerFormatterFactory::CreateFormatter(Ledger const & ledger_values)
{
    return LedgerFormatter(ledger_values);
}

//=============================================================================
xsltStylesheetPtr LedgerFormatterFactory::GetStylesheet(std::string const & filename)
{
    Stylesheets::const_iterator it = stylesheets.find(filename);
    if (it != stylesheets.end())
        {
        return it->second;
        }

    if (stylesheets.empty())
        {
        // initialize libxml for libxslt
        xmlSubstituteEntitiesDefault(1);
        xmlLoadExtDtdDefaultValue = 0;
        }

    std::string full_name;
    try
        {
        boost::filesystem::path xslt_directory
            (configurable_settings::instance().xslt_directory()
            );
        full_name = (xslt_directory / filename).string();
        }
    catch(boost::filesystem::filesystem_error const & e)
        {
        hobsons_choice()
            << "Invalid file name '"
            << filename
            << "' or directory '"
            << configurable_settings::instance().xslt_directory()
            << "'."
            << LMI_FLUSH
            ;
        return NULL;
        }

    xsltStylesheetPtr stylesheet
        (xsltParseStylesheetFile(reinterpret_cast<xmlChar const*>(full_name.c_str()))
        );

    if (!stylesheet)
        {
        hobsons_choice()
            << "Can't load xslt file '"
            << filename
            << "' in directory '"
            << configurable_settings::instance().xslt_directory()
            << "'."
            << LMI_FLUSH
            ;
        return NULL;
        }
    stylesheets[filename] = stylesheet;
    return stylesheet;
}

/// Custom deleter for xmlDocPtr shared_ptr
///
/// Use this class in a shared_ptr to free an xml document pointer using
/// xmlFree method.
/// The pointer to be freed should be allocated using libxml functions.
///
/// See boost::shared_ptr requirements for a deleter class:
/// >> D  must be CopyConstructible.
/// >> The copy constructor and destructor of D must not throw.
/// >> The expression d(p) must be well-formed, must not invoke undefined
/// >> behavior, and must not throw exceptions.

class XmlDocSharedPtrDeleter
{
  public:
    void operator () (xmlDocPtr xml_doc)
    {
        try
            {
            xmlFreeDoc(xml_doc);
            }
        catch(...)
            {
            warning()
                << "A call to xmlFreeDoc failed."
                << LMI_FLUSH
                ;
            }
    }
    // default ctor, dtor and copy-ctor are ok
};

//=============================================================================
LedgerFormatter::LedgerFormatter()
    :ledger_values_(NULL)
{
}

//=============================================================================
LedgerFormatter::LedgerFormatter(Ledger const & ledger_values)
    :ledger_values_(&ledger_values)
{
}

//=============================================================================
LedgerFormatter::LedgerFormatter(LedgerFormatter const & rhs)
    :ledger_values_(rhs.ledger_values_)
{
}

//=============================================================================
LedgerFormatter & LedgerFormatter::operator = (LedgerFormatter const & rhs)
{
    if (this != &rhs && ledger_values_ != rhs.ledger_values_)
        {
        ledger_values_ = rhs.ledger_values_;
        xmlpp_document_light_ = rhs.xmlpp_document_light_;
        xmlpp_document_heavy_ = rhs.xmlpp_document_heavy_;
        }
    return *this;
}

//=============================================================================
xsltStylesheetPtr LedgerFormatter::GetStylesheet(std::string const & filename) const
{
    return LedgerFormatterFactory::Instance().GetStylesheet(filename);
}

//=============================================================================
void LedgerFormatter::ResetXmlData()
{
    xmlpp_document_light_.reset();
    xmlpp_document_heavy_.reset();
}

//=============================================================================
xmlDoc const* LedgerFormatter::GetXmlDocLight() const
{
    if (!xmlpp_document_light_)
    {
        xmlpp_document_light_ = DoGenerateXml(true); // light_version
        if (!xmlpp_document_light_)
            return NULL;
    }

    return xmlpp_document_light_->cobj();
}

//=============================================================================
xmlDoc const* LedgerFormatter::GetXmlDocHeavy() const
{
    if (!xmlpp_document_heavy_)
    {
        xmlpp_document_heavy_ = DoGenerateXml(true); // light_version
        if (!xmlpp_document_heavy_)
            return NULL;
    }

    return xmlpp_document_heavy_->cobj();
}

//=============================================================================
LedgerFormatter::XmlDocumentSharedPtr LedgerFormatter::DoGenerateXml
    (bool light_version
    ) const
{
    if (ledger_values_)
        {
        XmlDocumentSharedPtr doc(new xml_lmi::Document);
        xml_lmi::Element & root
            = *(doc->create_root_node(ledger_values_->xml_root_name()));

        ledger_values_->do_write(root, light_version);

        return doc;
        }

    return XmlDocumentSharedPtr();
}

//=============================================================================
void LedgerFormatter::FormatAsHtml(std::ostream & str) const
{
    XmlDocSharedPtr transformed_data = DoFormatAs
        (configurable_settings::instance().xslt_html_filename()
        ,GetXmlDocLight()
        );

    if (!transformed_data.get())
        return;

    char* buffer = NULL;
    int bytes_written;

    htmlDocDumpMemoryFormat
        (transformed_data.get()         // xml document to write
        ,reinterpret_cast<xmlChar**>(&buffer) // text buffer pointer, we own it
        ,&bytes_written                 // text buffer size
        ,true                           // indent the output
        );
    if(bytes_written >= 0)
        {
        boost::scoped_array<char> buffer_guard(buffer);

        str.write(buffer, bytes_written);
        }
}

//=============================================================================
void LedgerFormatter::FormatAsTabDelimited(std::ostream & str) const
{
    XmlDocSharedPtr transformed_data = DoFormatAs
        (configurable_settings::instance().xslt_tab_delimited_filename()
        ,GetXmlDocLight()
        );

    if (!transformed_data.get())
        return;

    char* buffer = NULL;
    int bytes_written;

    htmlDocDumpMemoryFormat
        (transformed_data.get()         // xml document to write
        ,reinterpret_cast<xmlChar**>(&buffer) // text buffer pointer, we own it
        ,&bytes_written                 // text buffer size
        ,false                          // do _not_ indent the output
        );
    if(bytes_written >= 0)
        {
        boost::scoped_array<char> buffer_guard(buffer);

        str.write(buffer, bytes_written);
        }
}

//=============================================================================
void LedgerFormatter::FormatAsXslFo(std::ostream & str) const
{
    // xsl template to use depends on the ledger type
    XmlDocSharedPtr transformed_data = DoFormatAs
        (ledger_values_->GetLedgerType().str() + ".xsl"
        ,GetXmlDocHeavy()
        );

    if (!transformed_data.get())
        return;

    char* buffer = NULL;
    int bytes_written;

    xmlDocDumpFormatMemory
        (transformed_data.get()         // xml document to write
        ,reinterpret_cast<xmlChar**>(&buffer) // text buffer pointer, we own it
        ,&bytes_written                 // text buffer size
        ,false                          // do _not_ indent the output
        );
    if(bytes_written >= 0)
        {
        boost::scoped_array<char> buffer_guard(buffer);

        str.write(buffer, bytes_written);
        }
}

//=============================================================================
LedgerFormatter::XmlDocSharedPtr LedgerFormatter::DoFormatAs
    (std::string const & xslt_filename
    ,xmlDoc const* xml_doc
    ) const
{
    if(xml_doc)
        {
        try
            {
            xsltStylesheetPtr stylesheet = GetStylesheet(xslt_filename);

            xmlDocPtr transformed_data
                (xsltApplyStylesheet
                    (stylesheet
                    ,const_cast<xmlDoc*>(xml_doc)
                    ,NULL
                    )
                );

            if (!transformed_data)
                {
                hobsons_choice()
                    << "Error in xsltApplyStylesheet('"
                    << xslt_filename
                    << "', <ledger_xml_doc>)."
                    << LMI_FLUSH
                    ;
                }

            return XmlDocSharedPtr
                (transformed_data
                ,XmlDocSharedPtrDeleter()
                );
            }
        catch(std::exception const & ex)
            {
            hobsons_choice()
                << "Error while formatting xml data '"
                << ex.what()
                << "'."
                << LMI_FLUSH;
            }
        }

    return XmlDocSharedPtr();
}

//=============================================================================
void LMI_SO PrintFormTabDelimited
    (Ledger const&      ledger_values
    ,std::string const& file_name
    )
{
    LedgerFormatter formatter
        (LedgerFormatterFactory::Instance().CreateFormatter(ledger_values)
        );

    std::ofstream ofs
        (file_name.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    formatter.FormatAsHtml(ofs);
}
