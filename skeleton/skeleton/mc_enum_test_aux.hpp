// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
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

// $Id: mc_enum_test_aux.hpp,v 1.1.1.1 2004-05-15 19:59:04 chicares Exp $

#ifndef mc_enum_test_aux_hpp
#define mc_enum_test_aux_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"
#include "mc_enum_test_aux_enums.hpp"

extern enum_holiday const holiday_enums[3];
extern char const*const holiday_strings[3];
typedef mc_enum<enum_holiday, 3, holiday_enums, holiday_strings> e_holiday;

#endif // mc_enum_test_aux_hpp

