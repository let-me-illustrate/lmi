// Rates that depend on the amount they're muliplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "stratified_charges.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "miscellany.hpp"         // minmax<T>()
#include "platform_dependent.hpp" // access()
#include "stratified_algorithms.hpp"
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/static_assert.hpp>

#include <algorithm>
#include <cfloat>                 // DBL_MAX
#include <istream>
#include <fstream>
#include <ostream>

// TODO ?? Shortcomings:
//
// Actual tax-accounting practice may use a simple approximation for
// AK and SD premium tax.
//
// DE tiered premium tax not used. Premium tax is tiered in AK and SD
// by policy-year premium for each contract separately, but in DE
// by calendar-year premium for all contracts owned by the same
// corporation combined. Those complications we'll eventually address
// elsewhere; meanwhile, this module contains code to represent the
// DE tiered rates, but ignores those rates and treats DE the same as
// any other state.

// Class stratified_entity implementation.

//============================================================================
stratified_entity::stratified_entity()
{
}

//============================================================================
stratified_entity::stratified_entity
    (std::vector<double> const& limits
    ,std::vector<double> const& values
    ,std::string const&         gloss
    )
    :limits_(limits)
    ,values_(values)
    ,gloss_ (gloss)
{
    assert_validity();
}

//============================================================================
stratified_entity::~stratified_entity()
{
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
    LMI_ASSERT(is_highest_representable_double(limits_.back()));
    minmax<double> extrema(limits_);
    LMI_ASSERT(0.0 <= extrema.minimum());
    LMI_ASSERT(0.0 <  extrema.maximum());
}

//============================================================================
std::vector<double> const& stratified_entity::limits() const
{
    return limits_;
}

//============================================================================
std::vector<double> const& stratified_entity::values() const
{
    return values_;
}

//============================================================================
std::string const& stratified_entity::gloss() const
{
    return gloss_;
}

//============================================================================
void stratified_entity::read(xml::element const& e)
{
    xml_serialize::get_element(e, "values", values_);
    xml_serialize::get_element(e, "limits", limits_);
    xml_serialize::get_element(e, "gloss" , gloss_ );

    assert_validity();
}

//============================================================================
void stratified_entity::write(xml::element& e) const
{
    assert_validity();

    xml_serialize::set_element(e, "values", values_);
    xml_serialize::set_element(e, "limits", limits_);
    xml_serialize::set_element(e, "gloss" , gloss_ );
}

// Class stratified_charges implementation.

//============================================================================
stratified_charges::stratified_charges()
{
    initialize_dictionary();
}

//============================================================================
stratified_charges::stratified_charges(std::string const& filename)
{
    initialize_dictionary();
    read(filename);
}

//============================================================================
stratified_charges::~stratified_charges()
{
}

//============================================================================
stratified_entity& stratified_charges::raw_entity(e_stratified e)
{
    return (*dictionary.find(e)).second;
}

//============================================================================
stratified_entity const& stratified_charges::raw_entity(e_stratified e) const
{
    return (*dictionary.find(e)).second;
}

//============================================================================
void stratified_charges::initialize_dictionary()
{
    // Dummy nodes: root and topic headers.
    dictionary[e_stratified_first                      ] = stratified_entity();
    dictionary[e_topic_premium_banded                  ] = stratified_entity();
    dictionary[e_topic_asset_banded                    ] = stratified_entity();
    dictionary[e_topic_asset_tiered                    ] = stratified_entity();
    dictionary[e_topic_tiered_premium_tax              ] = stratified_entity();

    // Information-bearing nodes.

    dictionary[e_curr_sepacct_load_banded_by_premium   ] = stratified_entity();
    dictionary[e_guar_sepacct_load_banded_by_premium   ] = stratified_entity();
    dictionary[e_curr_sepacct_load_banded_by_assets    ] = stratified_entity();
    dictionary[e_guar_sepacct_load_banded_by_assets    ] = stratified_entity();
    dictionary[e_curr_m_and_e_tiered_by_assets         ] = stratified_entity();
    dictionary[e_guar_m_and_e_tiered_by_assets         ] = stratified_entity();
    dictionary[e_asset_based_comp_tiered_by_assets     ] = stratified_entity();
    dictionary[e_investment_mgmt_fee_tiered_by_assets  ] = stratified_entity();
    dictionary[e_curr_sepacct_load_tiered_by_assets    ] = stratified_entity();
    dictionary[e_guar_sepacct_load_tiered_by_assets    ] = stratified_entity();

    dictionary[e_tiered_ak_premium_tax                 ] = stratified_entity();
    dictionary[e_tiered_de_premium_tax                 ] = stratified_entity();
    dictionary[e_tiered_sd_premium_tax                 ] = stratified_entity();
}

