// Rates that depend on the amount they're muliplied by.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: stratified_charges.cpp,v 1.11 2007-04-10 01:49:35 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "stratified_charges.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()
#include "stratified_algorithms.hpp"

#include <algorithm>
#include <cfloat>                 // DBL_MAX
#include <istream>
#include <fstream>
#include <ostream>
#include <stdexcept>

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
//
// File representation should be xml.

// Class stratified_entity implementation.

//============================================================================
stratified_entity::stratified_entity()
{
}

//============================================================================
stratified_entity::stratified_entity
    (std::vector<double> const& limits
    ,std::vector<double> const& values
    )
    :limits_(limits)
    ,values_(values)
{
    if(limits_.size() != values_.size())
        {
        throw std::logic_error("Tiered values and limits of unequal length.");
        }
}

//============================================================================
stratified_entity::~stratified_entity()
{
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

// INELEGANT !! Just omit the last band when the product editor is
// replaced.
//
// When reading or writing the entities, assert that the last limit is
// greater than (.999 * DBL_MAX): in effect, the highest representable
// number. Don't assert equality with DBL_MAX because the default
// precision of the stream >> and << operators for doubles hasn't been
// changed, so exact equality will not obtain.

//============================================================================
void stratified_entity::read(std::istream& is)
{
    std::vector<double>::size_type vector_size;
    std::vector<double>::value_type z;

    values_.clear();
    is >> vector_size;
    values_.reserve(vector_size);
    for(std::vector<double>::size_type j = 0; j < vector_size; j++)
        {
        is >> z;
        values_.push_back(z);
        }
    LMI_ASSERT(vector_size == values_.size());

    limits_.clear();
    is >> vector_size;
    limits_.reserve(vector_size);
    for(std::vector<double>::size_type j = 0; j < vector_size; j++)
        {
        is >> z;
        limits_.push_back(z);
        }
    LMI_ASSERT(vector_size == limits_.size());

    LMI_ASSERT(values_.size() == limits_.size());
    LMI_ASSERT((.999 * DBL_MAX) < limits_.back());
}

//============================================================================
void stratified_entity::write(std::ostream& os) const
{
    LMI_ASSERT(values_.size() == limits_.size());
    LMI_ASSERT((.999 * DBL_MAX) < limits_.back());

    std::vector<double>::const_iterator i;

    os << values_.size() << " ";
    for(i = values_.begin(); i < values_.end(); i++)
        {
        os << (*i) << " ";
        }
    os << '\n';

    os << limits_.size() << " ";
    for(i = limits_.begin(); i < limits_.end(); i++)
        {
        os << (*i) << " ";
        }
    os << '\n';
}

//============================================================================
std::istream& operator>>
    (std::istream&      is
    ,stratified_entity& z
    )
{
    z.read(is);
    return is;
}

//============================================================================
std::ostream& operator<<
    (std::ostream&            os
    ,stratified_entity const& z
    )
{
    z.write(os);
    return os;
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
    dictionary[e_topic_asset_tiered                    ] = stratified_entity();
    dictionary[e_topic_tiered_premium_tax              ] = stratified_entity();

    // Information-bearing nodes.

    dictionary[e_curr_sepacct_load_banded_by_premium   ] = stratified_entity();
    dictionary[e_guar_sepacct_load_banded_by_premium   ] = stratified_entity();
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
double stratified_charges::banded_sepacct_load
    (e_basis const& basis
    ,double         assets
    ,double         premium
    )
{
    switch(basis)
        {
        case e_currbasis:
            {
            return banded_curr_sepacct_load(assets, premium);
            }
            break;
        case e_guarbasis:
            {
            return banded_guar_sepacct_load(assets, premium);
            }
            break;
        case e_mdptbasis:
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
    return 0.0; // Actually unreachable, but some compilers don't know that.
}

//============================================================================
double stratified_charges::tiered_sepacct_load
    (e_basis const& basis
    ,double         assets
    ,double         premium
    )
{
    switch(basis)
        {
        case e_currbasis:
            {
            return tiered_curr_sepacct_load(assets, premium);
            }
            break;
        case e_guarbasis:
            {
            return tiered_guar_sepacct_load(assets, premium);
            }
            break;
        case e_mdptbasis:
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
    return 0.0; // Actually unreachable, but some compilers don't know that.
}

//============================================================================
double stratified_charges::banded_curr_sepacct_load
    (double assets
    ,double premium
    ) const
{
    stratified_entity const& x = raw_entity(e_curr_sepacct_load_banded_by_premium);
// TODO ?? DATABASE !! This is a goofy workaround until we can add a
// new 'e_curr_sepacct_load_banded_by_assets' entity.
    stratified_entity const& y = raw_entity(e_tiered_de_premium_tax);
    return
            banded_rate<double>() (premium, x.limits(), x.values())
        +   banded_rate<double>() (assets , y.limits(), y.values())
        ;
}

//============================================================================
double stratified_charges::banded_guar_sepacct_load
    (double assets
    ,double premium
    ) const
{
    stratified_entity const& x = raw_entity(e_guar_sepacct_load_banded_by_premium);
// TODO ?? DATABASE !! This is a goofy workaround until we can add a
// new 'e_guar_sepacct_load_banded_by_assets' entity. Until then,
// '* 1.25' provides an arbitrary but plausible margin of conservatism
// '* 1.25' the current-basis value.
    stratified_entity const& y = raw_entity(e_tiered_de_premium_tax);
    return
            banded_rate<double>() (premium, x.limits(), x.values())
        +   banded_rate<double>() (assets , y.limits(), y.values()) * 1.25
        ;
}

//============================================================================
double stratified_charges::tiered_current_m_and_e(double assets) const
{
    stratified_entity const& z = raw_entity(e_curr_m_and_e_tiered_by_assets);
    return tiered_rate<double>() (assets, z.limits(), z.values());
}

//============================================================================
double stratified_charges::tiered_guaranteed_m_and_e(double assets) const
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
    return *std::min_element(z.values().begin(), z.values().end());
}

namespace
{
    e_stratified premium_tax_table(e_state const& state)
        {
        enum_state const z = state.value();
        if(e_s_AK == z)
            {
            return e_tiered_ak_premium_tax;
            }
        else if(e_s_DE == z)
            {
            // TRICKY !! We'll eventually implement DE like this:
            //   return e_tiered_de_premium_tax;
            // But we haven't implemented DE's tiered premium tax yet,
            // so we treat it as any other state for now:
            return e_stratified_last;
            }
        else if(e_s_SD == z)
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
    (e_state const& state
    ,double         payment
    ,double         aggregate_payment
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
bool stratified_charges::premium_tax_is_tiered(e_state const& state) const
{
    return e_stratified_last != premium_tax_table(state);
}

//============================================================================
double stratified_charges::minimum_tiered_premium_tax_rate(e_state const& state) const
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
    std::ifstream is(filename.c_str());

    is >> raw_entity(e_curr_sepacct_load_banded_by_premium  );
    is >> raw_entity(e_guar_sepacct_load_banded_by_premium  );
    is >> raw_entity(e_curr_m_and_e_tiered_by_assets        );
    is >> raw_entity(e_guar_m_and_e_tiered_by_assets        );
    is >> raw_entity(e_asset_based_comp_tiered_by_assets    );
    is >> raw_entity(e_investment_mgmt_fee_tiered_by_assets );
    is >> raw_entity(e_curr_sepacct_load_tiered_by_assets   );
    is >> raw_entity(e_guar_sepacct_load_tiered_by_assets   );
    is >> raw_entity(e_tiered_ak_premium_tax                );
    is >> raw_entity(e_tiered_de_premium_tax                );
    is >> raw_entity(e_tiered_sd_premium_tax                );

    if(!is.good())
        {
        fatal_error()
            << "Unexpected end of stratified-data file '"
            << filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
    std::string dummy;
    is >> dummy;
    if(!is.eof())
        {
        fatal_error()
            << "Data past expected end of stratified-data file '"
            << filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void stratified_charges::write(std::string const& filename) const
{
    std::ofstream os(filename.c_str());
    if(!os.good())
        {
        warning()
            << "Cannot open stratified-data file '"
            << filename
            << "' for writing."
            << LMI_FLUSH
            ;
        }

    os << raw_entity(e_curr_sepacct_load_banded_by_premium  );
    os << raw_entity(e_guar_sepacct_load_banded_by_premium  );
    os << raw_entity(e_curr_m_and_e_tiered_by_assets        );
    os << raw_entity(e_guar_m_and_e_tiered_by_assets        );
    os << raw_entity(e_asset_based_comp_tiered_by_assets    );
    os << raw_entity(e_investment_mgmt_fee_tiered_by_assets );
    os << raw_entity(e_curr_sepacct_load_tiered_by_assets   );
    os << raw_entity(e_guar_sepacct_load_tiered_by_assets   );
    os << raw_entity(e_tiered_ak_premium_tax                );
    os << raw_entity(e_tiered_de_premium_tax                );
    os << raw_entity(e_tiered_sd_premium_tax                );

    if(!os.good())
        {
        fatal_error()
            << "Unable to write stratified-data file '"
            << filename
            << "'."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void stratified_charges::write_stratified_files()
{
    stratified_charges foo;

    foo.raw_entity(e_curr_sepacct_load_banded_by_premium  ).values_.push_back(0.0);
    foo.raw_entity(e_curr_sepacct_load_banded_by_premium  ).limits_.push_back(DBL_MAX);
    foo.raw_entity(e_guar_sepacct_load_banded_by_premium  ).values_.push_back(0.0);
    foo.raw_entity(e_guar_sepacct_load_banded_by_premium  ).limits_.push_back(DBL_MAX);

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

    // DE: not yet implemented.
    foo.raw_entity(e_tiered_de_premium_tax                ).values_.push_back (0.0);
    foo.raw_entity(e_tiered_de_premium_tax                ).limits_.push_back(DBL_MAX);

    foo.raw_entity(e_tiered_sd_premium_tax                ).values_.push_back (0.02500);
    foo.raw_entity(e_tiered_sd_premium_tax                ).values_.push_back (0.00080);
    foo.raw_entity(e_tiered_sd_premium_tax                ).limits_.push_back(100000.0);
    foo.raw_entity(e_tiered_sd_premium_tax                ).limits_.push_back(DBL_MAX);

    foo.write(AddDataDir("sample.tir"));
}

