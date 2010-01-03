// XML serialization helpers.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

#include "xml_lmi.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <xmlwrapp/xmlwrapp.h>

#include <string>
#include <sstream>
#include <vector>


/// Helpers for serialization to XML

namespace xml_serialize
{
    /// Serialization and deserialization of a type to/from XML.
    ///
    /// This template must be specialized for all nontrivial types that
    /// will be serialized to XML.

    template<typename T>
    struct type_io
    {
        // writes value 'in' under existing node 'out'
        static void to_xml(xml::node& out, T const& in)
        {
            std::ostringstream s;
            s << in;
            out.set_content(s.str().c_str());
        }

        // reads value from nodes under node 'in' into 'out'
        static void from_xml(T& out, xml::node const& in)
        {
            std::istringstream s(xml_lmi::get_content(in));
            s >> out;
        }
    };

    template<typename T>
    struct container_type_io
    {
        typedef typename T::value_type element_type;

        static void to_xml(xml::node& out, T const& in)
        {
            for(typename T::const_iterator i = in.begin(); i != in.end(); ++i)
                {
                add_property(out, "item", *i);
                }
        }

        static void from_xml(T& out, xml::node const& in)
        {
            out.clear();
            xml::const_nodes_view items = in.elements("item");
            for(xml::const_nodes_view::iterator i = items.begin(); i != items.end(); ++i)
                {
                element_type e;
                type_io<element_type>::from_xml(e, *i);
                out.push_back(e);
                }
        }
    };

    template<typename T>
    struct type_io< std::vector<T> >
        : public container_type_io< std::vector<T> >
    {
    };

    template<typename T>
    struct enum_type_io_map
    {
        struct MapEntry
        {
            T value;
            const char *as_str;
        };

        static const char *to_str(T x)
        {
            for(size_t i = 0; i < sizeof(map)/sizeof(MapEntry); i++)
                {
                if(map[i].value == x)
                    return map[i].as_str;
                }

            throw std::logic_error("invalid enum value");
        }

        static T from_str(const char *x)
        {
            for(size_t i = 0; i < sizeof(map)/sizeof(MapEntry); i++)
                {
                if(strcmp(map[i].as_str, x) == 0)
                    return map[i].value;
                }

            std::string msg = "Invalid enum value '";
            msg += x;
            msg += "'.";
            throw std::runtime_error(msg);
        }

    private:
        // This array must be defined somewhere for every serialized enum type.
        static const MapEntry map[];
    };

    template<typename T>
    struct enum_type_io
    {
        static void to_xml(xml::node& out, T const& in)
        {
            out.set_content(enum_type_io_map<T>::to_str(in));
        }

        static void from_xml(T& out, xml::node const& in)
        {
            out = enum_type_io_map<T>::from_str(xml_lmi::get_content(in).c_str());
        }
    };


    /// Adds a property to given XML node (root).
    /// A property is serialized as <property-name>value</property-name>,
    /// using C++ streams' operator<<.

    template<typename T>
    void add_property
        (xml::element& root
        ,const char* prop
        ,T const& value)
    {
        xml::element node(prop);
        type_io<T>::to_xml(node, value);
        root.push_back(node);
    }

    /// Reads a property from given XML node (root) and stores it in
    /// 'value'. Returns true if it was found, false otherwise (in which
    /// case the default value is assigned to 'value').

    template<typename T>
    bool get_property
        (xml::element const& root
        ,const char* prop
        ,T& value
        ,T const& default_value)
    {
        xml::node::const_iterator n = root.find(prop);
        if(root.end() != n)
            {
            type_io<T>::from_xml(value, *n);
            return true;
            }
        else
            {
            value = default_value;
            return false;
            }
    }

    /// Reads a property from given XML node (root) and stores it in
    /// 'value'. Throws if the property wasn't found.

    template<typename T>
    void get_property
        (xml::element const& root
        ,const char* prop
        ,T& value)
    {
        xml::node::const_iterator n = root.find(prop);
        if(root.end() == n)
            {
                std::string msg = "Required property '";
                msg += prop;
                msg += "' is missing.";
                throw std::runtime_error(msg);
            }
        type_io<T>::from_xml(value, *n);
    }
} // namespace xml_serialize

#endif // xml_serialize_hpp

