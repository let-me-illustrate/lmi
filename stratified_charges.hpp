// Rates that depend on the amount they're multiplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef stratified_charges_hpp
#define stratified_charges_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "cache_file_reads.hpp"
#include "mc_enum_type_enums.hpp"
#include "path.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <string>
#include <vector>

enum e_stratified
    {e_stratified_first

    ,e_topic_premium_banded
    ,e_curr_sepacct_load_banded_by_premium  // CurrSepAcctLoadBandedByPrem
    ,e_guar_sepacct_load_banded_by_premium  // GuarSepAcctLoadBandedByPrem

    ,e_topic_asset_banded
    ,e_curr_sepacct_load_banded_by_assets   // CurrSepAcctLoadBandedByAssets
    ,e_guar_sepacct_load_banded_by_assets   // GuarSepAcctLoadBandedByAssets

    ,e_topic_asset_tiered
    ,e_curr_m_and_e_tiered_by_assets        // CurrMandETieredByAssets
    ,e_guar_m_and_e_tiered_by_assets        // GuarMandETieredByAssets
    ,e_asset_based_comp_tiered_by_assets    // AssetCompTieredByAssets
    ,e_investment_mgmt_fee_tiered_by_assets // InvestmentMgmtFeeTieredByAssets
    ,e_curr_sepacct_load_tiered_by_assets   // CurrSepAcctLoadTieredByAssets
    ,e_guar_sepacct_load_tiered_by_assets   // GuarSepAcctLoadTieredByAssets

    ,e_topic_tiered_premium_tax
    ,e_tiered_ak_premium_tax                // TieredAKPremTax
    ,e_tiered_de_premium_tax                // TieredDEPremTax
    ,e_tiered_sd_premium_tax                // TieredSDPremTax

    ,e_stratified_last
    };

/// A tiered or banded datum.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO stratified_entity final
{
    friend class stratified_charges;
    friend class TierView;

  public:
    stratified_entity() = default;
    stratified_entity
        (std::vector<double> const& limits
        ,std::vector<double> const& values
        ,std::string const&         gloss = std::string()
        );

    stratified_entity(stratified_entity const&) = default;
    stratified_entity(stratified_entity&&) = default;
    stratified_entity& operator=(stratified_entity const&) = default;
    stratified_entity& operator=(stratified_entity&&) = default;
    ~stratified_entity() = default;

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

class LMI_SO stratified_charges final
    :public xml_serializable  <stratified_charges>
    ,public MemberSymbolTable <stratified_charges>
    ,public cache_file_reads  <stratified_charges>
{
    friend class TierDocument;

  public:
    explicit stratified_charges(fs::path const& filename);
    stratified_charges(stratified_charges const&);
    ~stratified_charges() override = default;

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

    double minimum_tiered_sepacct_load_for_7702() const;

    // Tiered premium tax in certain states.
    double tiered_premium_tax
        (mcenum_state state
        ,double       payment
        ,double       aggregate_payment
        ) const;
    bool premium_tax_is_tiered(mcenum_state) const;

    double maximum_tiered_premium_tax_rate(mcenum_state) const;
    double minimum_tiered_premium_tax_rate(mcenum_state) const;

    static void write_strata_files();
    static void write_proprietary_strata_files();

  private:
    stratified_charges(); // Private, but implemented for friends' use.

    void ascribe_members();

    stratified_entity& datum(std::string const& name);

    // Deprecated: for backward compatibility only. Prefer datum().
    stratified_entity& raw_entity(e_stratified);

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        ) override;
    void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const override;
    void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_basename
        ) const override;

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

LMI_SO void load(stratified_charges      &, fs::path const&);
LMI_SO void save(stratified_charges const&, fs::path const&);

LMI_SO stratified_entity const& StatutoryAKPremTax();
LMI_SO stratified_entity const& StatutoryDEPremTax();
LMI_SO stratified_entity const& StatutorySDPremTax();

#endif // stratified_charges_hpp
