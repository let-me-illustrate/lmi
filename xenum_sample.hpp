// Extended enumeration type--sample for unit testing.
//
// Copyright (C) 2001, 2003, 2005 Gregory W. Chicares.
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

// $Id: xenum_sample.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

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

#if !defined BC_BEFORE_5_5
// Any attempted use of this should be a compile-time error.
// COMPILER !! bc++5.02 cannot handle this even if it's not used.
enum enum_empty
  {
  };
typedef xenum<enum_empty, 0> e_empty;
#endif // old borland compiler

#endif // xenum_sample_hpp

