// Declaration of specific mc_enum types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef mc_enum_types_hpp
#define mc_enum_types_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"
#include "mc_enum_type_enums.hpp"

// Here write illustrative examples and anything that doesn't follow
// the macro paradigm, such as enumerators with nonsuccessive values,
// for parallelism with the corresponding TU.

typedef mc_enum<enum_tragic    > e_tragic          ;
typedef mc_enum<mcenum_emission> e_emission        ;
typedef mc_enum<rounding_style > mce_rounding_style;

typedef mc_enum<mcenum_yes_or_no            > mce_yes_or_no            ;
typedef mc_enum<mcenum_gender               > mce_gender               ;
typedef mc_enum<mcenum_smoking              > mce_smoking              ;
typedef mc_enum<mcenum_class                > mce_class                ;
typedef mc_enum<mcenum_dbopt                > mce_dbopt                ;
typedef mc_enum<mcenum_dbopt_7702           > mce_dbopt_7702           ;
typedef mc_enum<mcenum_cso_era              > mce_cso_era              ;
typedef mc_enum<mcenum_mode                 > mce_mode                 ;
typedef mc_enum<mcenum_gen_basis            > mce_gen_basis            ;
typedef mc_enum<mcenum_sep_basis            > mce_sep_basis            ;
typedef mc_enum<mcenum_rate_period          > mce_rate_period          ;
typedef mc_enum<mcenum_run_basis            > mce_run_basis            ;
typedef mc_enum<mcenum_ledger_type          > mce_ledger_type          ;
typedef mc_enum<mcenum_uw_basis             > mce_uw_basis             ;
typedef mc_enum<mcenum_table_rating         > mce_table_rating         ;
typedef mc_enum<mcenum_solve_type           > mce_solve_type           ;
typedef mc_enum<mcenum_solve_target         > mce_solve_target         ;
typedef mc_enum<mcenum_pmt_strategy         > mce_pmt_strategy         ;
typedef mc_enum<mcenum_sa_strategy          > mce_sa_strategy          ;
typedef mc_enum<mcenum_gen_acct_rate_type   > mce_gen_acct_rate_type   ;
typedef mc_enum<mcenum_sep_acct_rate_type   > mce_sep_acct_rate_type   ;
typedef mc_enum<mcenum_loan_rate_type       > mce_loan_rate_type       ;
typedef mc_enum<mcenum_fund_input_method    > mce_fund_input_method    ;
typedef mc_enum<mcenum_run_order            > mce_run_order            ;
typedef mc_enum<mcenum_survival_limit       > mce_survival_limit       ;
typedef mc_enum<mcenum_term_adj_method      > mce_term_adj_method      ;
typedef mc_enum<mcenum_state                > mce_state                ;
typedef mc_enum<mcenum_country              > mce_country              ;
typedef mc_enum<mcenum_defn_life_ins        > mce_defn_life_ins        ;
typedef mc_enum<mcenum_mec_avoid_method     > mce_mec_avoid_method     ;
typedef mc_enum<mcenum_defn_material_change > mce_defn_material_change ;
typedef mc_enum<mcenum_spread_method        > mce_spread_method        ;
typedef mc_enum<mcenum_coi_rate_method      > mce_coi_rate_method      ;
typedef mc_enum<mcenum_anticipated_deduction> mce_anticipated_deduction;
typedef mc_enum<mcenum_part_mort_table      > mce_part_mort_table      ;
typedef mc_enum<mcenum_premium_table        > mce_premium_table        ;
typedef mc_enum<mcenum_from_point           > mce_from_point           ;
typedef mc_enum<mcenum_to_point             > mce_to_point             ;
typedef mc_enum<mcenum_report_column        > mce_report_column        ;
typedef mc_enum<mcenum_suppl_illus_type     > mce_suppl_illus_type     ;

#endif // mc_enum_types_hpp
