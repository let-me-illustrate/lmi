// Declaration of specific mc_enum types.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mc_enum_types.hpp,v 1.1.1.1 2004-05-15 19:59:04 chicares Exp $

#ifndef mc_enum_types_hpp
#define mc_enum_types_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"        // Template class forward declaration.
#include "mc_enum_type_enums.hpp" // Plain enumerators.

extern enum_option const option_enums[3];
extern char const*const option_strings[3];
typedef mc_enum<enum_option, 3, option_enums, option_strings> e_option;

#endif // mc_enum_types_hpp

