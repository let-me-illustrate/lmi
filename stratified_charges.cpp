// Rates that depend on the amount they're muliplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "stratified_charges.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "data_directory.hpp"
#include "ieee754.hpp"                  // infinity<>()
#include "miscellany.hpp"               // minmax
#include "my_proem.hpp"                 // ::write_proem()
#include "ssize_lmi.hpp"
#include "stratified_algorithms.hpp"
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <algorithm>

template class xml_serializable<stratified_charges>;

namespace xml_serialize
{
    template<>
    struct xml_io<stratified_entity>
    {
        static void to_xml(xml::element& e, stratified_entity const& t)
        {
            t.write(e);
        }

        static void from_xml(xml::element const& t, stratified_entity& e)
        {
            e.read(t);
        }
    };
} // namespace xml_serialize

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::str()
/// which is not useful here.

template<>
std::string value_cast<std::string>(stratified_entity const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::operator=(std::string const&)
/// which is not useful here.

template<>
stratified_entity value_cast<stratified_entity>(std::string const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

// Class stratified_entity implementation.

stratified_entity::stratified_entity
    (std::vector<double> const& limits
    ,std::vector<double> const& values
    ,std::string const&         gloss
    )
    :limits_ {limits}
    ,values_ {values}
    ,gloss_  {gloss}
{
    assert_validity();
}

bool stratified_entity::operator==(stratified_entity const& z) const
{
    return
           limits_ == z.limits_
        && values_ == z.values_
        && gloss_  == z.gloss_
        ;
}

/// Throw if invalid.
///
/// Assert the intersection of constraints on 'tiered' (incremental)
/// and 'banded' (cumulative) limits.
///
/// TODO ?? Banded limits are constrained to be nondecreasing, but
/// that cannot be asserted for now because tiered and banded limits
/// are not distinguished here. They could be distinguished by adding
/// a flag to the class's state. Alternatively, all limits could be
/// expressed in the same way.

void stratified_entity::assert_validity() const
{
    LMI_ASSERT(!values_.empty());
    LMI_ASSERT(!limits_.empty());
    LMI_ASSERT(values_.size() == limits_.size());
    LMI_ASSERT(infinity<double>() == limits_.back());
    minmax<double> extrema(limits_);
    LMI_ASSERT(0.0 <= extrema.minimum());
    LMI_ASSERT(0.0 <  extrema.maximum());
}

std::vector<double> const& stratified_entity::limits() const
{
    return limits_;
}

std::vector<double> const& stratified_entity::values() const
{
    return values_;
}

std::string const& stratified_entity::gloss() const
{
    return gloss_;
}

void stratified_entity::read(xml::element const& e)
{
    xml_serialize::get_element(e, "values", values_);
    xml_serialize::get_element(e, "limits", limits_);
    xml_serialize::get_element(e, "gloss" , gloss_ );

    assert_validity();
}

void stratified_entity::write(xml::element& e) const
{
    assert_validity();

    xml_serialize::set_element(e, "values", values_);
    xml_serialize::set_element(e, "limits", limits_);
    xml_serialize::set_element(e, "gloss" , gloss_ );
}

// Class stratified_charges implementation.

stratified_charges::stratified_charges()
{
    ascribe_members();
}

stratified_charges::stratified_charges(fs::path const& filename)
{
    ascribe_members();
    load(filename);
}

stratified_charges::stratified_charges(stratified_charges const& z)
    :xml_serializable  <stratified_charges> {}
    ,MemberSymbolTable <stratified_charges> {}
{
    ascribe_members();
    MemberSymbolTable<stratified_charges>::assign(z);
}

stratified_charges& stratified_charges::operator=(stratified_charges const& z)
{
    MemberSymbolTable<stratified_charges>::assign(z);
    return *this;
}

stratified_entity const& stratified_charges::datum(std::string const& name) const
{
    return *member_cast<stratified_entity>(operator[](name));
}

stratified_entity& stratified_charges::datum(std::string const& name)
{
    return *member_cast<stratified_entity>(operator[](name));
}

void stratified_charges::ascribe_members()
{
    ascribe("CurrSepAcctLoadBandedByPrem"     , &stratified_charges::CurrSepAcctLoadBandedByPrem    );
    ascribe("GuarSepAcctLoadBandedByPrem"     , &stratified_charges::GuarSepAcctLoadBandedByPrem    );
    ascribe("CurrSepAcctLoadBandedByAssets"   , &stratified_charges::CurrSepAcctLoadBandedByAssets  );
    ascribe("GuarSepAcctLoadBandedByAssets"   , &stratified_charges::GuarSepAcctLoadBandedByAssets  );
    ascribe("CurrMandETieredByAssets"         , &stratified_charges::CurrMandETieredByAssets        );
    ascribe("GuarMandETieredByAssets"         , &stratified_charges::GuarMandETieredByAssets        );
    ascribe("AssetCompTieredByAssets"         , &stratified_charges::AssetCompTieredByAssets        );
    ascribe("InvestmentMgmtFeeTieredByAssets" , &stratified_charges::InvestmentMgmtFeeTieredByAssets);
    ascribe("CurrSepAcctLoadTieredByAssets"   , &stratified_charges::CurrSepAcctLoadTieredByAssets  );
    ascribe("GuarSepAcctLoadTieredByAssets"   , &stratified_charges::GuarSepAcctLoadTieredByAssets  );
    ascribe("TieredAKPremTax"                 , &stratified_charges::TieredAKPremTax                );
    ascribe("TieredDEPremTax"                 , &stratified_charges::TieredDEPremTax                );
    ascribe("TieredSDPremTax"                 , &stratified_charges::TieredSDPremTax                );
}

namespace
{
    char const* s_stratified_nodes[] =
        {"stratified_first"

        ,"topic_premium_banded"
        ,"CurrSepAcctLoadBandedByPrem"
        ,"GuarSepAcctLoadBandedByPrem"

        ,"topic_asset_banded"
        ,"CurrSepAcctLoadBandedByAssets"
        ,"GuarSepAcctLoadBandedByAssets"

        ,"topic_asset_tiered"
        ,"CurrMandETieredByAssets"
        ,"GuarMandETieredByAssets"
        ,"AssetCompTieredByAssets"
        ,"InvestmentMgmtFeeTieredByAssets"
        ,"CurrSepAcctLoadTieredByAssets"
        ,"GuarSepAcctLoadTieredByAssets"

        ,"topic_tiered_premium_tax"
        ,"TieredAKPremTax"
        ,"TieredDEPremTax"
        ,"TieredSDPremTax"

        ,"stratified_last"
        };

    static_assert(lmi::ssize(s_stratified_nodes) == 1 + e_stratified_last);
} // Unnamed namespace.

stratified_entity& stratified_charges::raw_entity(e_stratified e)
{
    LMI_ASSERT(contains(member_names(), s_stratified_nodes[e]));
    return datum(s_stratified_nodes[e]);
}

// 'special_limit' is 'DB_DynSepAcctLoadLimit'.
double stratified_charges::stratified_sepacct_load
    (mcenum_gen_basis basis
    ,double           assets
    ,double           premium
    ,double           special_limit
    ) const
{
    switch(basis)
        {
        case mce_gen_curr:
            {
            return
                    banded_curr_sepacct_load(assets, premium, special_limit)
                +   tiered_curr_sepacct_load(assets, premium)
                ;
            }
            // break;
        case mce_gen_guar:
            {
            return
                    banded_guar_sepacct_load(assets, premium, special_limit)
                +   tiered_guar_sepacct_load(assets, premium)
                ;
            }
            // break;
        case mce_gen_mdpt:
            {
            alarum()
                << "Dynamic separate-account load not supported with "
                << "midpoint expense basis, because variable products "
                << "are not subject to the illustration reg."
                << LMI_FLUSH
                ;
            }
            break;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

double stratified_charges::banded_curr_sepacct_load
    (double assets
    ,double premium
    ,double special_limit
    ) const
{
    double kx = 1.0;
    double ky = 1.0;
    if(0.0 != assets)
        {
        kx = std::min(assets, special_limit) / assets;
        }
    stratified_entity const& x = datum("CurrSepAcctLoadBandedByPrem");
    stratified_entity const& y = datum("CurrSepAcctLoadBandedByAssets");
    return
            kx * banded_rate<double>() (premium, x.limits(), x.values())
        +   ky * banded_rate<double>() (assets , y.limits(), y.values())
        ;
}

double stratified_charges::banded_guar_sepacct_load
    (double assets
    ,double premium
    ,double special_limit
    ) const
{
    double kx = 1.0;
    double ky = 1.0;
    if(0.0 != assets)
        {
        kx = std::min(assets, special_limit) / assets;
        }
    stratified_entity const& x = datum("GuarSepAcctLoadBandedByPrem");
    stratified_entity const& y = datum("GuarSepAcctLoadBandedByAssets");
    return
            kx * banded_rate<double>() (premium, x.limits(), x.values())
        +   ky * banded_rate<double>() (assets , y.limits(), y.values())
        ;
}

double stratified_charges::tiered_m_and_e(mcenum_gen_basis basis, double assets) const
{
    switch(basis)
        {
        case mce_gen_curr:
            {
            return tiered_curr_m_and_e(assets);
            }
            // break;
        case mce_gen_guar:
            {
            return tiered_guar_m_and_e(assets);
            }
            // break;
        case mce_gen_mdpt:
            {
            alarum()
                << "Dynamic separate-account M&E not supported with "
                << "midpoint expense basis, because variable products "
                << "are not subject to the illustration reg."
                << LMI_FLUSH
                ;
            }
            break;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

double stratified_charges::tiered_curr_m_and_e(double assets) const
{
    stratified_entity const& z = datum("CurrMandETieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

double stratified_charges::tiered_guar_m_and_e(double assets) const
{
    stratified_entity const& z = datum("GuarMandETieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

double stratified_charges::tiered_asset_based_compensation(double assets) const
{
    stratified_entity const& z = datum("AssetCompTieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

double stratified_charges::tiered_investment_management_fee(double assets) const
{
    stratified_entity const& z = datum("InvestmentMgmtFeeTieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

// The second argument (premium) is unused, so why does it exist?
double stratified_charges::tiered_curr_sepacct_load(double assets, double) const
{
    stratified_entity const& z = datum("CurrSepAcctLoadTieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

// The second argument (premium) is unused, so why does it exist?
double stratified_charges::tiered_guar_sepacct_load(double assets, double) const
{
    stratified_entity const& z = datum("GuarSepAcctLoadTieredByAssets");
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

/// Lowest tiered separate-account load.
///
/// Tiered compensation is not reflected here in order to forestall
/// an adjustment event if compensation decreases in the future.
/// Although producers may generally be expected to resist decreases,
/// it is conceivable that the incidence of compensation might be
/// changed on a block of business to produce a more front-loaded
/// pattern in general, with the inadvertent effect of reducing future
/// compensation on a particular contract.
///
/// TODO ?? TAXATION !! Missing "CurrSepAcctLoadBandedByAssets".
/// But "CurrSepAcctLoadBandedByPrem" is deliberately excluded,
/// because it's not based on assets. Elsewhere, "DB_CurrAcctValLoad"
/// should be added to the result.

double stratified_charges::minimum_tiered_spread_for_7702() const
{
    stratified_entity const& z = datum("CurrSepAcctLoadTieredByAssets");
    LMI_ASSERT(!z.values().empty());
    return *std::min_element(z.values().begin(), z.values().end());
}

namespace
{
std::string premium_tax_table(mcenum_state state)
{
    if(mce_s_AK == state)
        {
        return "TieredAKPremTax";
        }
    else if(mce_s_DE == state)
        {
        // TRICKY !! We'll eventually implement DE like this:
        //   return "TieredDEPremTax";
        // But we haven't implemented DE's tiered premium tax yet,
        // so we treat it as any other state for now:
        return "";
        }
    else if(mce_s_SD == state)
        {
        return "TieredSDPremTax";
        }
    else
        {
        return "";
        }
}
} // Unnamed namespace.

/// Tiered premium tax.
///
/// Actual tax-accounting practice may use a simple approximation for
/// AK and SD premium tax; this implementation follows the statutes.
///
/// DE tiered premium tax is not yet implemented. Premium tax in
/// AK and SD is tiered by policy-year premium for each contract
/// separately, but in DE by calendar-year premium for all contracts
/// owned by the same corporation, combined. Those complications might
/// eventually be addressed elsewhere; meanwhile, this module contains
/// code to represent the DE tiered rates, but ignores those rates and
/// treats DE the same as any other state except AK and SD.
///
/// UT tiered premium tax is not yet implemented. It applies only to
/// VLI premiums paid by a corporation or a corporate trust. How it's
/// affected by retaliation is unclear. Relevant citations:
///   http://le.utah.gov/~code/TITLE59/htm/59_09_010100.htm
///   http://le.utah.gov/~code/TITLE31A/htm/31A03_040100.htm

double stratified_charges::tiered_premium_tax
    (mcenum_state state
    ,double       payment
    ,double       aggregate_payment
    ) const
{
    std::string const table = premium_tax_table(state);
    if(table.empty())
        {
        return 0.0;
        }
    else
        {
        stratified_entity const& z = datum(table);
        return tiered_product<double>()
            (payment
            ,aggregate_payment
            ,z.limits()
            ,z.values()
            );
        }
}

bool stratified_charges::premium_tax_is_tiered(mcenum_state state) const
{
    return !premium_tax_table(state).empty();
}

/// Highest rate, for calculating pay-as-you-go premium.

double stratified_charges::maximum_tiered_premium_tax_rate(mcenum_state state) const
{
    std::string const table = premium_tax_table(state);
    if(table.empty())
        {
        return 0.0;
        }
    else
        {
        stratified_entity const& z = datum(table);
        LMI_ASSERT(!z.values().empty());
        return *std::max_element(z.values().begin(), z.values().end());
        }
}

/// Lowest rate, for conservative 7702 and 7702A calculations.

double stratified_charges::minimum_tiered_premium_tax_rate(mcenum_state state) const
{
    std::string const table = premium_tax_table(state);
    if(table.empty())
        {
        return 0.0;
        }
    else
        {
        stratified_entity const& z = datum(table);
        LMI_ASSERT(!z.values().empty());
        return *std::min_element(z.values().begin(), z.values().end());
        }
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20100525T2154Z

int stratified_charges::class_version() const
{
    return 0;
}

std::string const& stratified_charges::xml_root_name() const
{
    static std::string const s("strata");
    return s;
}

/// This override doesn't call redintegrate_ex_ante(); that wouldn't
/// make sense, at least not for now.

void stratified_charges::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 // file_version
    )
{
    xml_serialize::from_xml(e, datum(name));
}

void stratified_charges::write_element
    (xml::element&       parent
    ,std::string const&  name
    ) const
{
    xml_serialize::set_element(parent, name, datum(name));
}

void stratified_charges::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_leaf_name
    ) const
{
    ::write_proem(document, file_leaf_name);
}

void stratified_charges::write_strata_files()
{
    // Guard against recurrence of the problem described here:
    //   https://lists.nongnu.org/archive/html/lmi/2008-02/msg00024.html
    status() << "This line does nothing, but must not fail." << std::flush;

    static double const dbl_inf = infinity<double>();

    stratified_charges foo;

    foo.datum("CurrSepAcctLoadBandedByPrem"    ).values_.push_back(0.0);
    foo.datum("CurrSepAcctLoadBandedByPrem"    ).limits_.push_back(dbl_inf);
    foo.datum("GuarSepAcctLoadBandedByPrem"    ).values_.push_back(0.0);
    foo.datum("GuarSepAcctLoadBandedByPrem"    ).limits_.push_back(dbl_inf);
    foo.datum("CurrSepAcctLoadBandedByAssets"  ).values_.push_back(0.0);
    foo.datum("CurrSepAcctLoadBandedByAssets"  ).limits_.push_back(dbl_inf);
    foo.datum("GuarSepAcctLoadBandedByAssets"  ).values_.push_back(0.0);
    foo.datum("GuarSepAcctLoadBandedByAssets"  ).limits_.push_back(dbl_inf);

    foo.datum("CurrMandETieredByAssets"        ).values_.push_back(0.0);
    foo.datum("CurrMandETieredByAssets"        ).limits_.push_back(dbl_inf);
    foo.datum("GuarMandETieredByAssets"        ).values_.push_back(0.0);
    foo.datum("GuarMandETieredByAssets"        ).limits_.push_back(dbl_inf);
    foo.datum("AssetCompTieredByAssets"        ).values_.push_back(0.0);
    foo.datum("AssetCompTieredByAssets"        ).limits_.push_back(dbl_inf);
    foo.datum("InvestmentMgmtFeeTieredByAssets").values_.push_back(0.0);
    foo.datum("InvestmentMgmtFeeTieredByAssets").limits_.push_back(dbl_inf);
    foo.datum("CurrSepAcctLoadTieredByAssets"  ).values_.push_back(0.0);
    foo.datum("CurrSepAcctLoadTieredByAssets"  ).limits_.push_back(dbl_inf);
    foo.datum("GuarSepAcctLoadTieredByAssets"  ).values_.push_back(0.0);
    foo.datum("GuarSepAcctLoadTieredByAssets"  ).limits_.push_back(dbl_inf);

    foo.datum("TieredAKPremTax") = StatutoryAKPremTax();
    foo.datum("TieredDEPremTax") = StatutoryDEPremTax();
    foo.datum("TieredSDPremTax") = StatutorySDPremTax();

    foo.save(AddDataDir("sample.strata"));
}

/// Load from file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void load(stratified_charges& z, fs::path const& path)
{
    z.xml_serializable<stratified_charges>::load(path);
}

/// Save to file. This free function can be invoked across dll
/// boundaries, even though xml_serializable<> is instantiated only
/// in the present TU.

void save(stratified_charges const& z, fs::path const& path)
{
    z.xml_serializable<stratified_charges>::save(path);
}

/// AK parameters and citations as of 2017-05.
/// AK 21.09.210(m):
///   http://codes.findlaw.com/ak/title-21-insurance/ak-st-sect-21-09-210.html

stratified_entity const& StatutoryAKPremTax()
{
    static std::vector<double> const values = {0.02700, 0.00080};
    static std::vector<double> const limits = {100000.0, infinity<double>()};
    static stratified_entity const z(limits, values, "AK 21.09.210(m)");
    return z;
}

/// DE: not yet implemented.

stratified_entity const& StatutoryDEPremTax()
{
    static std::vector<double> const values = {0.0};
    static std::vector<double> const limits = {infinity<double>()};
    static stratified_entity const z(limits, values, "DE [not implemented]");
    return z;
}

/// SD parameters and citations as of 2011-05.
/// SD 10-44-2(2) and 58-6-70:
///   http://legis.state.sd.us/statutes/DisplayStatute.aspx?Type=Statute&Statute=10-44-2
///   http://legis.state.sd.us/statutes/DisplayStatute.aspx?Statute=58-6&Type=Statute
/// SD Chapter 260 (HB 1200), signed 2008-02-19, amended 58-6-70 by
/// removing the former million-dollar first-year-premium threshold:
///   http://legis.state.sd.us/sessions/2008/SessionLaws/DisplayChapter.aspx?Chapter=260

stratified_entity const& StatutorySDPremTax()
{
    static std::vector<double> const values = {0.02500, 0.00080};
    static std::vector<double> const limits = {100000.0, infinity<double>()};
    static stratified_entity const z(limits, values, "SD 10-44-2(2), 58-6-70");
    return z;
}
