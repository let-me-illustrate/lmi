// Instantiation of specific mc_enum types.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mc_enum_types.cpp,v 1.1 2005-02-03 16:03:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.tpp"            // Template class implementation.
#include "mc_enum_type_enums.hpp" // Plain enums.

// Don't do this:
//   #include "mc_enum_types.hpp"
// because that header specifies array bounds explicitly. Omitting
// that causes the bounds to be 'calculated' here [8.3.4/3], and the
// calculated bounds are checked at compile time.

// These lines would engender diagnostics
//   extern enum_option const option_enums[] = {option_A, option_B};
//   extern char const*const option_strings[] = {"A", "B", "C", "X"};
// at compile time when the template is explicitly instantiated.

extern enum_option const option_enums[] = {option_A, option_B, option_C};
extern char const*const option_strings[] = {"A", "B", "C"};
template class mc_enum<enum_option, 3, option_enums, option_strings>;

