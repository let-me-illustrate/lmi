// Auxiliary functions for specific mc_enum types.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef mc_enum_types_aux_hpp
#define mc_enum_types_aux_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "so_attributes.hpp"

#include <string>
#include <vector>

/// Cardinality of certain enumerations, useful as ICEs.
///
/// Correctness of these values is asserted in the corresponding TU.

enum
    {mc_n_gen_bases    = 3
    ,mc_n_sep_bases    = 3
    ,mc_n_rate_periods = 2
    };

LMI_SO std::vector<std::string> const& all_strings_gender   ();
LMI_SO std::vector<std::string> const& all_strings_class    ();
LMI_SO std::vector<std::string> const& all_strings_smoking  ();
LMI_SO std::vector<std::string> const& all_strings_uw_basis ();
LMI_SO std::vector<std::string> const& all_strings_state    ();

LMI_SO std::vector<std::string> allowed_strings_emission();

mcenum_dbopt_7702 effective_dbopt_7702
    (mcenum_dbopt      actual_dbopt
    ,mcenum_dbopt_7702 rop_equivalent
    );

LMI_SO bool is_subject_to_ill_reg(mcenum_ledger_type);

bool is_three_rate_finra(mcenum_ledger_type);

/// Transform a plain enumerator to its string equivalent.
///
/// This message:
///   https://lists.nongnu.org/archive/html/lmi/2008-08/msg00022.html
/// explains why this template is explicitly instantiated in the
/// accompanying TU, and not defined here.

template<typename T>
std::string mc_str(T);

LMI_SO mcenum_emission mc_emission_from_string(std::string const&);

mcenum_state mc_state_from_string(std::string const&);

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