//============================================================================
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
            break;
        case mce_gen_guar:
            {
            return
                    banded_guar_sepacct_load(assets, premium, special_limit)
                +   tiered_guar_sepacct_load(assets, premium)
                ;
            }
            break;
        case mce_gen_mdpt:
            {
            fatal_error()
                << "Dynamic separate-account load not supported with "
                << "midpoint expense basis, because variable products "
                << "are not subject to the illustration reg."
                << LMI_FLUSH
                ;
            }
            break;
        default:
            {
            fatal_error() << "Case '" << basis << "' not found." << LMI_FLUSH;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
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
    stratified_entity const& x = raw_entity(e_curr_sepacct_load_banded_by_premium);
    stratified_entity const& y = raw_entity(e_curr_sepacct_load_banded_by_assets);
    return
            kx * banded_rate<double>() (premium, x.limits(), x.values())
        +   ky * banded_rate<double>() (assets , y.limits(), y.values())
        ;
}

//============================================================================
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
    stratified_entity const& x = raw_entity(e_guar_sepacct_load_banded_by_premium);
    stratified_entity const& y = raw_entity(e_guar_sepacct_load_banded_by_assets);
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
            break;
        case mce_gen_guar:
            {
            return tiered_guar_m_and_e(assets);
            }
            break;
        case mce_gen_mdpt:
            {
            fatal_error()
                << "Dynamic separate-account M&E not supported with "
                << "midpoint expense basis, because variable products "
                << "are not subject to the illustration reg."
                << LMI_FLUSH
                ;
            }
            break;
        default:
            {
            fatal_error() << "Case '" << basis << "' not found." << LMI_FLUSH;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

//============================================================================
double stratified_charges::tiered_curr_m_and_e(double assets) const
{
    stratified_entity const& z = raw_entity(e_curr_m_and_e_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_guar_m_and_e(double assets) const
{
    stratified_entity const& z = raw_entity(e_guar_m_and_e_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_asset_based_compensation(double assets) const
{
    stratified_entity const& z = raw_entity(e_asset_based_comp_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_investment_management_fee(double assets) const
{
    stratified_entity const& z = raw_entity(e_investment_mgmt_fee_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_curr_sepacct_load(double assets, double /* premium */) const
{
    stratified_entity const& z = raw_entity(e_curr_sepacct_load_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_guar_sepacct_load(double assets, double /* premium */) const
{
    stratified_entity const& z = raw_entity(e_guar_sepacct_load_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
// Tiered compensation is not reflected here in order to forestall
// an adjustment event if compensation decreases in the future.
// Although producers may generally be expected to resist decreases,
// it is conceivable that the incidence of compensation might be
// changed on a block of business to produce a more front-loaded
// pattern in general, with the inadvertent effect of reducing future
// compensation on a particular contract.
//
double stratified_charges::minimum_tiered_spread_for_7702() const
{
    stratified_entity const& z = raw_entity(e_curr_sepacct_load_tiered_by_assets);
    LMI_ASSERT(!z.values().empty());
    return *std::min_element(z.values().begin(), z.values().end());
}

namespace
{
    e_stratified premium_tax_table(mcenum_state state)
        {
        if(mce_s_AK == state)
            {
            return e_tiered_ak_premium_tax;
            }
        else if(mce_s_DE == state)
            {
            // TRICKY !! We'll eventually implement DE like this:
            //   return e_tiered_de_premium_tax;
            // But we haven't implemented DE's tiered premium tax yet,
            // so we treat it as any other state for now:
            return e_stratified_last;
            }
        else if(mce_s_SD == state)
            {
            return e_tiered_sd_premium_tax;
            }
        else
            {
            return e_stratified_last;
            }
        }
} // Unnamed namespace.

//============================================================================
double stratified_charges::tiered_premium_tax
    (mcenum_state state
    ,double       payment
    ,double       aggregate_payment
    ) const
{
    e_stratified table = premium_tax_table(state);
    if(e_stratified_last == table)
        {
        return 0.0;
        }
    else
        {
        stratified_entity const& z = raw_entity(table);
        return tiered_product<double>()
            (payment
            ,aggregate_payment
            ,z.limits()
            ,z.values()
            );
        }
}

//============================================================================
bool stratified_charges::premium_tax_is_tiered(mcenum_state state) const
{
    return e_stratified_last != premium_tax_table(state);
}

//============================================================================
double stratified_charges::minimum_tiered_premium_tax_rate(mcenum_state state) const
{
    e_stratified table = premium_tax_table(state);
    if(e_stratified_last == table)
        {
        return 0.0;
        }
    else
        {
        stratified_entity const& z = raw_entity(table);
        LMI_ASSERT(!z.values().empty());
        return *std::min_element(z.values().begin(), z.values().end());
        }
}

//============================================================================
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

    BOOST_STATIC_ASSERT(sizeof s_stratified_nodes / sizeof(char const*) == 1 + e_stratified_last);
} // Unnamed namespace.

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

//============================================================================
void stratified_charges::read(std::string const& filename)
{
    if(access(filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(filename);
    xml::element const& root = parser.root_node("strata");

#define READ(ELEMENT,ENTITY) xml_serialize::get_element(ELEMENT, s_stratified_nodes[ENTITY], raw_entity(ENTITY))

    READ(root, e_curr_sepacct_load_banded_by_premium  );
    READ(root, e_guar_sepacct_load_banded_by_premium  );
    READ(root, e_curr_sepacct_load_banded_by_assets   );
    READ(root, e_guar_sepacct_load_banded_by_assets   );
    READ(root, e_curr_m_and_e_tiered_by_assets        );
    READ(root, e_guar_m_and_e_tiered_by_assets        );
    READ(root, e_asset_based_comp_tiered_by_assets    );
    READ(root, e_investment_mgmt_fee_tiered_by_assets );
    READ(root, e_curr_sepacct_load_tiered_by_assets   );
    READ(root, e_guar_sepacct_load_tiered_by_assets   );
    READ(root, e_tiered_ak_premium_tax                );
    READ(root, e_tiered_de_premium_tax                );
    READ(root, e_tiered_sd_premium_tax                );

#undef READ
}

//============================================================================
void stratified_charges::write(std::string const& filename) const
{
    xml_lmi::xml_document document("strata");
    xml::element& root = document.root_node();

#define WRITE(ELEMENT,ENTITY) xml_serialize::set_element(ELEMENT, s_stratified_nodes[ENTITY], raw_entity(ENTITY));

    WRITE(root, e_curr_sepacct_load_banded_by_premium  );
    WRITE(root, e_guar_sepacct_load_banded_by_premium  );
    WRITE(root, e_curr_sepacct_load_banded_by_assets   );
    WRITE(root, e_guar_sepacct_load_banded_by_assets   );
    WRITE(root, e_curr_m_and_e_tiered_by_assets        );
    WRITE(root, e_guar_m_and_e_tiered_by_assets        );
    WRITE(root, e_asset_based_comp_tiered_by_assets    );
    WRITE(root, e_investment_mgmt_fee_tiered_by_assets );
    WRITE(root, e_curr_sepacct_load_tiered_by_assets   );
    WRITE(root, e_guar_sepacct_load_tiered_by_assets   );
    WRITE(root, e_tiered_ak_premium_tax                );
    WRITE(root, e_tiered_de_premium_tax                );
    WRITE(root, e_tiered_sd_premium_tax                );

#undef WRITE

    // Instead of this:
//    document.save_to_file(filename.c_str());
    // for the nonce, explicitly change the extension, in order to
    // force external product-file code to use the new extension.
    fs::path path(filename, fs::native);
    path = fs::change_extension(path, ".strata");
    document.save(path.string());
}

//============================================================================
void stratified_charges::write_stratified_files()
{
    // Guard against recurrence of the problem described here:
    //   http://lists.nongnu.org/archive/html/lmi/2008-02/msg00024.html
    status() << "This line does nothing, but must not fail." << std::flush;

    stratified_charges foo;

    foo.raw_entity(e_curr_sepacct_load_banded_by_premium  ).values_.push_back(0.0);
    foo.raw_entity(e_curr_sepacct_load_banded_by_premium  ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_guar_sepacct_load_banded_by_premium  ).values_.push_back(0.0);
    foo.raw_entity(e_guar_sepacct_load_banded_by_premium  ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_curr_sepacct_load_banded_by_assets   ).values_.push_back(0.0);
    foo.raw_entity(e_curr_sepacct_load_banded_by_assets   ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_guar_sepacct_load_banded_by_assets   ).values_.push_back(0.0);
    foo.raw_entity(e_guar_sepacct_load_banded_by_assets   ).limits_.push_back(DBL_MAX);

    foo.raw_entity(e_curr_m_and_e_tiered_by_assets        ).values_.push_back(0.0);
    foo.raw_entity(e_curr_m_and_e_tiered_by_assets        ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_guar_m_and_e_tiered_by_assets        ).values_.push_back(0.0);
    foo.raw_entity(e_guar_m_and_e_tiered_by_assets        ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_asset_based_comp_tiered_by_assets    ).values_.push_back(0.0);
    foo.raw_entity(e_asset_based_comp_tiered_by_assets    ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_investment_mgmt_fee_tiered_by_assets ).values_.push_back(0.0);
    foo.raw_entity(e_investment_mgmt_fee_tiered_by_assets ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_curr_sepacct_load_tiered_by_assets   ).values_.push_back(0.0);
    foo.raw_entity(e_curr_sepacct_load_tiered_by_assets   ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_guar_sepacct_load_tiered_by_assets   ).values_.push_back(0.0);
    foo.raw_entity(e_guar_sepacct_load_tiered_by_assets   ).limits_.push_back(DBL_MAX);

    // For AK and SD, these are the actual rates as of 2003-09-09. Statutes:
    // AK 21.09.210(m)
    // SD 10-4-22(2) (see also 58-6-70)

    foo.raw_entity(e_tiered_ak_premium_tax                ).values_.push_back (0.02700);
    foo.raw_entity(e_tiered_ak_premium_tax                ).values_.push_back (0.00100);
    foo.raw_entity(e_tiered_ak_premium_tax                ).limits_.push_back(100000.0);
    foo.raw_entity(e_tiered_ak_premium_tax                ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_tiered_ak_premium_tax).gloss_ = "AK 21.09.210(m)";

    // DE: not yet implemented.
    foo.raw_entity(e_tiered_de_premium_tax                ).values_.push_back (0.0);
    foo.raw_entity(e_tiered_de_premium_tax                ).limits_.push_back(DBL_MAX);

    foo.raw_entity(e_tiered_sd_premium_tax                ).values_.push_back (0.02500);
    foo.raw_entity(e_tiered_sd_premium_tax                ).values_.push_back (0.00080);
    foo.raw_entity(e_tiered_sd_premium_tax                ).limits_.push_back(100000.0);
    foo.raw_entity(e_tiered_sd_premium_tax                ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_tiered_sd_premium_tax).gloss_ = "SD 10-4-22(2) (see also 58-6-70)";

    foo.write(AddDataDir("sample.strata"));
}

/// Determine whether a double is in effect the highest representable.
///
/// Don't assert exact equality with DBL_MAX, which is especially
/// unlikely to obtain with values written to a file and then read
/// back.
///
/// SOMEDAY !! Strive to obviate this function. It's used only to
/// determine whether the top bracket's limit is effectively infinite.
/// However, the top bracket's limit should never have any other
/// value, so perhaps it should be treated as such implicitly, and not
/// actually expressed.

bool is_highest_representable_double(double z)
{
    return (.999 * DBL_MAX) < z;
}

