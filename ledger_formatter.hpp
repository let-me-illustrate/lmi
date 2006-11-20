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

// $Id: ledger_formatter.hpp,v 1.1.2.1 2006-11-20 13:17:34 etarassov Exp $

#ifndef ledger_formatter_hpp
#define ledger_formatter_hpp

#include "config.hpp"

#include "ledger_excerpt.hpp" // enum_xml_version
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"
#include "xslt_lmi.hpp"

#include <boost/shared_ptr.hpp>

#include <libxslt/documents.h>

#include <map>
#include <ostream>
#include <string>

class Ledger;
class LedgerFormatter;

/// LedgerFormatter class
///
/// Implements ledger_values formatting into various media types
/// such as html, csv, xsl-fo.
///
/// Singleton. It caches XSL templates in memory.

class LMI_SO LedgerFormatter
{
  public:
    static LedgerFormatter& instance();

    void FormatAsHtml         (Ledger const&, std::ostream&);
    void FormatAsLightTSV     (Ledger const&, std::ostream&);
    void FormatAsTabDelimited (Ledger const&, std::ostream&);
    void FormatAsXslFo        (Ledger const&, std::ostream&);

  private:
    typedef boost::shared_ptr<xslt_lmi::Stylesheet> XmlStylesheetPtr;
    typedef std::map<std::string, XmlStylesheetPtr> XmlStylesheets;

    XmlStylesheets stylesheets_;

    LedgerFormatter();

    xslt_lmi::Stylesheet const& GetStylesheet(std::string const& filename);

    typedef boost::shared_ptr<xml_lmi::Document> XmlDocumentPtr;

    // generate the corresponding xml data
    XmlDocumentPtr GenerateXmlData(Ledger const&, enum_xml_version);
};

// A shortcut method, that opens a file for writing and uses a fresh instance
// of LedgerFormatter to produce output.
// Note that every call to this method will result in ledger xml data being recalculated.
void LMI_SO PrintFormTabDelimitedXXX
    (Ledger const&      ledger_values
    ,std::string const& file_name
    );

#endif // ledger_formatter_hpp

