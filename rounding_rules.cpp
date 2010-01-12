// Rounding rules.
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

#include "rounding_rules.hpp"

//============================================================================
rounding_rules::rounding_rules()
    :round_specamt_           (0, r_upward    )
    ,round_death_benefit_     (2, r_to_nearest)
    ,round_naar_              (2, r_to_nearest)
    ,round_coi_rate_          (8, r_downward  )
    ,round_coi_charge_        (2, r_to_nearest)
    ,round_gross_premium_     (2, r_to_nearest)
    ,round_net_premium_       (2, r_to_nearest)
    ,round_interest_rate_     (0, r_not_at_all)
    ,round_interest_credit_   (2, r_to_nearest)
    ,round_withdrawal_        (2, r_to_nearest)
    ,round_loan_              (2, r_to_nearest)
    ,round_corridor_factor_   (2, r_to_nearest)
    ,round_surrender_charge_  (2, r_to_nearest)
    ,round_irr_               (5, r_downward  )
    ,round_min_specamt_       (0, r_upward    )
    ,round_max_specamt_       (0, r_downward  )
    ,round_min_premium_       (2, r_upward    )
    ,round_max_premium_       (2, r_downward  )
    ,round_interest_rate_7702_(0, r_not_at_all)
{
}

