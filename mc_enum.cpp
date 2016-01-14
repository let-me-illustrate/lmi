// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.hpp"

#include <algorithm> // std::find()

mc_enum_base::mc_enum_base(int cardinality_of_the_enumeration)
    :allowed_(cardinality_of_the_enumeration, true)
{
}

/// Permit or forbid a particular enumerative choice.
///
/// The 'index' argument is the index of the enumerator within the
/// enumeration (i.e, what ordinal() returns), which does not
/// necessarily equal the value of the enumerator.
///
/// Throws (via at()) if 'index' is out of range.

void mc_enum_base::allow(int index, bool b)
{
    allowed_.at(index) = b;
}

void mc_enum_base::allow_all(bool b)
{
    for(std::size_t j = 0; j < cardinality(); ++j)
        {
        allow(j, b);
        }
}

std::size_t mc_enum_base::first_allowed_ordinal() const
{
    return std::find(allowed_.begin(), allowed_.end(), true) - allowed_.begin();
}

bool mc_enum_base::is_allowed(int index) const
{
    return allowed_.at(index);
}

