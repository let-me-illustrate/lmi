// Instantiation of specific mc_enum types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: mc_enum_types.cpp,v 1.27 2009-03-04 12:34:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.tpp"            // Template class implementation.
#include "mc_enum_type_enums.hpp" // Plain enums.

// Here write illustrative examples and anything that doesn't follow
// the macro paradigm, such as enumerators with nonsuccessive values.

// Don't do this:
//   #include "mc_enum_types.hpp"
// because that header specifies array bounds explicitly. Omitting
// that causes the bounds to be 'calculated' here [8.3.4/3], and the
// calculated bounds are checked at compile time.

// These lines would engender diagnostics
//   extern enum_option const option_enums[] = {mce_option_A, mce_option_B};
//   extern char const*const option_strings[] = {"A", "B", "C", "X"};
// at compile time when the template is explicitly instantiated.

extern enum_option const option_enums[] =
    {mce_option_A
    ,mce_option_B
    ,mce_option_C
    };
extern char const*const option_strings[] =
    {"A"
    ,"B"
    ,"C"
    };
template class mc_enum<enum_option, 3, option_enums, option_strings>;

extern mcenum_emission const emission_enums[] =
    {mce_emit_nothing
    ,mce_emit_composite_only
    ,mce_emit_quietly
    ,mce_emit_timings
    ,mce_emit_pdf_file
    ,mce_emit_pdf_to_printer
    ,mce_emit_pdf_to_viewer
    ,mce_emit_test_data
    ,mce_emit_spreadsheet
    ,mce_emit_text_stream
    ,mce_emit_custom_0
    };
extern char const*const emission_strings[] =
    {"emit_nothing"
    ,"emit_composite_only"
    ,"emit_quietly"
    ,"emit_timings"
    ,"emit_pdf_file"
    ,"emit_pdf_to_printer"
    ,"emit_pdf_to_viewer"
    ,"emit_test_data"
    ,"emit_spreadsheet"
    ,"emit_text_stream"
    ,"emit_custom_0"
    };
template class mc_enum<mcenum_emission, 11, emission_enums, emission_strings>;

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
MC_DEFINE(gen_basis,3)
MC_DEFINE(sep_basis,3)
MC_DEFINE(rate_period,2)
MC_DEFINE(run_basis,7)
MC_DEFINE(ledger_type,8)
MC_DEFINE(uw_basis,5)
MC_DEFINE(table_rating,11)
MC_DEFINE(solve_type,6)
MC_DEFINE(solve_target,4)
MC_DEFINE(solve_tgt_at,4)
MC_DEFINE(solve_from,4)
MC_DEFINE(solve_to,4)
MC_DEFINE(pmt_strategy,8)
MC_DEFINE(sa_strategy,8)
MC_DEFINE(interest_rate_type,3)
MC_DEFINE(loan_rate_type,2)
MC_DEFINE(fund_input_method,3)
MC_DEFINE(run_order,2)
MC_DEFINE(survival_limit,4)
MC_DEFINE(term_adj_method,3)
MC_DEFINE(state,53)
MC_DEFINE(country,239)
MC_DEFINE(defn_life_ins,3)
MC_DEFINE(mec_avoid_method,2)
MC_DEFINE(defn_material_change,5)
MC_DEFINE(spread_method,2)
MC_DEFINE(coi_rate_method,2)
MC_DEFINE(anticipated_deduction,4)
MC_DEFINE(part_mort_table,1)
MC_DEFINE(premium_table,1)
MC_DEFINE(from_point,4)
MC_DEFINE(to_point,4)
MC_DEFINE(report_column,47)

#undef MC_DEFINE

