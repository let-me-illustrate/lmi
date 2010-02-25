// Instantiation of specific mc_enum types.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.hpp"
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
template class mc_enum<enum_option>;

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
template class mc_enum<mcenum_emission>;

#include "mc_enum_types.xpp"

#define MC_DEFINE(TYPE) \
extern mcenum_##TYPE const TYPE##_enums[] = TYPE##_VALUES \
extern char const*const TYPE##_strings[] = TYPE##_NAMES \
template class mc_enum<mcenum_##TYPE>;

MC_DEFINE(yes_or_no)
MC_DEFINE(gender)
MC_DEFINE(smoking)
MC_DEFINE(class)
MC_DEFINE(dbopt)
MC_DEFINE(dbopt_7702)
MC_DEFINE(mode)
MC_DEFINE(gen_basis)
MC_DEFINE(sep_basis)
MC_DEFINE(rate_period)
MC_DEFINE(run_basis)
MC_DEFINE(ledger_type)
MC_DEFINE(uw_basis)
MC_DEFINE(table_rating)
MC_DEFINE(solve_type)
MC_DEFINE(solve_target)
MC_DEFINE(pmt_strategy)
MC_DEFINE(sa_strategy)
MC_DEFINE(gen_acct_rate_type)
MC_DEFINE(sep_acct_rate_type)
MC_DEFINE(loan_rate_type)
MC_DEFINE(fund_input_method)
MC_DEFINE(run_order)
MC_DEFINE(survival_limit)
MC_DEFINE(term_adj_method)
MC_DEFINE(state)
MC_DEFINE(country)
MC_DEFINE(defn_life_ins)
MC_DEFINE(mec_avoid_method)
MC_DEFINE(defn_material_change)
MC_DEFINE(spread_method)
MC_DEFINE(coi_rate_method)
MC_DEFINE(anticipated_deduction)
MC_DEFINE(part_mort_table)
MC_DEFINE(premium_table)
MC_DEFINE(from_point)
MC_DEFINE(to_point)
MC_DEFINE(report_column)

#undef MC_DEFINE

// Explicit instantiation of class mc_enum above does not require a
// definition of mc_enum_key<T>, which does not appear in the class
// definition. However, because mc_enum_key<T> is used in the bodies
// of member functions, it must be defined in this TU--before the
// point of explicit instantiation of the member functions, but not
// necessarily of the class [14.5.1.1/1]. The reason for including
// its definition below rather than above is to force array bounds to
// be 'calculated' [8.3.4/3], so that errors are detected at compile
// time: this is the motivation for keeping class template mc_enum
// and its metadata physically separate.

#include "mc_enum.tpp"
#include "mc_enum_types.hpp"

