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

// $Id: ledger_text_formats.hpp,v 1.5.2.1 2006-10-20 17:46:02 etarassov Exp $

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
  public:
    static LedgerFormatterFactory & Instance();

    LedgerFormatter   CreateFormatter(Ledger const & ledger_values);
    xsltStylesheetPtr GetStylesheet(std::string const & filename);

  private:
    typedef std::map<std::string, xsltStylesheetPtr> Stylesheets;
    Stylesheets stylesheets;

    LedgerFormatterFactory();
};

/// LedgerFormatter class
///
/// Implements ledger_values formatting into various media types
/// such as html, csv, xsl-fo.
/// It has value semantics.
/// Instances of the class could only be obtained through LedgerFormatterFactory.

class LMI_SO LedgerFormatter
{
  public:
    // default empty constructor does nothing
    LedgerFormatter();

    LedgerFormatter(LedgerFormatter const & rhs);
    LedgerFormatter & operator = (LedgerFormatter const & rhs);

    void FormatAsHtml         (std::ostream & str) const;
    void FormatAsTabDelimited (std::ostream & str) const;
    void FormatAsXslFo        (std::ostream & str) const;

    Ledger const* GetLedger() const { return ledger_values_; }

    // calculate corresponding XML data if needed
    void PrepareXmlDataForHtml()         { GetXmlDocLight(); }
    void PrepareXmlDataForTabDelimited() { GetXmlDocLight(); }
    void PrepareXmlDataForXslFo()        { GetXmlDocHeavy(); }

  private:
    Ledger const* ledger_values_;

    typedef boost::shared_ptr<xmlDoc> XmlDocSharedPtr;
    typedef boost::shared_ptr<xml_lmi::Document> XmlDocumentSharedPtr;

    // light and heavy version of xml data, mutable is needed to allow
    // its lazy initialisation
    mutable XmlDocumentSharedPtr xmlpp_document_heavy_;
    mutable XmlDocumentSharedPtr xmlpp_document_light_;

    // clear xml data
    void ResetXmlData();

    // generate the corresponding xml data if it was not already done
    xmlDoc const* GetXmlDocHeavy() const;
    xmlDoc const* GetXmlDocLight() const;

    XmlDocumentSharedPtr DoGenerateXml(bool light_version) const;

    // load the xsl template specified and apply it to the ledger_data
    XmlDocSharedPtr DoFormatAs
        (std::string const & xslt_filename
        ,xmlDoc const*
        ) const;

    // load the xsl template into memory
    xsltStylesheetPtr GetStylesheet(std::string const & filename) const;

    friend class LedgerFormatterFactory;

    // copy ctor, accessible to LedgerFormatterFactory only
    LedgerFormatter(Ledger const & ledger_values);
};

// A shortcut method, that opens a file for writing and uses a fresh instance
// of LedgerFormatter to produce output.
// Note that every call to this method will result in ledger xml data being recalculated.
void LMI_SO PrintFormTabDelimited
    (Ledger const&      ledger_values
    ,std::string const& file_name
    );

#endif // ledger_text_formats_hpp

