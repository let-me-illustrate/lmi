// Extended enumerations for life insurance.
//
// Copyright (C) 2001, 2002, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xenum.cpp,v 1.3 2005-07-16 22:51:04 chicares Exp $

// Definitions for template class xenum.

#ifndef OK_TO_COMPILE_XENUM_CPP
#   error This file is not intended for standalone compilation.
#endif // OK_TO_COMPILE_XENUM_CPP

#include "alert.hpp"

#include <ios>
#include <istream>
#include <ostream>

template <typename EnumType, int N>
xenum<EnumType, N>::xenum()
    :representation(enumerators[0])
{
}

template <typename EnumType, int N>
xenum<EnumType, N>::xenum(xenum const& x)
    :representation(x.representation)
{
}

template <typename EnumType, int N>
xenum<EnumType, N>::xenum(EnumType e)
    :representation(e)
{
}

template <typename EnumType, int N>
xenum<EnumType, N>::xenum(std::string const& s)
{
    set_value(s);
}

template <typename EnumType, int N>
xenum<EnumType, N>& xenum<EnumType, N>::operator=(xenum<EnumType, N> const& x)
{
    representation = x.representation;
    return *this;
}

template <typename EnumType, int N>
xenum<EnumType, N>& xenum<EnumType, N>::operator=(EnumType e)
{
    representation = e;
    return *this;
}

template <typename EnumType, int N>
xenum<EnumType, N>& xenum<EnumType, N>::operator=(std::string const& s)
{
    set_value(s);
    return *this;
}

template <typename EnumType, int N>
bool xenum<EnumType, N>::operator==(xenum<EnumType, N> const& x) const
{
    return representation == x.representation;
}

template <typename EnumType, int N>
bool xenum<EnumType, N>::operator==(EnumType e) const
{
    return representation == e;
}

template <typename EnumType, int N>
bool xenum<EnumType, N>::operator==(std::string const& s) const
{
    return str() == s;
}

template <typename EnumType, int N>
EnumType xenum<EnumType, N>::value() const
{
    return representation;
}

template <typename EnumType, int N>
std::string const& xenum<EnumType, N>::str() const
{
    std::ptrdiff_t index =
        std::find
            (enumerators
            ,enumerators + N
            ,representation
            )
        - enumerators
        ;
    if(N == index)
        {
/*
        std::ostringstream error;
        error
            << "Invalid enumerator: '"
            << representation
            << "' out of range for type '"
            << typeid(EnumType).name()
            << "'."
            ;
// TODO ?? Don't want to throw until code more stable....
//        throw std::range_error(error.str());
*/
        hobsons_choice()
            << "Invalid enumerator: '"
            << representation
            << "' out of range for type '"
            << typeid(EnumType).name()
            << "'."
            << LMI_FLUSH
            ;
        return all_strings()[0];
        }
    else
        {
        return all_strings()[index];
        }
// TODO We'd rather just write:
/*
    return all_strings()
        [
        std::find
            (enumerators
            ,enumerators + N
            ,representation
            )
        - enumerators
        ];
*/
}

template <typename EnumType, int N>
std::vector<std::string> const& xenum<EnumType, N>::all_strings()
{
    static std::vector<std::string> const v(names, names + N);
    return v;
}

template <typename EnumType, int N>
void xenum<EnumType, N>::set_value(std::string const& s)
{
    std::ptrdiff_t index =
        std::find
            (all_strings().begin()
            ,all_strings().end()
            ,s
            )
        - all_strings().begin()
        ;

    if(N == index)
        {
/*
        std::ostringstream error;
        error
            << "'"
            << s
            << "' out of range for type '"
            << typeid(EnumType).name()
            << "'."
            ;
// TODO ?? Don't want to throw until code more stable....
//        throw std::range_error(error.str());
*/
        hobsons_choice()
            << "'"
            << s
            << "' out of range for type '"
            << typeid(EnumType).name()
            << "'."
            << LMI_FLUSH
            ;
        representation = enumerators[0];
        }
    else
        {
        representation = enumerators[index];
        }
}

// We prefer to implement streaming operators in terms of names
// rather than enumerators. It's too inflexible to forbid names
// to contain embedded spaces, but it's not easy to read names
// that contain spaces without std::locale support: if
//   'some_value'
//   'some_value 1'
// are among the names for a given type, then we can't know
// whether to stop reading after the letter 'e'; and if the
// next field is an integer, the problem is intractable, unless
// we (quite reasonably) forbid names to end in a space.
//
// I see two ways to approach this:
//  - delimit enums with a newline
//  - change embedded spaces to a different character
// The second approach is chosen here, but the choice is arbitrary.
//
// Later changed: peek next character, and continue extracting until
// it's not a blank. This maintains backward compatibility, but does
// not secure forward compatibility: old versions of the code won't
// read input files produced with new versions, but that's generally
// the case anyway. This breaks borland builds because of a defect in
// their standard library.

/*
// Here's an alternative approach.
// TODO ?? Decide which one we want.

template<typename EnumType, int N>
std::istream& operator>>(std::istream& is, xenum<EnumType, N>& x)
{
    std::string s;
    nonstd::str_get(is, s);
    x = s;
    return is;
}

template<typename EnumType, int N>
std::ostream& operator<<(std::ostream& os, xenum<EnumType, N> const& x)
{
    nonstd::str_put(os, x.str());
    return os;
}
*/

///*
template<typename EnumType, int N>
std::istream& operator>>(std::istream& is, xenum<EnumType, N>& x)
{
    static std::string const underbar("_");
    std::string s;
    is >> s;
    if(!is)
        {
        // We suppose that this will trigger some error downstream.
        x = s;
        return is;
        }

    for(;;)
        {
        // COMPILER !! Versions 5.02 and 5.5.1 of the borland compiler
        // both seem to eat whitespace at the end of an inserted
        // std::string, as 21.3.7.9 would not seem to permit. See
        //   <3c59a5fa$0$5155$724ebb72@reader3.ash.ops.us.uu.net>
//        http://groups.google.com/groups?selm=3c59a5fa$0$5155$724ebb72@reader3.ash.ops.us.uu.net
        int next_char = is.peek();
// TODO ?? To work around the borland problem, try (untested) changing
// the 'break' condition to
//        if('<' == next_char)
// That'll still fail the unit test, but might read input files
// successfully.
        if(' ' != next_char)
            break;
        std::string more;
        is >> more;
        s += ' ' + more;
        }
    is.clear();

// Keep doing this for backward compatibility.
    for(;;)
        {
        std::string::size_type xpos = s.find_first_of(underbar);
        if(std::string::npos == xpos)
            {
            break;
            }
        else
            {
            s[xpos] = ' ';
            }
        }

    x = s;
    return is;
}

template<typename EnumType, int N>
std::ostream& operator<<(std::ostream& os, xenum<EnumType, N> const& x)
{
/* Do not do this anymore.
    static std::string const space(" ");
    std::string s = x.str();

    for(;;)
        {
        std::string::size_type xpos = s.find_first_of(space);
        if(std::string::npos == xpos)
            {
            break;
            }
        else
            {
            s[xpos] = '_';
            }
        }

    os << s;
*/

    os << x.str();
    return os;
}
//*/

