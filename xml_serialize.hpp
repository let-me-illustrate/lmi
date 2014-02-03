// Serialization to and from xml.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

// $Id$

#ifndef xml_serialize_hpp
#define xml_serialize_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_enum.hpp>

#include <xmlwrapp/nodes_view.h>

#include <string>
#include <vector>

/// Serialization to and from xml.

namespace xml_serialize
{
/// Serialization for built-in and other streamable types.
///
/// This class template is intended to be specialized for other types.
///
/// Member functions to_xml() and from_xml() might return an xml
/// element and a T, respectively, instead of taking them as
/// arguments; but that would entail copy-construction overhead.
///
/// Free function templates might have been used instead, but a class
/// template is more amenable to specialization.
///
/// value_cast<>() is already specialized for string-to-string
/// conversions, both for efficiency and to ensure correct handling
/// of strings with embedded whitespace.

template<typename T>
struct xml_io
{
    BOOST_STATIC_ASSERT(!boost::is_enum<T>::value); // Prefer mc_enum.

    static void to_xml(xml::element& e, T const& t)
    {
        // XMLWRAPP !! Add a clear() function.
        e.erase(e.begin(), e.end());
        // XMLWRAPP !! Someday, this might be rewritten thus:
        //   e.set_content(value_cast<std::string>(t).c_str());
        // but for now that doesn't work with embedded ampersands.
        e.push_back(xml::node(xml::node::text(value_cast<std::string>(t).c_str())));
    }

    static void from_xml(xml::element const& e, T& t)
    {
        t = value_cast<T>(xml_lmi::get_content(e));
    }
};

/// Serialization for sequences [23.1.1].
///
/// Derive publicly from this to use its implementation when
/// specializing class template xml_io for a particular sequence.
///
/// from_xml() reads only <item> elements, ignoring other elements
/// (and non-element nodes) that might have been added manually,
/// e.g., as documentation.

template<typename T>
struct xml_sequence_io
{
    typedef typename T::value_type item_t;

    static void to_xml(xml::element& e, T const& t)
    {
        // XMLWRAPP !! Add a clear() function.
        e.erase(e.begin(), e.end());
        typedef typename T::const_iterator tci;
        for(tci i = t.begin(); i != t.end(); ++i)
            {
            // This is not equivalent to calling set_element():
            // multiple <item> elements are expressly permitted.
            xml::element z("item");
            xml_io<item_t>::to_xml(z, *i);
            e.push_back(z);
            }
    }

    static void from_xml(xml::element const& e, T& t)
    {
        t.clear();
        xml::const_nodes_view const items(e.elements("item"));
        typedef xml::const_nodes_view::const_iterator cnvi;
        for(cnvi i = items.begin(); i != items.end(); ++i)
            {
            item_t z;
            xml_io<item_t>::from_xml(*i, z);
            t.push_back(z);
            }
    }
};

template<typename T>
struct xml_io<std::vector<T> >
  :public xml_sequence_io<std::vector<T> >
{};

/// Serialize a datum into a subelement of the given xml element.
///
/// Precondition: parent has no element with the given tagname.
/// Throws, via assertion failure, upon precondition violation.

template<typename T>
void set_element(xml::element& parent, std::string const& name, T const& t)
{
    LMI_ASSERT(parent.end() == parent.find(name.c_str()));
    xml::element z(name.c_str());
    xml_io<T>::to_xml(z, t);
    parent.push_back(z);
}

/// Deserialize a datum from a subelement of the given xml element.
///
/// Precondition: parent has an element with the given tagname.
/// Throws, via retrieve_element(), upon precondition violation.

template<typename T>
void get_element(xml::element const& parent, std::string const& name, T& t)
{
    xml::node::const_iterator i = xml_lmi::retrieve_element(parent, name.c_str());
    xml_io<T>::from_xml(*i, t);
}

// Implementation note. These convenience wrappers are intended for
// use outside this header. With early forward declarations, they
// could replace some occurrences of 'xml_io<...>::[to|from]_xml'
// above, but the benefit doesn't seem worth the loss of clarity.

/// Convenience wrapper for serialization to xml.

template<typename T>
void to_xml(xml::element& e, T const& t)
{
    xml_io<T>::to_xml(e, t);
}

/// Convenience wrapper for deserialization from xml.

template<typename T>
void from_xml(xml::element const& e, T& t)
{
    xml_io<T>::from_xml(e, t);
}
} // namespace xml_serialize

#endif // xml_serialize_hpp

