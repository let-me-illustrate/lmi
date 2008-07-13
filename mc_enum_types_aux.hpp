// Auxiliary functions for specific mc_enum types.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

// $Id: mc_enum_types_aux.hpp,v 1.3 2008-07-13 18:24:16 chicares Exp $

#ifndef mc_enum_types_aux_hpp
#define mc_enum_types_aux_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"

/// Cardinality of certain enumerations, useful as ICEs.
///
/// Correctness of these values is asserted in the corresponding TU.

enum
    {mc_n_gen_bases    = 3
    ,mc_n_sep_bases    = 3
    ,mc_n_rate_periods = 2
    };

bool is_subject_to_ill_reg(mcenum_ledger_type);

bool is_three_rate_nasd(mcenum_ledger_type);

void set_cloven_bases_from_run_basis
    (mcenum_run_basis
    ,mcenum_gen_basis&
    ,mcenum_sep_basis&
    );

void set_run_basis_from_cloven_bases
    (mcenum_run_basis&
    ,mcenum_gen_basis
    ,mcenum_sep_basis
    );

#endif // mc_enum_types_aux_hpp

