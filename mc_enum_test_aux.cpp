// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "mc_enum.tpp"                // Template class implementation.
#include "mc_enum_test_aux_enums.hpp" // Plain enums.

// Normally, one would prefer to instantiate all mc_enum templates
// in a single file. Here, however, the point is to instantiate one
// such template in a different translation unit than the unit-test
// driver, in order to prove that explicit instantion works.

extern enum_holiday const holiday_enums[] =
    {h_Theophany
    ,h_Easter
    ,h_Pentecost
    };
extern char const*const holiday_strings[] =
    {"Theophany"
    ,"Easter"
    ,"Pentecost"
    };
template class mc_enum<enum_holiday, 3, &holiday_enums, &holiday_strings>;

