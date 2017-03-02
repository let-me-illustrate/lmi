// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "multiple_cell_document.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/document.h>
#include <xmlwrapp/nodes_view.h>
#include <xmlwrapp/schema.h>
#include <xsltwrapp/stylesheet.h>

#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

//============================================================================
multiple_cell_document::multiple_cell_document()
    :case_parms_  (1)
    ,class_parms_ (1)
    ,cell_parms_  (1)
{
    assert_vector_sizes_are_sane();
}

//============================================================================
multiple_cell_document::multiple_cell_document(std::string const& filename)
{
    xml_lmi::dom_parser parser(filename);
    parse(parser);
    assert_vector_sizes_are_sane();
}

//============================================================================
multiple_cell_document::~multiple_cell_document() = default;

/// Verify invariants.
///
/// Throws if any asserted invariant does not hold.

void multiple_cell_document::assert_vector_sizes_are_sane() const
{
    LMI_ASSERT(1 == case_parms_.size());
    LMI_ASSERT(    !class_parms_.empty());
    LMI_ASSERT(    !cell_parms_ .empty());
}

/// Backward-compatibility serial number of this class's xml version.
///
/// What is now called version 0 had no "version" attribute.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20120220T0158Z
/// version 2: 20150316T0409Z

int multiple_cell_document::class_version() const
{
    return 2;
}

//============================================================================
std::string const& multiple_cell_document::xml_root_name() const
{
    static std::string const s("multiple_cell_document");
    return s;
}

namespace
{
/// Throw an exception while pretending to return an lvalue.
///
/// Motivating example:
///   X& x(boolean ? x1 : throw("oops"));
/// It is handy to write a throw-expression as the last operand in a
/// cascaded conditional-expression. However, that doesn't work when
/// an lvalue must be returned: according to C++2003, [15/1] a throw-
/// expression is of type void, so [5.16/2] the conditional-expression
/// above returns an lvalue. The motivating example works correctly
/// when rewritten this way:
///   X& x(boolean ? x1 : invalid<X>("oops"));

template<typename T>
T& hurl(std::string const& s)
{
    throw std::runtime_error(s.c_str());
}
} // Unnamed namespace.

/// Read xml into vectors of class Input.

void multiple_cell_document::parse(xml_lmi::dom_parser const& parser)
{
    xml::element const& root(parser.root_node(xml_root_name()));

    int file_version = 0;
    if(!xml_lmi::get_attr(root, "version", file_version))
        {
        parse_v0(parser);
        return;
        }

    // Version 0 should have been handled above.
    LMI_ASSERT(0 < file_version);
    if(class_version() < file_version)
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        }

    if(data_source_is_external(parser.document()))
        {
        validate_with_xsd_schema(parser.document(), xsd_schema_name(file_version));
        }

    case_parms_ .clear();
    class_parms_.clear();
    cell_parms_ .clear();

    Input cell;
    int counter = 0;
    for(auto const& i : root.elements())
        {
        std::string const tag(i.get_name());
        std::vector<Input>& v
            ( ("case_default"     == tag) ? case_parms_
            : ("class_defaults"   == tag) ? class_parms_
            : ("particular_cells" == tag) ? cell_parms_
            : hurl<std::vector<Input>>("Unexpected element '" + tag + "'.")
            );
        xml::const_nodes_view const subelements(i.elements());
        v.reserve(subelements.size());
        for(auto const& j : subelements)
            {
            j >> cell;
            v.push_back(cell);
            status() << "Read " << ++counter << " cells." << std::flush;
            }
        }

    assert_vector_sizes_are_sane();
}

/// Parse obsolete version 0 xml (for backward compatibility).

