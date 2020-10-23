// Serialization to and from xml.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef xml_serialize_hpp
#define xml_serialize_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

#include <string>
#include <type_traits>
#include <utility>                      // pair
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
    static_assert(!std::is_enum<T>::value); // Prefer mc_enum.

    static void to_xml(xml::element& e, T const& t)
    {
        e.set_text_content(value_cast<std::string>(t).c_str());
    }

    static void from_xml(xml::element const& e, T& t)
    {
        t = value_cast<T>(xml_lmi::get_content(e));
    }
};

/// Serialization for std::pair.

template<typename P>
struct xml_pair_io
{
    using T1 = typename P::first_type;
    using T2 = typename P::second_type;

    static_assert(std::is_same<P,std::pair<T1,T2>>::value);

    static void to_xml(xml::element& parent, P const& p)
    {
        parent.clear();

        // This is equivalent to calling set_element(), except that
        // the parent element has imperatively been cleared.
        xml::element e1("first");
        xml_io<T1>::to_xml(e1, p.first);
        parent.push_back(e1);

        xml::element e2("second");
        xml_io<T2>::to_xml(e2, p.second);
        parent.push_back(e2);
    }

    static void from_xml(xml::element const& parent, P& p)
    {
        auto const& view_first {parent.elements("first")};
        LMI_ASSERT(1 == lmi::ssize(view_first));
        xml_io<T1>::from_xml(*view_first.begin(), p.first);

        auto const& view_second {parent.elements("second")};
        LMI_ASSERT(1 == lmi::ssize(view_second));
        xml_io<T2>::from_xml(*view_second.begin(), p.second);
    }
};

template<typename T1, typename T2>
struct xml_io<std::pair<T1,T2>>
  :public xml_pair_io<std::pair<T1,T2>>
{};

/// Serialization for sequence containers.
///
/// Derive publicly from this to use its implementation when
/// specializing class template xml_io for a particular sequence.
///
/// from_xml() reads only <item> elements, ignoring other elements
/// (and non-element nodes) that might have been added manually,
/// e.g., as documentation.
///
/// C++11 has no way to assert that C is a Sequence; for the nonce,
/// no other Sequence being used, assert that it's a vector.

template<typename C>
struct xml_sequence_io
{
    using T = typename C::value_type;

    static_assert(std::is_same<C,std::vector<T>>::value);

    static void to_xml(xml::element& parent, C const& c)
    {
        parent.clear();
        for(auto const& i : c)
            {
            // This is not equivalent to calling set_element():
            // multiple <item> elements are expressly permitted.
            xml::element e("item");
            xml_io<T>::to_xml(e, i);
            parent.push_back(e);
            }
    }

    static void from_xml(xml::element const& parent, C& c)
    {
        c.clear();
        for(auto const& i : parent.elements("item"))
            {
            T t;
            xml_io<T>::from_xml(i, t);
            c.push_back(t);
            }
    }
};

template<typename T>
struct xml_io<std::vector<T>>
  :public xml_sequence_io<std::vector<T>>
{};

/// Serialize a datum into a subelement of the given xml element.
///
/// Precondition: parent has no element with the given tagname.
/// Throws, via assertion failure, upon precondition violation.

template<typename T>
void set_element(xml::element& parent, std::string const& name, T const& t)
{
    LMI_ASSERT(parent.end() == parent.find(name.c_str()));
    xml::element e(name.c_str());
    xml_io<T>::to_xml(e, t);
    parent.push_back(e);
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
