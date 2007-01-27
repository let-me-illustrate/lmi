// Extended enumeration type--sample for unit testing.
//
// Copyright (C) 2001, 2003, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: xenum_sample.hpp,v 1.4 2007-01-27 00:00:52 wboutin Exp $

#ifndef xenum_sample_hpp
#define xenum_sample_hpp

#include "config.hpp"

#include "xenum.hpp"

enum enum_123
    {e_one
    ,e_two
    ,e_three
    };
typedef xenum<enum_123, 3> e_123;

enum enum_foobar
    {e_foo
    ,e_bar
    };
typedef xenum<enum_foobar, 2> e_foobar;

// Any attempted use of this should be a compile-time error.
enum enum_empty
  {
  };
typedef xenum<enum_empty, 0> e_empty;

#endif // xenum_sample_hpp

