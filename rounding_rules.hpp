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

#ifndef rounding_rules_hpp
#define rounding_rules_hpp

#include "config.hpp"

#include "round_to.hpp"
#include "so_attributes.hpp"

// Design principle: round transactional changes to account value,
// rather than rounding account value itself at each transaction.
// This is the way admin systems are most likely to work, and
// gratuitous differences between systems should be avoided.
// Rounding is a property of the transaction, and not of the
// variable it ultimately affects.

class LMI_SO rounding_rules
{
    friend class StreamableRoundingRules;
    friend class RoundingDocument;
    friend class RoundingView;

  public:
    rounding_rules();

    round_to<double> const& round_specamt           () const;
    round_to<double> const& round_death_benefit     () const;
    round_to<double> const& round_naar              () const;
    round_to<double> const& round_coi_rate          () const;
    round_to<double> const& round_coi_charge        () const;
    round_to<double> const& round_gross_premium     () const;
    round_to<double> const& round_net_premium       () const;
    round_to<double> const& round_interest_rate     () const;
    round_to<double> const& round_interest_credit   () const;
    round_to<double> const& round_withdrawal        () const;
    round_to<double> const& round_loan              () const;
    round_to<double> const& round_corridor_factor   () const;
    round_to<double> const& round_surrender_charge  () const;
    round_to<double> const& round_irr               () const;
    round_to<double> const& round_min_specamt       () const;
    round_to<double> const& round_max_specamt       () const;
    round_to<double> const& round_min_premium       () const;
    round_to<double> const& round_max_premium       () const;
    round_to<double> const& round_interest_rate_7702() const;

  private:
    round_to<double> round_specamt_;
    round_to<double> round_death_benefit_;
    round_to<double> round_naar_;
    round_to<double> round_coi_rate_;
    round_to<double> round_coi_charge_;
    round_to<double> round_gross_premium_;
    round_to<double> round_net_premium_;
    round_to<double> round_interest_rate_;
    round_to<double> round_interest_credit_;
    round_to<double> round_withdrawal_;
    round_to<double> round_loan_;
    round_to<double> round_corridor_factor_;
    round_to<double> round_surrender_charge_;

    // IRR must be rounded down in order to avoid overstating it, but
    // its precision is an arbitrary parameter.

    round_to<double> round_irr_;

    // Premium and specamt must be rounded in the right direction for
    // 7702 and 7702A calculations. Here,
    //   min- means we want the minimum amount that satisfies some
    //     condition, so we have to round it up to be sure; and
    //   max- means the opposite.
    // The 7702 and 7702A interest rate must be rounded up, if at all.

    round_to<double> round_min_specamt_;
    round_to<double> round_max_specamt_;
    round_to<double> round_min_premium_;
    round_to<double> round_max_premium_;
    round_to<double> round_interest_rate_7702_;
};

inline round_to<double> const& rounding_rules::round_specamt() const
{
    return round_specamt_;
}

inline round_to<double> const& rounding_rules::round_death_benefit() const
{
    return round_death_benefit_;
}

inline round_to<double> const& rounding_rules::round_naar() const
{
    return round_naar_;
}

inline round_to<double> const& rounding_rules::round_coi_rate() const
{
    return round_coi_rate_;
}

inline round_to<double> const& rounding_rules::round_coi_charge() const
{
    return round_coi_charge_;
}

inline round_to<double> const& rounding_rules::round_gross_premium() const
{
    return round_gross_premium_;
}

inline round_to<double> const& rounding_rules::round_net_premium() const
{
    return round_net_premium_;
}

inline round_to<double> const& rounding_rules::round_interest_rate() const
{
    return round_interest_rate_;
}

inline round_to<double> const& rounding_rules::round_interest_credit() const
{
    return round_interest_credit_;
}

inline round_to<double> const& rounding_rules::round_withdrawal() const
{
    return round_withdrawal_;
}

inline round_to<double> const& rounding_rules::round_loan() const
{
    return round_loan_;
}

inline round_to<double> const& rounding_rules::round_corridor_factor() const
{
    return round_corridor_factor_;
}

inline round_to<double> const& rounding_rules::round_surrender_charge() const
{
    return round_surrender_charge_;
}

inline round_to<double> const& rounding_rules::round_irr() const
{
    return round_irr_;
}

inline round_to<double> const& rounding_rules::round_min_specamt() const
{
    return round_min_specamt_;
}

inline round_to<double> const& rounding_rules::round_max_specamt() const
{
    return round_max_specamt_;
}

inline round_to<double> const& rounding_rules::round_min_premium() const
{
    return round_min_premium_;
}

inline round_to<double> const& rounding_rules::round_max_premium() const
{
    return round_max_premium_;
}

inline round_to<double> const& rounding_rules::round_interest_rate_7702() const
{
    return round_interest_rate_7702_;
}

#endif // rounding_rules_hpp

