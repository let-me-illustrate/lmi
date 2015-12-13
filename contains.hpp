// Ascertain whether a "container" includes a given element.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef contains_hpp
#define contains_hpp

#include "config.hpp"

#include <boost/utility/enable_if.hpp>

#include <algorithm> // std::find()

#if defined __BORLANDC__ || defined __COMO_VERSION__ && __COMO_VERSION__ <= 4303
#   define LMI_NO_SFINAE
#   include <deque>
#   include <list>
#   include <map>
#   include <set>
#   include <string>
#   include <vector>
#endif // defined __BORLANDC__ || defined __COMO_VERSION__ && __COMO_VERSION__ <= 4303

#if !defined LMI_NO_SFINAE

/// Determine whether a class has a find() member.
///
/// See:
///   http://lists.nongnu.org/archive/html/lmi/2010-05/msg00008.html
/// I don't know who invented the technique. This implementation
/// largely follows this pseudonymous posting:
///   http://www.rsdn.ru/forum/cpp/2720363.aspx
/// which, however, was foreshadowed here:
///   http://lists.boost.org/Archives/boost/2002/03/27233.php

template<typename T>
struct has_member_find
{
    typedef char (&yea_t)[1];
    typedef char (&nay_t)[2];

    struct Mixin {void find();};
    struct Derived : T, Mixin {};

    template<typename PMF, PMF pmf> struct spoiler;

    template<typename D>
    static nay_t deduce(D*, spoiler<void (Mixin::*)(), &D::find>* = 0);

    static yea_t deduce(...);

    static bool const value = sizeof(yea_t) == sizeof deduce((Derived*)(0));
};

/// Ascertain whether a "container" includes a given element.
///
/// Here, std::basic_string is considered a "container", this:
///   http://www.comeaucomputing.com/iso/lwg-active.html#718
///   "basic_string is not a sequence"
///  notwithstanding.
///
/// This overload is for "containers" that have no find() member.

template<typename T>
bool contains
    (T const& t
    ,typename T::value_type const& element
    ,typename boost::disable_if<has_member_find<T> >::type* = 0
    )
{
    return t.end() != std::find(t.begin(), t.end(), element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'traits_type::char_type', assume it behaves like std::basic_string.
///
/// This overload is for std::basic_string::find(std::basic_string).

template<typename T>
bool contains
    (T const& t
    ,T const& element
    ,typename T::traits_type::char_type const* = 0
    )
{
    return T::npos != t.find(element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'traits_type::char_type', assume it behaves like std::basic_string.
///
/// This overload is for std::basic_string::find(traits_type::char_type const*).

template<typename T>
bool contains
    (T const& t
    ,typename T::traits_type::char_type const* element
    )
{
    return T::npos != t.find(element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'traits_type::char_type', assume it behaves like std::basic_string.
///
/// This overload is for std::basic_string::find(traits_type::char_type).

template<typename T>
bool contains
    (T const& t
    ,typename T::traits_type::char_type element
    )
{
    return T::npos != t.find(element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'key_type', assume it behaves like an associative container.

template<typename T>
bool contains
    (T const& t
    ,typename T::key_type const& element
    )
{
    return t.end() != t.find(element);
}

#else // defined LMI_NO_SFINAE

// Strings. Only std::string is supported here: compilers that can't
// handle SFINAE might not provide std::basic_string either.

bool contains(std::string const& t, std::string const& element)
{
    return std::string::npos != t.find(element);
}

bool contains(std::string const& t, char const* element)
{
    return std::string::npos != t.find(element);
}

bool contains(std::string const& t, char element)
{
    return std::string::npos != t.find(element);
}

// Associative containers.

template<typename K, typename T>
bool contains(std::map<K,T> const& t, typename std::map<K,T>::key_type const& element)
{
    return t.end() != t.find(element);
}

template<typename T>
bool contains(std::set<T> const& t, typename std::set<T>::key_type const& element)
{
    return t.end() != t.find(element);
}

// Sequences.

template<typename T>
bool contains(std::deque<T> const& t, T const& element)
{
    return t.end() != std::find(t.begin(), t.end(), element);
}

template<typename T>
bool contains(std::list<T> const& t, T const& element)
{
    return t.end() != std::find(t.begin(), t.end(), element);
}

template<typename T>
bool contains(std::vector<T> const& t, T const& element)
{
    return t.end() != std::find(t.begin(), t.end(), element);
}

#endif // defined LMI_NO_SFINAE

#endif // contains_hpp

