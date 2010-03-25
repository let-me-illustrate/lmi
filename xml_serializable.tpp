// Mixin for xml serialization.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_serializable.hpp"

#include "alert.hpp"
#include "any_member.hpp" // MemberSymbolTable<>
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits.hpp>
#else  // defined __BORLANDC__
#   define BOOST_STATIC_ASSERT(deliberately_ignored) class IgNoRe
#endif // defined __BORLANDC__

#include <xmlwrapp/nodes_view.h>

#include <algorithm>      // std::copy(), std::find()
#include <iterator>       // std::back_inserter
#include <vector>

template<typename T>
xml_serializable<T>::~xml_serializable()
{
    // Assert that static_cast<T cv&> doesn't engender undefined
    // behavior, and that the base class provides the expected
    // operator[]() and member_names() functions.
    //
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
           boost::is_base_and_derived<xml_serializable <T>,T>::value
        && boost::is_base_and_derived<MemberSymbolTable<T>,T>::value
        ));
}

template<typename T>
void xml_serializable<T>::read(xml::element const& x)
{
    T& t = static_cast<T&>(*this);

    if(xml_root_name() != x.get_name())
        {
        fatal_error()
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    std::string file_version_string;
    if(!xml_lmi::get_attr(x, "version", file_version_string))
        {
        fatal_error()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    int file_version = value_cast<int>(file_version_string);

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::map<std::string, std::string> detritus_map;

    std::list<std::string> residuary_names;
    std::copy
        (t.member_names().begin()
        ,t.member_names().end()
        ,std::back_inserter(residuary_names)
        );
    std::list<std::string>::iterator current_member;

    xml::const_nodes_view const elements(x.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi child = elements.begin(); child != elements.end(); ++child)
        {
        std::string node_tag(child->get_name());
        current_member = std::find
            (residuary_names.begin()
            ,residuary_names.end()
            ,node_tag
            );
        if(residuary_names.end() != current_member)
            {
            t[node_tag] = redintegrate_ex_ante
                (file_version
                ,node_tag
                ,xml_lmi::get_content(*child)
                );
            residuary_names.erase(current_member);
            }
        else if(is_detritus(node_tag))
            {
            // Hold certain obsolete entities that must be translated.
            detritus_map[node_tag] = xml_lmi::get_content(*child);
            }
        else
            {
            warning()
                << "XML tag '"
                << node_tag
                << "' not recognized by this version of the program."
                << LMI_FLUSH
                ;
            }
        }

    redintegrate_ex_post(file_version, detritus_map, residuary_names);

    redintegrate_ad_terminum();
}

template<typename T>
void xml_serializable<T>::write(xml::element& x) const
{
    T const& t = static_cast<T const&>(*this);

    xml::element root(xml_root_name().c_str());

// XMLWRAPP !! There's no way to set an integer attribute.
    std::string const version(value_cast<std::string>(class_version()));
    xml_lmi::set_attr(root, "version", version.c_str());

    std::vector<std::string>::const_iterator i;
    for(i = t.member_names().begin(); i != t.member_names().end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = t[node_tag].str();
        root.push_back(xml::element(node_tag.c_str(), value.c_str()));
        }

    x.push_back(root);
}

