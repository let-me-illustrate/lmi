// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: mc_enum.cpp,v 1.2 2005-02-19 03:27:45 chicares Exp $

#include "config.hpp"

#include "mc_enum.hpp"

mc_enum_base::mc_enum_base(int n)
    :allowed_(n, true)
{
}

void mc_enum_base::allow(int item, bool b)
{
    allowed_[item] = b;
}

bool mc_enum_base::is_allowed(int item) const
{
    return allowed_[item];
}

bool mc_enum_base::is_valid(std::string const&) const
{
    return true;
}

