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

#include "pchfile.hpp"

#include "mc_enum_types_aux.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"

#include <stdexcept>

namespace
{
/// Suppress enumerators for emission options not fully implemented.

void constrain_values(e_emission& z)
{
    z.allow(z.ordinal("emit_pdf_to_printer"), false);
    z.allow(z.ordinal("emit_pdf_to_viewer" ), false);
    z.allow(z.ordinal("emit_group_quote"   ), false);
}

/// Validate mc_n_gen_bases, mc_n_sep_bases, and mc_n_rate_periods.
///
/// Each of these very useful constants must equal the cardinality of
/// its corresponding mc_enum type. This cannot gracefully be affirmed
/// by a static assertion, so it's asserted at run time here.

bool validate_mc_n_values()
{
    LMI_ASSERT(mc_n_gen_bases    == all_strings<mcenum_gen_basis  >().size());
    LMI_ASSERT(mc_n_sep_bases    == all_strings<mcenum_sep_basis  >().size());
    LMI_ASSERT(mc_n_rate_periods == all_strings<mcenum_rate_period>().size());
    return true;
}

/// See:
///   http://groups.google.com/groups?selm=1006352851.15484.0.nnrp-08.3e31d362@news.demon.co.uk
/// and Kanze's reply:
///   http://groups.google.com/groups?selm=d6651fb6.0111221034.42e78b95@posting.google.com

bool volatile ensure_setup = validate_mc_n_values();
} // Unnamed namespace.

std::vector<std::string> const& all_strings_gender   () {return all_strings<mcenum_gender  >();}
std::vector<std::string> const& all_strings_class    () {return all_strings<mcenum_class   >();}
std::vector<std::string> const& all_strings_smoking  () {return all_strings<mcenum_smoking >();}
std::vector<std::string> const& all_strings_uw_basis () {return all_strings<mcenum_uw_basis>();}
std::vector<std::string> const& all_strings_state    () {return all_strings<mcenum_state   >();}

std::vector<std::string> allowed_strings_emission()
{
    e_emission emission;
    constrain_values(emission);
    std::vector<std::string> z;
    for(int j = 0; j < emission.cardinality(); ++j)
        {
        if(emission.is_allowed(j))
            {
            z.push_back(emission.str(j));
            }
        }
    return z;
}

/// GPT recognizes death benefit options A and B only. A contract
/// might have a death benefit option other than that usual pair,
/// but for 7702 (and 7702A) purposes it's treated as either A or B.
/// For ROP, it might be either. For MDB, it can only be A.

