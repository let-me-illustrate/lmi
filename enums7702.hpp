// Enumerators for 7702 and 7702A.
//
// Copyright (C) 2003, 2005, 2006 Gregory W. Chicares.
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

// $Id: enums7702.hpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifndef enums7702_hpp
#define enums7702_hpp

#include "config.hpp"

enum enum_defn_life_ins
    {e_cvat
    ,e_gpt
    ,e_noncompliant
    };

enum enum_mec_avoid_method
    {e_allow_mec
    ,e_reduce_prem
    ,e_increase_specamt
    };

enum enum_defn_material_change
    {e_unnecessary_premium
    ,e_benefit_increase
    ,e_later_of_increase_or_unnecessary_premium
    ,e_earlier_of_increase_or_unnecessary_premium
    ,e_adjustment_event
    };

#endif // enums7702_hpp

