// Plain enumerators for specific mc_enum types.
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

#ifndef mc_enum_type_enums_hpp
#define mc_enum_type_enums_hpp

#include "config.hpp"

// Here write illustrative examples and anything that doesn't follow
// the macro paradigm, such as enumerators with nonsuccessive values.

enum enum_option
    {mce_option_A
    ,mce_option_B
    ,mce_option_C
    };

/// Output target for illustration generation.
///
/// Enumerators are binary powers so that more than one can be
/// specified in a single scalar entity.

enum mcenum_emission
    {mce_emit_nothing        =    0
    ,mce_emit_composite_only =    1
    ,mce_emit_quietly        =    2
    ,mce_emit_timings        =    4
    ,mce_emit_pdf_file       =    8
    ,mce_emit_pdf_to_printer =   16 // GUI only.
    ,mce_emit_pdf_to_viewer  =   32 // GUI only.
    ,mce_emit_test_data      =   64
    ,mce_emit_spreadsheet    =  128
    ,mce_emit_text_stream    =  256
    ,mce_emit_custom_0       =  512
    };

/// Not yet implemented--used only to initialize an unimplemented
/// database entity.

enum mcenum_spread_freq
    {mce_spread_annual     =   1
    ,mce_spread_semiannual =   2
    ,mce_spread_quarterly  =   4
    ,mce_spread_monthly    =  12
    ,mce_spread_daily      = 365
    };

enum mcenum_mode
    {mce_annual     =  1
    ,mce_semiannual =  2
    ,mce_quarterly  =  4
    ,mce_monthly    = 12
    };

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
//MC_DEFINE_ENUMS(mode) // Defined above.
MC_DEFINE_ENUMS(gen_basis)
MC_DEFINE_ENUMS(sep_basis)
MC_DEFINE_ENUMS(rate_period)
MC_DEFINE_ENUMS(run_basis)
MC_DEFINE_ENUMS(ledger_type)
MC_DEFINE_ENUMS(uw_basis)
MC_DEFINE_ENUMS(table_rating)
MC_DEFINE_ENUMS(solve_type)
MC_DEFINE_ENUMS(solve_target)
MC_DEFINE_ENUMS(pmt_strategy)
MC_DEFINE_ENUMS(sa_strategy)
MC_DEFINE_ENUMS(gen_acct_rate_type)
MC_DEFINE_ENUMS(sep_acct_rate_type)
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

#undef MC_DEFINE_ENUMS

#endif // mc_enum_type_enums_hpp

