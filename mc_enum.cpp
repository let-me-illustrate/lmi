// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: mc_enum.cpp,v 1.8 2006-07-10 13:14:34 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.hpp"

#include "alert.hpp"

#include <typeinfo>

mc_enum_base::mc_enum_base(int cardinality_of_the_enumeration)
    :allowed_(cardinality_of_the_enumeration, true)
{
}

void mc_enum_base::allow(int index, bool b)
{
    validate_index(index);
    allowed_[index] = b;
}

bool mc_enum_base::is_allowed(int index) const
{
    validate_index(index);
    return allowed_[index];
}

void mc_enum_base::validate_index(int index) const
{
    if(index < 0)
        {
        fatal_error()
            << "Index "
            << index
            << " is invalid for type '"
            << typeid(*this).name()
            << "': it must not be less than zero."
            << LMI_FLUSH
            ;
        }

    if(static_cast<int>(cardinality()) <= index)
        {
        fatal_error()
            << "Index "
            << index
            << " is invalid for type '"
            << typeid(*this).name()
            << "': it must less than "
            << cardinality()
            << "."
            << LMI_FLUSH
            ;
        }

    if(cardinality() != allowed_.size())
        {
        fatal_error()
            << "Number of allowable flags for type '"
            << typeid(*this).name()
            << "' is "
            << allowed_.size()
            << " but it should be "
            << cardinality()
            << " instead."
            << LMI_FLUSH
            ;
        }
}

