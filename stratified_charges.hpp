// Rates that depend on the amount they're muliplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef stratified_charges_hpp
#define stratified_charges_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

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

class LMI_SO stratified_entity
    :virtual private obstruct_slicing<stratified_entity>
{
    friend class stratified_charges;
    friend class TierView;

  public:
    stratified_entity();
    stratified_entity
        (std::vector<double> const& limits
        ,std::vector<double> const& values
        );
    ~stratified_entity();

    void read (std::istream&);
    void write(std::ostream&) const;

  private:
    void assert_validity() const;

    std::vector<double> const& limits() const;
    std::vector<double> const& values() const;

    std::vector<double> limits_;
    std::vector<double> values_;
};

// Implicitly-declared special member functions do the right thing.

class LMI_SO stratified_charges
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
    // - tiered_guar_sepacct_load: seems to be incorrectly implemented.

    double stratified_sepacct_load
        (mcenum_gen_basis basis
        ,double           assets
        ,double           premium
        ,double           special_limit
        );

    // TODO ?? In the public interface, consider replacing these:
    //   tiered_current_m_and_e()
    //   tiered_guaranteed_m_and_e()
    // with a single tiered_m_and_e(mcenum_gen_basis basis, double assets).

    double tiered_current_m_and_e           (double assets) const;
    double tiered_guaranteed_m_and_e        (double assets) const;
    double tiered_asset_based_compensation  (double assets) const;
    double tiered_investment_management_fee (double assets) const;

    double minimum_tiered_spread_for_7702() const;

    // Tiered premium tax in certain states.
    double tiered_premium_tax
        (mcenum_state state
        ,double       payment
        ,double       aggregate_payment
        ) const;
    bool premium_tax_is_tiered(mcenum_state) const;

    // Lowest rate for conservatism in complicated formulas that
    // don't yet reflect tiering.
    double minimum_tiered_premium_tax_rate(mcenum_state) const;

    static void write_stratified_files();
    static void write_proprietary_stratified_files();

  private:
    stratified_charges(); // Private, but implemented for friends' use.

    stratified_entity&       raw_entity(e_stratified);
    stratified_entity const& raw_entity(e_stratified) const;

    void initialize_dictionary();

    void read (std::string const& filename);
    void write(std::string const& filename) const;

    double banded_curr_sepacct_load
        (double assets
        ,double premium
        ,double special_limit
        ) const;

    double banded_guar_sepacct_load
        (double assets
        ,double premium
        ,double special_limit
        ) const;

    double tiered_curr_sepacct_load(double assets, double premium) const;
    double tiered_guar_sepacct_load(double assets, double premium) const;

    std::map<e_stratified, stratified_entity> dictionary;
};

bool LMI_SO is_highest_representable_double(double);

#endif // stratified_charges_hpp

