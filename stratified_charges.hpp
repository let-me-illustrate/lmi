// Rates that depend on the amount they're muliplied by.
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

// $Id: stratified_charges.hpp,v 1.3 2005-08-22 15:35:53 chicares Exp $

#ifndef stratified_charges_hpp
#define stratified_charges_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "obstruct_slicing.hpp"
#include "xenumtypes.hpp"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

enum e_stratified
    {e_stratified_first

    ,e_topic_premium_banded
    ,e_curr_sepacct_load_banded_by_premium
    ,e_guar_sepacct_load_banded_by_premium

    ,e_topic_asset_tiered
    ,e_curr_m_and_e_tiered_by_assets
    ,e_guar_m_and_e_tiered_by_assets
    ,e_asset_based_comp_tiered_by_assets
    ,e_investment_mgmt_fee_tiered_by_assets
    ,e_curr_sepacct_load_tiered_by_assets
    ,e_guar_sepacct_load_tiered_by_assets

    ,e_topic_tiered_premium_tax
    ,e_tiered_ak_premium_tax
    ,e_tiered_de_premium_tax
    ,e_tiered_sd_premium_tax

    ,e_stratified_last
    };

// Implicitly-declared special member functions do the right thing.

class LMI_EXPIMP stratified_entity
    :virtual private obstruct_slicing<stratified_entity>
{
    friend class TierView;
    friend class stratified_charges;

  public:
    stratified_entity();
    stratified_entity
        (std::vector<double> const& limits
        ,std::vector<double> const& values
        );
    ~stratified_entity();

    void read(std::istream& is);
    void write(std::ostream& os) const;

  private:
    std::vector<double> const& limits() const;
    std::vector<double> const& values() const;

    std::vector<double> limits_;
    std::vector<double> values_;
};

// Implicitly-declared special member functions do the right thing.

class LMI_EXPIMP stratified_charges
    :virtual private obstruct_slicing<stratified_charges>
{
    friend class TierDocument;
    friend class TierView;

  public:
    stratified_charges(std::string const& filename);
    ~stratified_charges();

    // TODO ?? These things are not implemented implemented correctly:
    //
    // - tiered_asset_based_compensation, tiered_investment_management_fee:
    // setting these to any nonzero value produces a runtime error in
    // the place where they ought to be used.
    //
    // - tiered_guaranteed_separate_account_load: seems to be
    // incorrectly implemented.

    double banded_current_separate_account_load    (double premium) const;
    double banded_guaranteed_separate_account_load (double premium) const;

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

    static void write_stratified_files();
    static void write_proprietary_stratified_files();

  private:
    stratified_charges(); // Private, but implemented: needed by std::map.

    stratified_entity&       raw_entity(e_stratified);
    stratified_entity const& raw_entity(e_stratified) const;

    void initialize_dictionary();

    void read (std::string const& filename);
    void write(std::string const& filename) const;

    std::map<e_stratified, stratified_entity> dictionary;
};

#endif  // stratified_charges_hpp

