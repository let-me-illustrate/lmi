// Instantiation of specific mc_enum types.
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

// $Id: mc_enum_types.cpp,v 1.6 2005-04-23 02:06:52 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.tpp"            // Template class implementation.
#include "mc_enum_type_enums.hpp" // Plain enums.

// Don't do this:
//   #include "mc_enum_types.hpp"
// because that header specifies array bounds explicitly. Omitting
// that causes the bounds to be 'calculated' here [8.3.4/3], and the
// calculated bounds are checked at compile time.

// These lines would engender diagnostics
//   extern enum_option const option_enums[] = {option_A, option_B};
//   extern char const*const option_strings[] = {"A", "B", "C", "X"};
// at compile time when the template is explicitly instantiated.

// TODO ?? Move this one to a 'test' file.

extern enum_option const option_enums[] = {option_A, option_B, option_C};
extern char const*const option_strings[] = {"A", "B", "C"};
template class mc_enum<enum_option, 3, option_enums, option_strings>;

#include "mc_enum_types.xpp"

#define MC_DEFINE(TYPE,NUMBER) \
extern mcenum_##TYPE const TYPE##_enums[] = TYPE##_VALUES \
extern char const*const TYPE##_strings[] = TYPE##_NAMES \
template class mc_enum<mcenum_##TYPE, NUMBER, TYPE##_enums, TYPE##_strings>;

MC_DEFINE(yes_or_no,2)
MC_DEFINE(gender,3)
MC_DEFINE(smoking,3)
MC_DEFINE(class,4)
MC_DEFINE(dbopt,3)
MC_DEFINE(dbopt_7702,2)
MC_DEFINE(mode,4)
MC_DEFINE(basis,3)
MC_DEFINE(sep_acct_basis,3)
MC_DEFINE(rate_period,2)
MC_DEFINE(run_basis,7)
MC_DEFINE(ledger_type,7)
MC_DEFINE(uw_basis,5)
MC_DEFINE(table_rating,11)
MC_DEFINE(solve_type,9)
MC_DEFINE(solve_target,2)
MC_DEFINE(solve_tgt_at,4)
MC_DEFINE(solve_from,4)
MC_DEFINE(solve_to,4)
MC_DEFINE(pmt_strategy,9)
MC_DEFINE(sa_strategy,9)
MC_DEFINE(loan_strategy,4)
MC_DEFINE(wd_strategy,4)
MC_DEFINE(interest_rate_type,3)
MC_DEFINE(loan_rate_type,2)
MC_DEFINE(fund_input_method,3)
MC_DEFINE(run_order,2)
MC_DEFINE(survival_limit,4)
MC_DEFINE(term_adj_method,3)
MC_DEFINE(state,53)
MC_DEFINE(country,239)
MC_DEFINE(defn_life_ins,3)
MC_DEFINE(mec_avoid_method,3)
MC_DEFINE(defn_material_change,5)
MC_DEFINE(spread_method,2)
MC_DEFINE(coi_rate_method,2)
MC_DEFINE(anticipated_deduction,4)
MC_DEFINE(part_mort_table,1)
MC_DEFINE(premium_table,1)
MC_DEFINE(from_point,4)
MC_DEFINE(to_point,4)
MC_DEFINE(report_column,35)

#undef MC_DEFINE

