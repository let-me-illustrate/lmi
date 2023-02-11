// Enumerative types used sparsim across subsystems.
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

#ifndef oecumenic_enumerations_hpp
#define oecumenic_enumerations_hpp

#include "config.hpp"

/// Genesis 5:27. Oecumenic, though not an enum.

constexpr int methuselah {969};

enum oenum_7702_7pp_whence
    {oe_7702_7pp_first_principles
    ,oe_7702_7pp_from_table
    };

enum oenum_7702_corr_whence
    {oe_7702_corr_first_principles
    ,oe_7702_corr_from_table
    };

enum oenum_7702_nsp_whence
    {oe_7702_nsp_first_principles
    ,oe_7702_nsp_from_table
    ,oe_7702_nsp_reciprocal_cvat_corridor
    };

enum oenum_7702_q_whence
    {oe_7702_q_builtin
    ,oe_7702_q_external_table
    };

enum oenum_7702_term
    {oe_7702_term_is_ignored
    ,oe_7702_term_is_db
    ,oe_7702_term_is_qab
    };

enum oenum_alb_or_anb
    {oe_age_last_birthday
    ,oe_age_nearest_birthday_ties_younger
    ,oe_age_nearest_birthday_ties_older
    };

enum oenum_allocation_method
    {oe_input_allocation
    ,oe_override_allocation
    };

enum oenum_asset_charge_type
    {oe_asset_charge_spread
    ,oe_asset_charge_load
    };

enum oenum_autopisty
    {oe_orthodox
    ,oe_heterodox
    };

enum oenum_elasticity
    {oe_inelastic
    ,oe_elastic
    };

enum oenum_format_style
    {oe_format_normal
    ,oe_cents_as_dollars
    ,oe_format_percentage
    };

enum oenum_glp_or_gsp
    {oe_glp
    ,oe_gsp
    };

enum oenum_guar_coi_whence
    {oe_guar_coi_external_table
    ,oe_guar_coi_builtin
    };

enum oenum_h_align
    {oe_left
    ,oe_center
    ,oe_right
    };

enum oenum_increment_account_preference
    {oe_prefer_general_account
    ,oe_prefer_separate_account
    };

enum oenum_increment_method
    {oe_proportional
    ,oe_progressive
    };

enum oenum_modal_prem_type
    {oe_monthly_deduction
    ,oe_modal_nonmec
    ,oe_modal_table
    };

/// Return code for MVC-docview editing.

enum oenum_mvc_dv_rc
    {oe_mvc_dv_cancelled
    ,oe_mvc_dv_unchanged
    ,oe_mvc_dv_changed
    };

/// A regulatory association defines various types of life insurance here:
///   https://www.insurancecompact.org/rulemaking_records/adopted_uniform_standards.pdf
/// and "modified single premium" in particular here:
///   https://www.insurancecompact.org/rulemaking_records/070930_mod_sp_adjustable.pdf
/// but lists no non-"modified" category for single-premium UL.
///
/// At least one state apparently concludes that all single-premium UL
/// must be captioned as "Modified". Use
///   oe_plain_single_premium
/// for states that allow captioning UL as "Single Premium" without
/// any modifier;
///   oe_modified_single_premium
/// for states that require a "Modified Single Premium" caption;
///   oe_limited_flexible_premium
/// in situations that require "Limited Flexible Premium" in lieu of
/// "Modified Single Premium" (which means the same thing even though
/// it might seem not to); and
///   oe_flexible_premium
/// otherwise.

enum oenum_premium_flexibility
    {oe_flexible_premium
    ,oe_plain_single_premium
    ,oe_modified_single_premium
    ,oe_limited_flexible_premium
    };

enum oenum_min_single_prem_type
    {oe_no_min_single_premium
    ,oe_min_single_premium_corr_mult
    };

/// Used only for backward compatibility with old versions that didn't
/// distinguish state of jurisdiction from premium-tax state.

enum oenum_premium_tax_state
    {oe_ee_state
    ,oe_er_state
    };

#if 0
/// Not yet used.
enum oenum_premium_tax_tier_period
    {oe_premium_tax_tiering_by_policy_year
    ,oe_premium_tax_tiering_by_calendar_year
    };
#endif // 0

enum oenum_render_or_only_measure
    {oe_render
    ,oe_only_measure
    };

enum oenum_smoking_or_tobacco
    {oe_smoker_nonsmoker
    ,oe_tobacco_nontobacco
    };

enum oenum_visibility
    {oe_shown
    ,oe_hidden
    };

enum oenum_waiver_charge_method
    {oe_waiver_times_deductions
    ,oe_waiver_times_specamt
    };

#endif // oecumenic_enumerations_hpp
