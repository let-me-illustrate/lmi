// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "single_cell_document.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "xml_lmi.hpp"

#include <xmlwrapp/document.h>
#include <xmlwrapp/nodes_view.h>
#include <xmlwrapp/schema.h>
#include <xsltwrapp/stylesheet.h>

#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>

/// Copy constructor.

single_cell_document::single_cell_document(Input const& z)
    :input_data_ {z}
{
}

/// This ctor is used to read the default input file.

single_cell_document::single_cell_document(std::string const& filename)
    :input_data_ {}
{
    xml_lmi::dom_parser parser(filename);
    parse(parser);
}

/// Backward-compatibility serial number of this class's xml version.
///
/// What is now called version 0 had no "version" attribute.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20130428T1828Z
/// version 2: 20150316T0409Z
///   [versions 3-8 never existed]
/// version 9: 20180628T1936Z

int single_cell_document::class_version() const
{
    return 9;
}

/// Name of xml root element.

std::string const& single_cell_document::xml_root_name() const
{
    static std::string const s("single_cell_document");
    return s;
}

/// Read xml into an instance of class Input.

void single_cell_document::parse(xml_lmi::dom_parser const& parser)
{
    xml::element const& root(parser.root_node(xml_root_name()));

    int file_version = 0;
    if(!xml_lmi::get_attr(root, "version", file_version))
        {
        // Do nothing. Ancient lmi files have no "version" attribute,
        // and that's okay. Here, 'file_version' is used only for
        // schema validation, which is performed iff a "data_source"
        // attribute exists and has a nondefault value--but, because
        // "data_source" is a newer attribute than "version", there
        // should be no "data_source" without "version". However, some
        // old vendor extracts failed to follow that rule; therefore,
        // this file
        //   single_cell_document_01.xsd
        // has been copied to
        //   single_cell_document_00.xsd
        // which is the correct schema for vendor files that have an
        // "InforceDataSource" element but no "data_source" attribute.
        // No corresponding '.rnc' schema is needed because validation
        // uses only '.xsd' schemata.
        }

    if(class_version() < file_version)
        {
        alarum() << "Incompatible file version." << LMI_FLUSH;
        }

    if(data_source_is_external(parser.document()))
        {
        validate_with_xsd_schema(parser.document(), xsd_schema_name(file_version));
        }

    xml::const_nodes_view const elements(root.elements());
    // An '.ill' document's root contains only one child element.
    LMI_ASSERT(1 == elements.size());
    // "*elements.begin()" because there is no front(). A 2017-01-26
    // discussion off the mailing list explained why:
    // 'The nodes returned by nodes_view::iterator are actually
    //  temporary objects as the only "real" nodes we have are
    //  xmlNodes in the tree maintained by libxml2 itself. So
    //  returning "*begin()" from front() actually results in a
    //  dangling reference'.
    *elements.begin() >> input_data_;
    if(data_source_is_external(parser.document()))
        {
        input_data_.validate_external_data();
        input_data_.Reconcile();
        }
}

/// Ascertain whether input file comes from a system other than lmi.
///
/// External files are validated with an xml schema. This validation,
/// which imposes an overhead of about twenty percent, is skipped for
/// files produced by lmi itself, which are presumptively valid.
///
/// Values for root attribute "data_source":
///   "0" is reserved;
///   "1" means lmi; and
///   each external system is assigned a higher integer.
///
/// Regrettably, some older external files defectively represent the
/// data source only in obsolete <cell> element <InforceDataSource>,
/// rather than in root attribute "data_source", so if that attribute
/// is missing, it is necessary to look for the lower-level element
/// (which uses the same values to represent the data source).

bool single_cell_document::data_source_is_external(xml::document const& d) const
{
    xml::element const& root(d.get_root_node());

    int data_source = 0;
    if(xml_lmi::get_attr(root, "data_source", data_source))
        {
        LMI_ASSERT(0 != data_source);
        return 1 != data_source;
        }
    else
        {
        int file_version = 0;
        xml_lmi::get_attr(root, "version", file_version);
        LMI_ASSERT(file_version <= 2);
        }

    xml::const_nodes_view const i_nodes(root.elements("cell"));
    LMI_ASSERT(1 == i_nodes.size());
    for(auto const& i : i_nodes)
        {
        for(auto const& j : i.elements("InforceDataSource"))
            {
            std::string s(xml_lmi::get_content(j));
            if("0" != s && "1" != s)
                {
                return true;
                }
            }
        }

    return false;
}

/// Coarsely validate file format with XSD schema.

void single_cell_document::validate_with_xsd_schema
    (xml::document const& xml
    ,std::string const&   xsd
    ) const
{
    xml::schema const schema(xml_lmi::dom_parser(AddDataDir(xsd)).document());
    xml::error_messages errors;
    if(!schema.validate(cell_sorter().apply(xml), errors))
        {
        warning()
            << "Validation with schema '"
            << xsd
            << "' failed.\n\n"
            << errors.print()
            << std::flush
            ;
        alarum() << "Invalid input file." << LMI_FLUSH;
        }
}

/// Stylesheet to sort <cell> elements.
///
/// This is needed for an external system that cannot economically
/// provide xml with alphabetically-sorted elements.

xslt::stylesheet& single_cell_document::cell_sorter() const
{
    static std::string const f("sort_cell_subelements.xsl");
    static xslt::stylesheet z(xml_lmi::dom_parser(AddDataDir(f)).document());
    return z;
}

/// Filename of XSD schema for coarsely validating file format.

std::string single_cell_document::xsd_schema_name(int version) const
{
    static std::string const s("single_cell_document.xsd");
    if(class_version() == version)
        {
        return s;
        }
    std::ostringstream oss;

    oss
        << "single_cell_document"
        << '_' << std::setfill('0') << std::setw(2) << version
        << ".xsd"
        ;
    return oss.str();
}

/// Read from xml file.

void single_cell_document::read(std::istream const& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser);
}

/// Write to xml file.

void single_cell_document::write(std::ostream& os) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());
    xml_lmi::set_attr(root, "data_source", 1); // "1" means lmi.
    root << input_data_;
    os << document;
}
