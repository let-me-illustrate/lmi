// Enumerative types used sparsim across subsystems.
//
// Copyright (C) 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: oecumenic_enumerations.hpp,v 1.1 2008-07-03 14:38:38 chicares Exp $

#ifndef oecumenic_enumerations_hpp
#define oecumenic_enumerations_hpp

#include "config.hpp"

enum e_allocation_method
    {e_input_allocation
    ,e_override_allocation
    };

enum e_increment_account_preference
    {e_prefer_general_account
    ,e_prefer_separate_account
    };

enum e_increment_method
    {e_proportional
    ,e_progressive
    };

enum e_modal_prem_type
    {e_monthly_deduction
    ,e_modal_nonmec
    ,e_modal_table
    };

enum e_preferred_class_name
    {e_called_preferred
    ,e_called_select
    };

/// Deprecated: state of jurisdiction should be an input field.
enum e_premium_tax_state
    {e_ee_state
    ,e_er_state
    };

#if 0
// Not yet used.
enum e_premium_tax_tier_period
    {e_premium_tax_tiering_by_policy_year
    ,e_premium_tax_tiering_by_calendar_year
    };
#endif // 0

enum e_smoking_or_tobacco
    {e_smoker_nonsmoker
    ,e_tobacco_nontobacco
    };

enum e_waiver_charge_method
    {e_waiver_times_deductions
    ,e_waiver_times_naar
    };

#endif // oecumenic_enumerations_hpp