void multiple_cell_document::parse_v0(xml_lmi::dom_parser const& parser)
{
    xml::element const& root(parser.root_node(xml_root_name()));

    Input temp;

    xml::const_nodes_view const elements(root.elements());
    // With C++14 we would write
    //   auto i = elements.cbegin();
    // instead of the next two lines.
    typedef xml::const_nodes_view::const_iterator cnvi;
    cnvi i = elements.begin();

    // Case default parameters.

    case_parms_.clear();

    LMI_ASSERT(i != elements.end());
    if("cell" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "cell"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    // In some old extracts, case and class defaults were defectively
    // empty except for a version-attribute node, which should always
    // name exact version 5.
    if(i->size() <= 1)
        {
        int version = 0;
        xml_lmi::get_attr(*i, "version", version);
        if(5 != version)
            {
            fatal_error()
                << "Case-default 'cell' element is empty, but is version "
                << version
                << " where version 5 was expected."
                << LMI_FLUSH
                ;
            }
        }
    else
        {
        *i >> temp;
        }
    case_parms_.push_back(temp);

    // Number of classes.
    ++i;
    LMI_ASSERT(i != elements.end());
    if("NumberOfClasses" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "NumberOfClasses"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    std::string const n_classes = xml_lmi::get_content(*i);
    LMI_ASSERT(!n_classes.empty());
    unsigned int number_of_classes = value_cast<unsigned int>(n_classes);

    // Parameters for each class.
    class_parms_.clear();
    class_parms_.reserve(number_of_classes);

    ++i;
    for(; i != elements.end(); ++i)
        {
        // See comment on defective extracts above.
        if(i->size() <= 1)
            {
            if(1 != number_of_classes)
                {
                fatal_error()
                    << "Class-default 'cell' element is empty, and there are "
                    << number_of_classes
                    << " classes where 1 was expected."
                    << LMI_FLUSH
                    ;
                }
            int version = 0;
            xml_lmi::get_attr(*i, "version", version);
            if(5 != version)
                {
                fatal_error()
                    << "Class-default 'cell' element is empty, but is version "
                    << version
                    << " where version 5 was expected."
                    << LMI_FLUSH
                    ;
                }
            }
        else
            {
            *i >> temp;
            }
        class_parms_.push_back(temp);
        if(class_parms_.size() == number_of_classes)
            {
            break;
            }
        }
    if(class_parms_.size() != number_of_classes)
        {
        fatal_error()
            << "Number of classes read is "
            << class_parms_.size()
            << " but should have been "
            << number_of_classes
            << "."
            << LMI_FLUSH
            ;
        }

    // Number of cells.
    LMI_ASSERT(i != elements.end());
    ++i;
    LMI_ASSERT(i != elements.end());
    if("NumberOfCells" != xml_lmi::get_name(*i))
        {
        fatal_error()
            << "XML node name is '"
            << xml_lmi::get_name(*i)
            << "' but '"
            << "NumberOfCells"
            << "' was expected."
            << LMI_FLUSH
            ;
        }
    std::string const n_cells = xml_lmi::get_content(*i);
    LMI_ASSERT(!n_cells.empty());
    unsigned int number_of_cells = value_cast<unsigned int>(n_cells);

    // Parameters for each cell.
    cell_parms_.clear();
    cell_parms_.reserve(number_of_cells);

    ++i;
    for(; i != elements.end(); ++i)
        {
        *i >> temp;
        cell_parms_.push_back(temp);
        status()
            << "Read "
            << cell_parms_.size()
            << " of "
            << number_of_cells
            << " lives."
            << std::flush
            ;
        if(cell_parms_.size() == number_of_cells)
            {
            break;
            }
        }
    if(cell_parms_.size() != number_of_cells)
        {
        fatal_error()
            << "Number of individuals read is "
            << cell_parms_.size()
            << " but should have been "
            << number_of_cells
            << "."
            << LMI_FLUSH
            ;
        }

    LMI_ASSERT(i != elements.end());
    ++i;
    if(i != elements.end())
        {
        fatal_error()
            << "Read all data expected in XML document, "
            << "but more data remains."
            << LMI_FLUSH
            ;
        }

    assert_vector_sizes_are_sane();
}

/// Ascertain whether input file comes from a system other than lmi.
///
/// External files are validated with an xml schema. This validation,
/// which imposes an overhead of about twenty percent, is skipped for
/// files produced by lmi itself, which are presumptively valid.
///
/// Regrettably, as this is written in 2013-04, external files
/// represent the data source in <cell> element <InforceDataSource>
/// rather than in root attribute "data_source", so for now it is
/// necessary to look for the lower-level element. Both represent the
/// data source the same way: "0" is reserved, "1" means lmi, and
/// each external system is assigned a higher integer.

bool multiple_cell_document::data_source_is_external(xml::document const& d) const
{
    xml::element const& root(d.get_root_node());

    int data_source = 0;
    if(xml_lmi::get_attr(root, "data_source", data_source))
        {
        return 1 < data_source;
        }

    // INPUT !! Remove "InforceDataSource" and the following code when
    // external systems are updated to use the "data_source" attribute.

    // Tag names vary: {"case_default", "class_defaults", "particular_cells"}.
    xml::const_nodes_view const i_nodes(root.elements());
    LMI_ASSERT(3 == i_nodes.size());
    for(auto const& i : i_nodes)
        {
        for(auto const& j : i.elements("cell"))
            {
            for(auto const& k : j.elements("InforceDataSource"))
                {
                std::string s(xml_lmi::get_content(k));
                if("0" != s && "1" != s)
                    {
                    return true;
                    }
                }
            }
        }

    return false;
}

//============================================================================
void multiple_cell_document::validate_with_xsd_schema
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
        }
}

/// Stylesheet to sort <cell> elements.
///
/// This is needed for an external system that cannot economically
/// provide xml with alphabetically-sorted elements.

xslt::stylesheet& multiple_cell_document::cell_sorter() const
{
    static std::string const f("sort_cell_subelements.xsl");
    static xslt::stylesheet z(xml_lmi::dom_parser(AddDataDir(f)).document());
    return z;
}

//============================================================================
std::string multiple_cell_document::xsd_schema_name(int version) const
{
    static std::string const s("multiple_cell_document.xsd");
    if(class_version() == version)
        {
        return s;
        }

    std::ostringstream oss;
    oss
        << "multiple_cell_document"
        << '_' << std::setfill('0') << std::setw(2) << version
        << ".xsd"
        ;
    return oss.str();
}

//============================================================================
void multiple_cell_document::read(std::istream const& is)
{
    xml_lmi::dom_parser parser(is);
    parse(parser);
}

//============================================================================
void multiple_cell_document::write(std::ostream& os) const
{
    assert_vector_sizes_are_sane();

    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());
    xml_lmi::set_attr(root, "data_source", 1); // "1" means lmi.

    xml::element case_default("case_default");
    xml::node::iterator case_i = root.insert(case_default);
    case_parms_[0].write(*case_i);

    xml::element class_defaults("class_defaults");
    xml::node::iterator classes_i = root.insert(class_defaults);
    for(auto const& i : class_parms_)
        {
        i.write(*classes_i);
        }

    xml::element particular_cells("particular_cells");
    xml::node::iterator cells_i = root.insert(particular_cells);
    for(auto const& i : cell_parms_)
        {
        i.write(*cells_i);
        }

    os << document;
}

