// Enumerative types used sparsim across subsystems.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#ifndef oecumenic_enumerations_hpp
#define oecumenic_enumerations_hpp

#include "config.hpp"

/// Genesis 5:27.

enum {methuselah = 969};

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

enum oenum_format_style
    {oe_format_normal
    ,oe_format_percentage
    };

enum oenum_glp_or_gsp
    {oe_glp
    ,oe_gsp
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

enum oenum_waiver_charge_method
    {oe_waiver_times_deductions
    ,oe_waiver_times_specamt
    };

#endif // oecumenic_enumerations_hpp

