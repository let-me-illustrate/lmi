// TODO ?? Goofy compiler workarounds that should be expunged.
//
// Copyright (C) 2001, 2005 Gregory W. Chicares.
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

// $Id: kludges.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// This is an exception to our general rule that "config.hpp" is to be
// included only in headers, for good reasons:
//   this file is a kludge anyway;
//   this file includes no other headers;
//   "config.hpp" is the natural header for this file.
#include "config.hpp"

// See comments in config.hpp on redundant and reverse include guards.
#ifdef BC_BEFORE_5_5
#   define OK_TO_COMPILE_KLUDGES_BC502_CPP
#   include "kludges_bc502.cpp"
#   undef OK_TO_COMPILE_KLUDGES_BC502_CPP
#endif // BC_BEFORE_5_5

