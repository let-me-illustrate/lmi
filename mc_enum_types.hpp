// Declaration of specific mc_enum types.
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

#ifndef mc_enum_types_hpp
#define mc_enum_types_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"
#include "mc_enum_type_enums.hpp"

// Here write illustrative examples and anything that doesn't follow
// the macro paradigm, such as enumerators with nonsuccessive values.

typedef mc_enum<enum_option    > e_option          ;
typedef mc_enum<mcenum_emission> e_emission        ;
typedef mc_enum<rounding_style > mce_rounding_style;

#define MC_DECLARE(TYPE) \
typedef mc_enum<mcenum_##TYPE> mce_##TYPE;

MC_DECLARE(yes_or_no)
MC_DECLARE(gender)
MC_DECLARE(smoking)
MC_DECLARE(class)
MC_DECLARE(dbopt)
MC_DECLARE(dbopt_7702)
MC_DECLARE(mode)
MC_DECLARE(gen_basis)
MC_DECLARE(sep_basis)
MC_DECLARE(rate_period)
MC_DECLARE(run_basis)
MC_DECLARE(ledger_type)
MC_DECLARE(uw_basis)
MC_DECLARE(table_rating)
MC_DECLARE(solve_type)
MC_DECLARE(solve_target)
MC_DECLARE(pmt_strategy)
MC_DECLARE(sa_strategy)
MC_DECLARE(gen_acct_rate_type)
MC_DECLARE(sep_acct_rate_type)
MC_DECLARE(loan_rate_type)
MC_DECLARE(fund_input_method)
MC_DECLARE(run_order)
MC_DECLARE(survival_limit)
MC_DECLARE(term_adj_method)
MC_DECLARE(state)
MC_DECLARE(country)
MC_DECLARE(defn_life_ins)
MC_DECLARE(mec_avoid_method)
MC_DECLARE(defn_material_change)
MC_DECLARE(spread_method)
MC_DECLARE(coi_rate_method)
MC_DECLARE(anticipated_deduction)
MC_DECLARE(part_mort_table)
MC_DECLARE(premium_table)
MC_DECLARE(from_point)
MC_DECLARE(to_point)
MC_DECLARE(report_column)

#undef MC_DECLARE

#endif // mc_enum_types_hpp

