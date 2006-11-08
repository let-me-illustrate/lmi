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

// $Id: ledger_text_formats.hpp,v 1.5.2.11 2006-11-08 00:42:55 etarassov Exp $

#ifndef ledger_text_formats_hpp
#define ledger_text_formats_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "xml_lmi.hpp"

#include <boost/shared_ptr.hpp>
#include <libxslt/documents.h>

#include <map>
#include <ostream>
#include <string>

class Ledger;
class LedgerFormatter;

/// LedgerFormatterFactory class
///
/// This singleton class manages XSL templates used by instances
/// of LedgerFormatter class.

class LMI_SO LedgerFormatterFactory
{
    typedef boost::shared_ptr<xml_lmi::Stylesheet>  XmlStylesheetPtr;
    typedef std::map<std::string, XmlStylesheetPtr> XmlStylesheets;

  public:
    static LedgerFormatterFactory& Instance();

    LedgerFormatter CreateFormatter(Ledger const& ledger_values);

    xml_lmi::Stylesheet const& GetStylesheet(std::string const& filename);

  private:
    XmlStylesheets stylesheets_;

    LedgerFormatterFactory();
};

/// LedgerFormatter class
///
/// Implements ledger_values formatting into various media types
/// such as html, csv, xsl-fo.
/// It has value semantics. Instances of the class could only be obtained
/// through LedgerFormatterFactory.

class LMI_SO LedgerFormatter
{
  public:
    // default empty constructor does nothing
    LedgerFormatter();

    LedgerFormatter(LedgerFormatter const&);
    LedgerFormatter& operator=(LedgerFormatter const&);

    void FormatAsHtml          (std::ostream&) const;
    void FormatAsLightTSV      (std::ostream&) const;
    void FormatAsTabDelimited  (std::ostream&) const;
    void FormatAsXslFo         (std::ostream&) const;

    Ledger const* GetLedger() const { return ledger_values_; }

  private:
    Ledger const* ledger_values_;

    typedef boost::shared_ptr<xml_lmi::Document> XmlDocumentPtr;

    mutable std::map<enum_xml_version,XmlDocumentPtr> cached_xml_docs_;

    // generate the corresponding xml data if it was not already done
    xml_lmi::Document const& GetXmlDoc(enum_xml_version) const;

    xml_lmi::Stylesheet const& GetStylesheet
        (std::string const& filename
        ) const;

    friend class LedgerFormatterFactory;

    // copy ctor, accessible to LedgerFormatterFactory only
    LedgerFormatter(Ledger const& ledger_values);
};

// A shortcut method, that opens a file for writing and uses a fresh instance
// of LedgerFormatter to produce output.
// Note that every call to this method will result in ledger xml data being recalculated.
void LMI_SO PrintFormTabDelimited
    (Ledger const&      ledger_values
    ,std::string const& file_name
    );

#endif // ledger_text_formats_hpp

