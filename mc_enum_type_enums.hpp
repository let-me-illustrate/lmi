// Plain enumerators for specific mc_enum types.
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

// $Id: mc_enum_type_enums.hpp,v 1.3 2005-03-10 04:44:06 chicares Exp $

#ifndef mc_enum_type_enums_hpp
#define mc_enum_type_enums_hpp

#include "config.hpp"

// TODO ?? Move this one to a 'test' file.

enum enum_option {option_A, option_B, option_C};

#include "mc_enum_types.xpp"

#define MC_DEFINE_ENUMS(TYPE) \
enum mcenum_##TYPE \
TYPE##_VALUES

MC_DEFINE_ENUMS(yes_or_no)
MC_DEFINE_ENUMS(gender)
MC_DEFINE_ENUMS(smoking)
MC_DEFINE_ENUMS(class)
MC_DEFINE_ENUMS(dbopt)
MC_DEFINE_ENUMS(dbopt_7702)
MC_DEFINE_ENUMS(mode)
MC_DEFINE_ENUMS(basis)
MC_DEFINE_ENUMS(sep_acct_basis)
MC_DEFINE_ENUMS(rate_period)
MC_DEFINE_ENUMS(run_basis)
MC_DEFINE_ENUMS(ledger_type)
MC_DEFINE_ENUMS(uw_basis)
MC_DEFINE_ENUMS(table_rating)
MC_DEFINE_ENUMS(solve_type)
MC_DEFINE_ENUMS(solve_target)
MC_DEFINE_ENUMS(solve_tgt_at)
MC_DEFINE_ENUMS(solve_from)
MC_DEFINE_ENUMS(solve_to)
MC_DEFINE_ENUMS(pmt_strategy)
MC_DEFINE_ENUMS(sa_strategy)
MC_DEFINE_ENUMS(loan_strategy)
MC_DEFINE_ENUMS(wd_strategy)
MC_DEFINE_ENUMS(interest_rate_type)
MC_DEFINE_ENUMS(loan_rate_type)
MC_DEFINE_ENUMS(fund_input_method)
MC_DEFINE_ENUMS(run_order)
MC_DEFINE_ENUMS(survival_limit)
MC_DEFINE_ENUMS(term_adj_method)
MC_DEFINE_ENUMS(state)
MC_DEFINE_ENUMS(country)
MC_DEFINE_ENUMS(defn_life_ins)
MC_DEFINE_ENUMS(mec_avoid_method)
MC_DEFINE_ENUMS(defn_material_change)
MC_DEFINE_ENUMS(spread_method)
MC_DEFINE_ENUMS(coi_rate_method)
MC_DEFINE_ENUMS(anticipated_deduction)
MC_DEFINE_ENUMS(part_mort_table)
MC_DEFINE_ENUMS(premium_table)
MC_DEFINE_ENUMS(from_point)
MC_DEFINE_ENUMS(to_point)
MC_DEFINE_ENUMS(report_column)
MC_DEFINE_ENUMS(contract_name)

#undef MC_DEFINE_ENUMS

#endif // mc_enum_type_enums_hpp

