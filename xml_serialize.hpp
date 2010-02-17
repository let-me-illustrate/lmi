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
#include "mc_enum.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/utility.hpp>

#include <xmlwrapp/xmlwrapp.h>

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>


/// Helpers for serialization to XML

namespace xml_serialize
{
    // forward declarations
    template<typename T> void to_xml(xml::node& out, T const& in);
    template<typename T> void from_xml(T& out, xml::node const& in);

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
            if(!s.good())
                throw std::runtime_error("Failure to serialize data into XML.");
            out.set_content(s.str().c_str());
        }

        // reads value from nodes under node 'in' into 'out'
        static void from_xml(T& out, xml::node const& in)
        {
            std::istringstream s(xml_lmi::get_content(in));
            s >> out;
            if(s.fail() || s.bad())
                throw std::runtime_error("Failure to read data from XML.");
            if(!s.eof())
                throw std::runtime_error("Unexpected extra data in XML.");
        }
    };

    // specialization for std::string is both more efficient and required for
    // correct reading of strings with whitespace in them
    template<>
    struct type_io<std::string>
    {
        static void to_xml(xml::node& out, std::string const& in)
        {
            out.set_content(in.c_str());
        }

        static void from_xml(std::string& out, xml::node const& in)
        {
            out = xml_lmi::get_content(in);
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
                ::xml_serialize::from_xml(e, *i);
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
    struct enum_type_io
    {
        typedef mc_enum<T> mce_type;

        static void to_xml(xml::node& out, T const& in)
        {
            ::xml_serialize::to_xml(out, mc_enum<T>(in));
        }

        static void from_xml(T& out, xml::node const& in)
        {
            mce_type x;
            ::xml_serialize::from_xml(x, in);
            out = x.value();
        }
    };

    /// At first sight, this only adds another level of indirection.
    /// But it actually is useful: it allows us to specialize type_io<> not
    /// only for particular types, but for *groups* of types. In particular,
    /// it makes it easy to use enum_type_io for all enum types.
    ///
    /// See http://www.boost.org/libs/utility/enable_if.html for explanation
    /// of how this works.

    template<typename T,
             typename Enable = void /* for enable_if below */>
    struct choose_type_io
    {
        typedef type_io<T> type;
    };

    template<typename T>
    struct choose_type_io<T, typename boost::enable_if< boost::is_enum<T> >::type>
    {
        typedef enum_type_io<T> type;
    };

    /// Serializes 'value' into XML node 'node'.
    /// This is convenience wrapper around type_io<>::to_xml().
    template<typename T>
    void to_xml(xml::node& out, T const& in)
    {
        choose_type_io<T>::type::to_xml(out, in);
    }

    /// Deserializes 'value' from XML node 'node'.
    /// This is convenience wrapper around type_io<>::from_xml().
    template<typename T>
    void from_xml(T& out, xml::node const& in)
    {
        choose_type_io<T>::type::from_xml(out, in);
    }

    /// Adds a property to given XML node (root).
    /// A property is serialized as <property-name>value</property-name>.
    /// Serialization format is defined by xml_serialize::type_io<T>;
    /// by default, C++ streams' operator<< is used.

    template<typename T>
    void add_property
        (xml::element& root
        ,const char* prop
        ,T const& value)
    {
        xml::element node(prop);
        to_xml(node, value);
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
            from_xml(value, *n);
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
        from_xml(value, *n);
    }
} // namespace xml_serialize

#endif // xml_serialize_hpp

