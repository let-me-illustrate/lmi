// 1980, 2001, and 2017 CSO tables.
//
// Copyright (C) 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef cso_table_hpp
#define cso_table_hpp

#include "config.hpp"

#include "mc_enum_types.hpp"
#include "oecumenic_enumerations.hpp"
#include "so_attributes.hpp"

#include <vector>

LMI_SO std::vector<double> cso_table
    (mcenum_cso_era
    ,oenum_autopisty
    ,oenum_alb_or_anb
    ,mcenum_gender
    ,mcenum_smoking
    ,int              min_age
    ,int              max_age
    );

LMI_SO std::vector<double> cso_table
    (mcenum_cso_era
    ,oenum_autopisty
    ,oenum_alb_or_anb
    ,mcenum_gender
    ,mcenum_smoking
    );

#endif // cso_table_hpp