mcenum_dbopt_7702 effective_dbopt_7702
    (mcenum_dbopt      actual_dbopt
    ,mcenum_dbopt_7702 rop_equivalent
    )
{
    switch(actual_dbopt)
        {
        case mce_option1: return mce_option1_for_7702;
        case mce_option2: return mce_option2_for_7702;
        case mce_rop:     return rop_equivalent      ;
        case mce_mdb:     return mce_option1_for_7702;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Illustration reg requires midpoint basis and guaranteed premium.

bool is_subject_to_ill_reg(mcenum_ledger_type z)
{
    return mce_ill_reg == z;
}

/// FINRA illustrations may have three interest bases (e.g. 0, 6, 12%)
/// instead of only two (e.g. 0, 12%). Three-rate FINRA illustrations
/// are not fully supported at the moment, so this function simply
/// returns 'false'.

bool is_three_rate_finra(mcenum_ledger_type)
{
    return false;
}

template<typename T>
std::string mc_str(T t)
{
    return mc_enum<T>(t).str();
}

template std::string mc_str(mcenum_class               );
template std::string mc_str(mcenum_country             );
template std::string mc_str(mcenum_cso_era             );
template std::string mc_str(mcenum_dbopt               );
template std::string mc_str(mcenum_defn_life_ins       );
template std::string mc_str(mcenum_defn_material_change);
template std::string mc_str(mcenum_gender              );
template std::string mc_str(mcenum_mec_avoid_method    );
template std::string mc_str(mcenum_mode                );
template std::string mc_str(mcenum_report_column       );
template std::string mc_str(mcenum_run_basis           );
template std::string mc_str(mcenum_smoking             );
template std::string mc_str(mcenum_solve_target        );
template std::string mc_str(mcenum_solve_type          );
template std::string mc_str(mcenum_state               );
template std::string mc_str(mcenum_table_rating        );
template std::string mc_str(mcenum_uw_basis            );

mcenum_emission mc_emission_from_string(std::string const& s)
{
    e_emission z(s);
    constrain_values(z);
    if(!z.is_allowed(z.ordinal(s)))
        {
        throw std::runtime_error(s);
        }
    return z.value();
}

mcenum_state mc_state_from_string(std::string const& s)
{
    return mce_state(s).value();
}

/// Inverse of set_run_basis_from_cloven_bases(), q.v.

void set_cloven_bases_from_run_basis
    (mcenum_run_basis const r
    ,mcenum_gen_basis&      g
    ,mcenum_sep_basis&      s
    )
{
         if(r == mce_run_gen_curr_sep_full) {g = mce_gen_curr; s = mce_sep_full;}
    else if(r == mce_run_gen_guar_sep_full) {g = mce_gen_guar; s = mce_sep_full;}
    else if(r == mce_run_gen_mdpt_sep_full) {g = mce_gen_mdpt; s = mce_sep_full;}
    else if(r == mce_run_gen_curr_sep_zero) {g = mce_gen_curr; s = mce_sep_zero;}
    else if(r == mce_run_gen_guar_sep_zero) {g = mce_gen_guar; s = mce_sep_zero;}
    else if(r == mce_run_gen_curr_sep_half) {g = mce_gen_curr; s = mce_sep_half;}
    else if(r == mce_run_gen_guar_sep_half) {g = mce_gen_guar; s = mce_sep_half;}
    else {alarum() << "Run basis " << r << " unknown." << LMI_FLUSH;}
}

/// Illustrations are run on two primary bases:
///
/// - a "general" basis that governs expense charges and general-
///   account rates:
///     {current, guaranteed, midpoint} (here, {C, G, M} for short)
///   where "midpoint" is used only when the illustration reg governs.
///
/// - a "separate" basis that governs separate-account rates:
///     {full, zero, half}  (here, {F, Z, H} for short)
///   where "full" is the undiminished hypothetical rate, and "half"
///   is used only on FINRA illustrations that show three rates instead
///   of two.
///
/// To avoid nested loops, it is convenient to represent combinations
/// of those primary bases as a distinct type: a "run" basis for
/// iteration.
///
/// Only these combinations ever arise:
///   {CF, GF, MF, CZ, GZ, CH, GH} actually-used bases
/// of which only these subsets are used:
///   {CF, GF, MF                } illustration reg
///   {CF, GF,     CZ, GZ        } normal FINRA
///   {CF, GF,     CZ, GZ, CH, GH} three-rate FINRA

void set_run_basis_from_cloven_bases
    (mcenum_run_basis&      r
    ,mcenum_gen_basis const g
    ,mcenum_sep_basis const s
    )
{
    if     (g == mce_gen_curr && s == mce_sep_full) r = mce_run_gen_curr_sep_full;
    else if(g == mce_gen_guar && s == mce_sep_full) r = mce_run_gen_guar_sep_full;
    else if(g == mce_gen_mdpt && s == mce_sep_full) r = mce_run_gen_mdpt_sep_full;
    else if(g == mce_gen_curr && s == mce_sep_zero) r = mce_run_gen_curr_sep_zero;
    else if(g == mce_gen_guar && s == mce_sep_zero) r = mce_run_gen_guar_sep_zero;
    else if(g == mce_gen_curr && s == mce_sep_half) r = mce_run_gen_curr_sep_half;
    else if(g == mce_gen_guar && s == mce_sep_half) r = mce_run_gen_guar_sep_half;
    else {alarum() << "Cannot set run basis." << LMI_FLUSH;}
}
