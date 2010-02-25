// Auxiliary functions for specific mc_enum types.
//
// Copyright (C) 2008, 2009, 2010 Gregory W. Chicares.
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

#include "mc_enum_types_aux.hpp"

#include "alert.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(mc_n_gen_bases    == static_cast<std::size_t>(mce_gen_basis  ::Cardinality));
BOOST_STATIC_ASSERT(mc_n_sep_bases    == static_cast<std::size_t>(mce_sep_basis  ::Cardinality));
BOOST_STATIC_ASSERT(mc_n_rate_periods == static_cast<std::size_t>(mce_rate_period::Cardinality));

std::vector<std::string> const& LMI_SO all_strings_gender   () {return mce_gender  ::all_strings();}
std::vector<std::string> const& LMI_SO all_strings_class    () {return mce_class   ::all_strings();}
std::vector<std::string> const& LMI_SO all_strings_smoking  () {return mce_smoking ::all_strings();}
std::vector<std::string> const& LMI_SO all_strings_uw_basis () {return mce_uw_basis::all_strings();}
std::vector<std::string> const& LMI_SO all_strings_state    () {return mce_state   ::all_strings();}

/// GPT recognizes death benefit options A and B only. A contract
/// might have a death benefit option other than that usual pair,
/// but for 7702 (and 7702A) purposes it's treated as either A or B.

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
        default: fatal_error() << "No " << actual_dbopt << " case." << LMI_FLUSH;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Illustration reg requires midpoint basis and guaranteed premium.

bool is_subject_to_ill_reg(mcenum_ledger_type z)
{
    return mce_ill_reg == z;
}

/// NASD illustrations may have three interest bases (e.g. 0, 6, 12%)
/// instead of only two (e.g. 0, 12%). Three-rate NASD illustrations
/// are not fully supported at the moment, so this function simply
/// returns 'false'.

bool is_three_rate_nasd(mcenum_ledger_type)
{
    return false;
}

std::string mc_str(mcenum_dbopt z)
{
    return mce_dbopt(z).str();
}

std::string mc_str(mcenum_run_basis z)
{
    return mce_run_basis(z).str();
}
  #include "alert.hpp"
std::string mc_str(mcenum_state z)
{
//    return mce_state(z).str();

try{
    return mce_state(z).str();
} catch(...) {fatal_error() << z << " !" << LMI_FLUSH; throw 0;}
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
    else {fatal_error() << "Run basis " << r << " unknown." << LMI_FLUSH;}
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
///   is used only on NASD illustrations that show three rates instead
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
///   {CF, GF,     CZ, GZ        } normal NASD
///   {CF, GF,     CZ, GZ, CH, GH} three-rate NASD

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
    else {fatal_error() << "Cannot set run basis." << LMI_FLUSH;}
}

