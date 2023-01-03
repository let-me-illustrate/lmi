// Interface to xmlwrapp.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "xml_lmi.hpp"

#include "alert.hpp"
#include "istream_to_string.hpp"
#include "platform_dependent.hpp"       // access()
#include "value_cast.hpp"

#include <xmlwrapp/attributes.h>
#include <xmlwrapp/document.h>
#include <xmlwrapp/init.h>
#include <xmlwrapp/tree_parser.h>

#include <ostream>
#include <sstream>
#include <stdexcept>

namespace xml_lmi
{
/// Parse an xml file.
///
/// Precondition: argument names an accessible xml file.
///
/// Postconditions: member parser_ is valid.
///
/// Throws: std::runtime_error, via alarum(), if a precondition is
/// violated, or if xml-library calls throw an exception derived
/// from std::exception.

xml_lmi::dom_parser::dom_parser(std::string const& filename)
{
    try
        {
        error_context_ = "Unable to parse xml file '" + filename + "': ";
        if(filename.empty())
            {
            throw std::runtime_error("File name is empty.");
            }
        if(0 != access(filename.c_str(), F_OK))
            {
            throw std::runtime_error("File does not exist.");
            }
        parser_.reset(::new DomParser(filename.c_str()));
        }
    catch(std::exception const& e)
        {
        alarum() << error_context_ << e.what() << LMI_FLUSH;
        }
}

/// Parse an xml stream.
///
/// Precondition: arguments describe an xml string.
///
/// Postconditions: member parser_ is valid.
///
/// Throws: std::runtime_error, via alarum(), if a precondition is
/// violated, or if xml-library calls throw an exception derived
/// from std::exception.

xml_lmi::dom_parser::dom_parser(char const* data, std::size_t length)
{
    try
        {
        error_context_ = "Unable to parse xml data: ";
        parser_.reset(::new DomParser(data, length));
        }
    catch(std::exception const& e)
        {
        alarum() << error_context_ << e.what() << LMI_FLUSH;
        }
}

/// Parse an xml stream.
///
/// XMLWRAPP !! xmlwrapp has no such ctor as
///   xml::tree_parser(std::istream&)
/// Therefore, read the std::istream into a std::string with
/// istream_to_string(), and pass that to the xml::tree_parser ctor
/// that takes a char* and a byte count.
///
/// Precondition: argument is an xml stream for which 0 == rdstate().
///
/// Postconditions: member parser_ is valid.
///
/// Throws: std::runtime_error, via alarum(), if a precondition is
/// violated, or if xml-library calls throw an exception derived
/// from std::exception.

xml_lmi::dom_parser::dom_parser(std::istream const& is)
{
    try
        {
        error_context_ = "Unable to parse xml stream: ";
        if(0 != is.rdstate())
            {
            throw std::runtime_error("Stream state is not 'good'.");
            }
        std::string s;
        istream_to_string(is, s);
        parser_.reset(::new DomParser(s.c_str(), 1 + s.size()));
        }
    catch(std::exception const& e)
        {
        alarum() << error_context_ << e.what() << LMI_FLUSH;
        }
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.
///
/// Throws: nothing unless member parser_'s destructor does.

xml_lmi::dom_parser::~dom_parser() = default;

/// Return the parsed document.
///
/// Preconditions: member parser_ has a document.
///
/// Throws: std::runtime_error, via alarum(), if a precondition is
/// violated, or if xml-library calls throw an exception derived
/// from std::exception. Ctor postconditions are assumed to have been
/// satisfied and are not tested.

xml_lmi::Document const& xml_lmi::dom_parser::document() const
{
    try
        {
        return parser_->get_document();
        }
    catch(std::exception const& e)
        {
        alarum() << error_context_ << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Return the parsed document's root node.
///
/// Preconditions: member parser_ has a document that is not null and
/// has a root node; the argument, if not empty, matches the name of
/// that root node.
///
/// Throws: std::runtime_error, via alarum(), if a precondition is
/// violated, or if xml-library calls throw an exception derived
/// from std::exception. Ctor postconditions are assumed to have been
/// satisfied and are not tested.

xml::element const& xml_lmi::dom_parser::root_node
    (std::string const& expected_name
    ) const
{
    try
        {
        xml_lmi::Document const& document = parser_->get_document();
        xml::element const& root = document.get_root_node();
        if(!expected_name.empty() && expected_name != root.get_name())
            {
            std::ostringstream oss;
            oss
                << "Root-node name is '"
                << root.get_name()
                << "', but '"
                << expected_name
                << "' was expected."
                ;
            throw std::runtime_error(oss.str());
            }

        return root;
        }
    catch(std::exception const& e)
        {
        alarum() << error_context_ << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

xml_lmi::xml_document::xml_document(std::string const& root_node_name)
    :document_ {::new xml_lmi::Document(xml::element(root_node_name.c_str()))}
{
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.

xml_lmi::xml_document::~xml_document() = default;

xml::element& xml_lmi::xml_document::root_node()
{
    return document_->get_root_node();
}

void xml_lmi::xml_document::save(std::string const& filename)
{
    bool okay = document_->save_to_file(filename.c_str());
    if(!okay)
        {
        alarum() << "Unable to save file '" << filename << "'." << LMI_FLUSH;
        }
}

std::string xml_lmi::xml_document::str()
{
    std::string s;
    document_->save_to_string(s);
    return s;
}

/// Add an xml comment just before the root (first element node).
///
/// For readability, the comment is padded with single spaces.

void xml_lmi::xml_document::add_comment(std::string const& s)
{
    for(xml::node::iterator i = document_->begin(); i != document_->end(); ++i)
        {
        if(xml::node::type_element == i->get_type())
            {
            std::string t = ' ' + s + ' ';
            document_->insert(i, xml::node(xml::node::comment(t.c_str())));
            return;
            }
        }
    alarum() << "Cannot add comment to rootless document." << LMI_FLUSH;
}

/// Find an element subnode by name, throwing if it is not found.

xml::node::const_iterator retrieve_element
    (xml::element const& parent
    ,std::string  const& name
    )
{
    xml::node::const_iterator i = parent.find(name.c_str());
    if(parent.end() == i)
        {
        alarum() << "Required element '" << name << "' not found." << LMI_FLUSH;
        }
    return i;
}

/// Retrieve an xml element's full text-node contents.
///
/// The contents of all text-node children are concatenated.
///
/// Only direct children are considered: children of child nodes
/// are not.

std::string get_content(xml::element const& element)
{
    try
        {
        std::string s;
        for(auto const& i : element)
            {
            if(i.is_text())
                {
                char const* content = i.get_content();
                if(content)
                    {
                    s += i.get_content();
                    }
                }
            }
        return s;
        }
    catch(std::exception const& e)
        {
        alarum() << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Retrieve an xml element's name.

std::string get_name(xml::element const& element)
{
    try
        {
        char const* name = element.get_name();
        return name ? name : "";
        }
    catch(std::exception const& e)
        {
        alarum() << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Get a named attribute of an xml element.
///
/// If the element has no such attribute, then return false and
/// guarantee not to modify 'value'.

bool get_attr
    (xml::element const& element
    ,std::string const&  name
    ,std::string&        value
    )
{
    try
        {
        xml::attributes const& attrs = element.get_attributes();
        xml::attributes::const_iterator i = attrs.find(name.c_str());
        if(i != attrs.end())
            {
            value = i->get_value();
            return true;
            }
        else
            {
            return false;
            }
        }
    catch(std::exception const& e)
        {
        alarum() << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Get a named attribute of an xml element; convert it to integer.
///
/// If the element has no such attribute, then return false and
/// guarantee not to modify 'value'.

bool get_attr
    (xml::element const& element
    ,std::string const&  name
    ,int&                value
    )
{
    std::string s;
    if(get_attr(element, name, s))
        {
        value = value_cast<int>(s);
        return true;
        }
    else
        {
        return false;
        }
}

/// Set a named attribute of an xml element.

void set_attr
    (xml::element&      element
    ,std::string const& name
    ,std::string const& value
    )
{
    try
        {
        element.get_attributes().insert(name.c_str(), value.c_str());
        }
    catch(std::exception const& e)
        {
        alarum() << e.what() << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

/// Set a named attribute of an xml element.

void set_attr
    (xml::element&      element
    ,std::string const& name
    ,int                value
    )
{
    set_attr(element, name, value_cast<std::string>(value));
}
} // namespace xml_lmi

std::ostream& operator<<(std::ostream& os, xml_lmi::xml_document const& d)
{
    os << d.document();
    return os;
}
