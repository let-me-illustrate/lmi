// Extended enumeration type--sample for unit testing.
//
// Copyright (C) 2001, 2002, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xenum_sample.cpp,v 1.5 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xenum_sample.hpp"

// Sample extended enums for unit testing. Not intended for use in any
// other context. There are extended enums elsewhere for production use;
// if we used them for unit testing, then the unit test would depend on
// production code, but we want it to stand alone. It is important to
// separate this file from the main unit test, because we want these
// globals defined in a separate translation unit in order to unmask
// initialization-order dependencies to the extent possible.

template<> enum_123 const e_123::enumerators[] =
    {e_one
    ,e_two
    ,e_three
// Unfortunately, omitting an initializer isn't a compile-time error.
// But adding an extra one should elicit a diagnostic:
//  ,e_one
    };

template<> char const*const e_123::names[] =
    {"1 One"
    ,"2 Two"
    ,"3 Three"
    };

template<> enum_foobar const e_foobar::enumerators[] =
    {e_foo
    ,e_bar
    };

template<> char const*const e_foobar::names[] =
    {"foo"
    ,"bar"
    };

#ifdef LMI_EXPLICIT_INSTANTIATION

#   define OK_TO_COMPILE_XENUM_CPP
#   include "xenum.cpp"
#   undef OK_TO_COMPILE_XENUM_CPP

template class xenum<enum_123, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_123, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_123, 3> const&);

template class xenum<enum_foobar, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_foobar, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_foobar, 2> const&);

// This demonstrates something that's a compile-time error.
//template class xenum<enum_empty, 0>;
//template std::istream& operator>> (std::istream& is, xenum<enum_empty, 0>&);
//template std::ostream& operator<< (std::ostream& os, xenum<enum_empty, 0> const&);

#endif // LMI_EXPLICIT_INSTANTIATION

