// Ascertain whether a "container" includes a given element.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

#include <algorithm> // std::find()

#if defined __BORLANDC__ || defined __COMO_VERSION__ && __COMO_VERSION__ <= 4303
#   define LMI_NO_SFINAE
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
#endif // !defined LMI_NO_SFINAE

/// Ascertain whether a "container" includes a given element.
///
/// Here, consider std::basic_string is considered a "container",
///   http://www.comeaucomputing.com/iso/lwg-active.html#718
///   "basic_string is not a sequence"
///  notwithstanding.
///
/// This overload is for "containers" that aren't handled specially.

template<typename T>
bool contains(T const& t, typename T::value_type const& element)
{
    return t.end() != std::find(t.begin(), t.end(), element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'traits_type::char_type', assume it behaves like std::basic_string.
///
/// This overload is for std::basic_string::find(std::basic_string).

template<typename T>
bool contains(T const& t, T const& element, typename T::traits_type::char_type const* = 0)
{
    return T::npos != t.find(element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'traits_type::char_type', assume it behaves like std::basic_string.
///
/// This overload is for std::basic_string::find(traits_type::char_type const*).

template<typename T>
bool contains(T const& t, typename T::traits_type::char_type const* element)
{
    return T::npos != t.find(element);
}

/// Ascertain whether a "container" includes a given element.
///
/// If T has 'key_type', assume it behaves like an associative container.

template<typename T>
bool contains(T const& t, typename T::key_type const& element)
{
    return t.end() != t.find(element);
}

#endif // contains_hpp

