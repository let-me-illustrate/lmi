// Rates that depend on the amount they're muliplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#include "any_member.hpp"
#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <string>
#include <vector>

enum e_stratified
    {e_stratified_first

    ,e_topic_premium_banded
    ,e_curr_sepacct_load_banded_by_premium
    ,e_guar_sepacct_load_banded_by_premium

    ,e_topic_asset_banded
    ,e_curr_sepacct_load_banded_by_assets
    ,e_guar_sepacct_load_banded_by_assets

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
        ,std::string const&         gloss = std::string()
        );
    ~stratified_entity();

    bool operator==(stratified_entity const&) const;

    void read (xml::element const& node);
    void write(xml::element&) const;

  private:
    void assert_validity() const;

    std::vector<double> const& limits() const;
    std::vector<double> const& values() const;
    std::string const&         gloss () const;

    std::vector<double> limits_;
    std::vector<double> values_;
    std::string         gloss_;
};

/// Rates that depend upon the amount they're multiplied by.

class LMI_SO stratified_charges
    :virtual private obstruct_slicing  <stratified_charges>
    ,        public  xml_serializable  <stratified_charges>
    ,        public  MemberSymbolTable <stratified_charges>
{
    friend class TierDocument;

  public:
    stratified_charges(std::string const& filename);
    stratified_charges(stratified_charges const&);
    ~stratified_charges();

    stratified_charges& operator=(stratified_charges const&);

    stratified_entity const& datum(std::string const& name) const;

    // TODO ?? These things are not implemented correctly:
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
        ) const;

    double tiered_m_and_e(mcenum_gen_basis basis, double assets) const;
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

    void ascribe_members();

    stratified_entity& datum(std::string const& name);

    // Deprecated: for backward compatibility only. Prefer datum().
    stratified_entity& raw_entity(e_stratified);

    // xml_serializable required implementation.
    virtual int                class_version() const;
    virtual std::string const& xml_root_name() const;

    // xml_serializable overrides.
    virtual void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        );
    virtual void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const;
    virtual void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_leaf_name
        ) const;

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

    double tiered_curr_m_and_e(double assets) const;
    double tiered_guar_m_and_e(double assets) const;

    stratified_entity CurrSepAcctLoadBandedByPrem;
    stratified_entity GuarSepAcctLoadBandedByPrem;
    stratified_entity CurrSepAcctLoadBandedByAssets;
    stratified_entity GuarSepAcctLoadBandedByAssets;
    stratified_entity CurrMandETieredByAssets;
    stratified_entity GuarMandETieredByAssets;
    stratified_entity AssetCompTieredByAssets;
    stratified_entity InvestmentMgmtFeeTieredByAssets;
    stratified_entity CurrSepAcctLoadTieredByAssets;
    stratified_entity GuarSepAcctLoadTieredByAssets;
    stratified_entity TieredAKPremTax;
    stratified_entity TieredDEPremTax;
    stratified_entity TieredSDPremTax;
};

void LMI_SO load(stratified_charges      &, fs::path const&);
void LMI_SO save(stratified_charges const&, fs::path const&);

#endif // stratified_charges_hpp

