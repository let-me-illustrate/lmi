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

// $Id: ledger_text_formats.cpp,v 1.22.2.7 2006-10-30 18:36:26 etarassov Exp $

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
xml_lmi::Stylesheet const& LedgerFormatterFactory::GetStylesheet
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
        boost::filesystem::path xslt_directory
            (configurable_settings::instance().xslt_directory()
            );
        full_name = (xslt_directory / filename).string();
        }
    catch(boost::filesystem::filesystem_error const& e)
        {
        fatal_error()
            << "Invalid file name '"
            << filename
            << "' or directory '"
            << configurable_settings::instance().xslt_directory()
            << "'."
            << LMI_FLUSH
            ;
        }

    XmlStylesheetPtr stylesheet_ptr(new xml_lmi::Stylesheet(full_name));

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
    if (this != &rhs && ledger_values_ != rhs.ledger_values_)
        {
        ledger_values_ = rhs.ledger_values_;
        xml_document_light_ = rhs.xml_document_light_;
        xml_document_heavy_ = rhs.xml_document_heavy_;
        }
    return *this;
}

//=============================================================================
xml_lmi::Stylesheet const& LedgerFormatter::GetStylesheet
    (std::string const& filename
    ) const
{
    return LedgerFormatterFactory::Instance().GetStylesheet(filename);
}

//=============================================================================
void LedgerFormatter::ResetXmlData()
{
    xml_document_light_.reset();
    xml_document_heavy_.reset();
}

//=============================================================================
xml_lmi::Document const& LedgerFormatter::GetXmlDocLight() const
{
    xml_document_light_ = DoGenerateXml(Ledger::e_xml_light);

    return *xml_document_light_;
}

//=============================================================================
xml_lmi::Document const& LedgerFormatter::GetXmlDocHeavy() const
{
    xml_document_heavy_ = DoGenerateXml(Ledger::e_xml_full);

    return *xml_document_heavy_;
}

//=============================================================================
LedgerFormatter::XmlDocumentPtr LedgerFormatter::DoGenerateXml
    (Ledger::enum_xml_version xml_version
    ) const
{
    if(!ledger_values_)
        {
        throw std::runtime_error("Can't generate xml for a NULL ledger.");
        }

    XmlDocumentPtr doc(new xml_lmi::Document);
    xml_lmi::Element& root
        = *(doc->create_root_node(ledger_values_->xml_root_name()));

    ledger_values_->do_write(root, xml_version);

    return doc;
}

//=============================================================================
void LedgerFormatter::FormatAsHtml(std::ostream& os) const
{
    try
        {
        xml_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_html_filename()
            );

        stylesheet.transform
            (GetXmlDocLight()
            ,os
            ,xml_lmi::Stylesheet::e_output_html
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
void LedgerFormatter::FormatAsTabDelimited(std::ostream& os) const
{
    try
        {
        xml_lmi::Stylesheet const& stylesheet = GetStylesheet
            (configurable_settings::instance().xslt_tab_delimited_filename()
            );

        stylesheet.transform
            (GetXmlDocLight()
            ,os
            ,xml_lmi::Stylesheet::e_output_text
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
        xml_lmi::Stylesheet const& stylesheet = GetStylesheet
            (ledger_values_->GetLedgerType().str() + ".xsl"
            );

        stylesheet.transform
            (GetXmlDocHeavy()
            ,os
            ,xml_lmi::Stylesheet::e_output_xml
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
