// Tiered data, e.g. compensation of x% on the first $z and y% thereafter; or
// the simpler x% or y% of total when total is up through or beyond $z.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: tiered_charges.hpp,v 1.1 2005-02-12 12:59:31 chicares Exp $

#ifndef tierdata_hpp
#define tierdata_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "xenumtypes.hpp"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

class LMI_EXPIMP tiered_item_rep
{
    friend class TierView;
    friend class tiered_charges;

  public:
    tiered_item_rep();
    tiered_item_rep
        (std::vector<double> const& bands
        ,std::vector<double> const& data
        );

    std::vector<double> const& bands() const;
    std::vector<double> const& data () const;

    void read(std::istream& is);
    void write(std::ostream& os) const;

  private:
    std::vector<double> bands_;
    std::vector<double> data_;
};

inline std::vector<double> const& tiered_item_rep::bands() const
{
    return bands_;
}

inline std::vector<double> const& tiered_item_rep::data () const
{
    return data_ ;
}

class LMI_EXPIMP tiered_charges
{
    friend class TierDocument;
    friend class TierView;

  public:
    enum tiered_enumerator
        {e_tier_first
        ,e_tier_topic_experience_rating
        ,e_tier_stabilization_reserve
        ,e_tier_coi_retention
        ,e_tier_topic_asset_based
        ,e_tier_current_m_and_e
        ,e_tier_guaranteed_m_and_e
        ,e_tier_asset_based_compensation
        ,e_tier_investment_management_fee
        ,e_tier_current_separate_account_load
        ,e_tier_guaranteed_separate_account_load
        ,e_tier_topic_premium_tax
        ,e_tier_ak_premium_tax
        ,e_tier_de_premium_tax
        ,e_tier_sd_premium_tax
        ,e_tier_last
        };

    tiered_charges(std::string const& filename);
    virtual ~tiered_charges();

    tiered_item_rep const& tiered_item(tiered_enumerator) const;

    // Function names generally use prefix 'tiered_' only if needed to
    // distinguish them from a parallel non-tiered name.

    double stabilization_reserve (double number_of_lives_inforce ) const;
    double coi_retention         (double number_of_lives_at_issue) const;

    double tiered_current_m_and_e                  (double assets) const;
    double tiered_guaranteed_m_and_e               (double assets) const;
    double tiered_asset_based_compensation         (double assets) const;
    double tiered_investment_management_fee        (double assets) const;
    double tiered_current_separate_account_load    (double assets) const;
    double tiered_guaranteed_separate_account_load (double assets) const;

    double minimum_tiered_spread_for_7702() const;

    // Tiered premium tax in certain states.
    double tiered_premium_tax
        (e_state const& state
        ,double         payment
        ,double         aggregate_payment
        ) const;
    bool premium_tax_is_tiered(e_state const& state) const;

    // Lowest rate for conservatism in complicated formulas that
    // don't yet reflect tiering.
    double minimum_tiered_premium_tax_rate(e_state const& state) const;

    static void write_tier_files();
    static void write_proprietary_tier_files();

  private:
    tiered_charges(); // Private, but implemented: needed by std::map.

    tiered_item_rep& tiered_item(tiered_enumerator);

    void initialize_dictionary();

    void read (std::string const& filename);
    void write(std::string const& filename) const;

    typedef std::map<tiered_enumerator, tiered_item_rep> tier_dictionary_type;
    tier_dictionary_type dictionary;
};

inline tiered_item_rep const& tiered_charges::tiered_item(tiered_enumerator e) const
{
    return (*dictionary.find(e)).second;
}

inline tiered_item_rep& tiered_charges::tiered_item(tiered_enumerator e)
{
    return (*dictionary.find(e)).second;
}

#endif  // tierdata_hpp

