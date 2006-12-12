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

// $Id: ledger_formatter.hpp,v 1.3 2006-12-12 10:46:36 chicares Exp $

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

/// LedgerFormatterFactory class
///
/// This singleton class manages XSL templates used by instances
/// of LedgerFormatter class.
///
/// TODO ?? CALCULATION_SUMMARY Wouldn't it be better to make
///   CreateFormatter()
///   GetStylesheet()
/// const, and
///   stylesheets_
/// mutable? Even if not, shouldn't at least CreateFormatter() be const?

class LMI_SO LedgerFormatterFactory
{
    typedef boost::shared_ptr<xslt_lmi::Stylesheet> XmlStylesheetPtr;
    typedef std::map<std::string, XmlStylesheetPtr> XmlStylesheets;

  public:
    static LedgerFormatterFactory& Instance();

    LedgerFormatter CreateFormatter(Ledger const& ledger_values);
    xslt_lmi::Stylesheet const& GetStylesheet(std::string const& filename);

  private:
    LedgerFormatterFactory();

    XmlStylesheets stylesheets_;
};

/// LedgerFormatter class
///
/// Implements ledger_values formatting into various media types
/// such as html, csv, xsl-fo.
///
/// It has value semantics. Instances of the class could only be obtained
/// through LedgerFormatterFactory.
/// TODO ?? CALCULATION_SUMMARY Is that so? Doesn't the default ctor
/// create an instance?

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

    mutable std::map
        <enum_xml_version
        ,boost::shared_ptr<xml_lmi::xml_document>
        > cached_xml_docs_;

    // generate the corresponding xml data if it was not already done
    xml_lmi::xml_document const& GetXmlDoc(enum_xml_version) const;

    xslt_lmi::Stylesheet const& GetStylesheet
        (std::string const& filename
        ) const;

    friend class LedgerFormatterFactory;

    // copy ctor, accessible to LedgerFormatterFactory only
    LedgerFormatter(Ledger const& ledger_values);
};

// A shortcut method, that opens a file for writing and uses a fresh instance
// of LedgerFormatter to produce output.
// Note that every call to this method will result in ledger xml data being recalculated.
void LMI_SO PrintFormTabDelimitedXXX
    (Ledger const&      ledger_values
    ,std::string const& file_name
    );

#endif // ledger_formatter_hpp

