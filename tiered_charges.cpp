// Tiered data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: tiered_charges.cpp,v 1.2 2005-02-13 23:17:18 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "tiered_charges.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "mathmisc.hpp"           // tiered_rate
#include "platform_dependent.hpp" // access()

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

// Class tiered_item_rep implementation.

//============================================================================
tiered_item_rep::tiered_item_rep()
{
}

//============================================================================
tiered_item_rep::tiered_item_rep
    (std::vector<double> const& a_bands
    ,std::vector<double> const& a_data
    )
    :bands_ (a_bands)
    ,data_  (a_data)
{
    if(bands_.size() != data_.size())
        {
        throw std::logic_error("Tiered data and bands of unequal length.");
        }
}

//============================================================================
tiered_item_rep::~tiered_item_rep()
{
}

// When reading or writing the data, assert that the last band is
// greater than (.999 * DBL_MAX): in effect, the highest representable
// number. Don't assert equality with DBL_MAX because the default
// precision of the stream >> and << operators for doubles hasn't been
// changed, so exact equality will not obtain.

//============================================================================
void tiered_item_rep::read(std::istream& is)
{
    std::vector<double>::size_type vector_size;
    std::vector<double>::value_type z;

    data_.clear();
    is >> vector_size;
    data_.reserve(vector_size);
    for(std::vector<double>::size_type j = 0; j < vector_size; j++)
        {
        is >> z;
        data_.push_back(z);
        }
    LMI_ASSERT(vector_size == data_.size());

    bands_.clear();
    is >> vector_size;
    bands_.reserve(vector_size);
    for(std::vector<double>::size_type j = 0; j < vector_size; j++)
        {
        is >> z;
        bands_.push_back(z);
        }
    LMI_ASSERT(vector_size == bands_.size());

    LMI_ASSERT(data_.size() == bands_.size());
    LMI_ASSERT((.999 * DBL_MAX) < bands_.back());
}

//============================================================================
void tiered_item_rep::write(std::ostream& os) const
{
    LMI_ASSERT(data_.size() == bands_.size());
    LMI_ASSERT((.999 * DBL_MAX) < bands_.back());

    std::vector<double>::const_iterator i;

    os << data_.size() << " ";
    for(i = data_.begin(); i < data_.end(); i++)
        {
        os << (*i) << " ";
        }
    os << '\n';

    os << bands_.size() << " ";
    for(i = bands_.begin(); i < bands_.end(); i++)
        {
        os << (*i) << " ";
        }
    os << '\n';
}

// Put these inline functions here because no other translation
// unit has any business using them.

//============================================================================
inline std::istream& operator>>
    (std::istream& is
    ,tiered_item_rep& z
    )
{
    z.read(is);
    return is;
}

//============================================================================
inline std::ostream& operator<<
    (std::ostream& os
    ,tiered_item_rep const& z
    )
{
    z.write(os);
    return os;
}

// Class tiered_charges implementation.

//============================================================================
tiered_charges::tiered_charges()
{
    initialize_dictionary();
}

//============================================================================
tiered_charges::tiered_charges(std::string const& filename)
{
    initialize_dictionary();
    read(filename);
}

//============================================================================
tiered_charges::~tiered_charges()
{
}

//============================================================================
void tiered_charges::initialize_dictionary()
{
    // Dummy nodes: root and topic headers
    dictionary[e_tier_first                            ] = tiered_item_rep();
    dictionary[e_tier_topic_experience_rating          ] = tiered_item_rep();
    dictionary[e_tier_topic_asset_based                ] = tiered_item_rep();
    dictionary[e_tier_topic_premium_tax                ] = tiered_item_rep();

    // Data-bearing nodes

    dictionary[e_tier_stabilization_reserve            ] = tiered_item_rep();
    dictionary[e_tier_coi_retention                    ] = tiered_item_rep();
    dictionary[e_tier_current_m_and_e                  ] = tiered_item_rep();
    dictionary[e_tier_guaranteed_m_and_e               ] = tiered_item_rep();
    dictionary[e_tier_asset_based_compensation         ] = tiered_item_rep();
    dictionary[e_tier_investment_management_fee        ] = tiered_item_rep();
    dictionary[e_tier_current_separate_account_load    ] = tiered_item_rep();
    dictionary[e_tier_guaranteed_separate_account_load ] = tiered_item_rep();

    dictionary[e_tier_ak_premium_tax                   ] = tiered_item_rep();
    dictionary[e_tier_de_premium_tax                   ] = tiered_item_rep();
    dictionary[e_tier_sd_premium_tax                   ] = tiered_item_rep();
}

//============================================================================
double tiered_charges::stabilization_reserve(double number_of_lives_inforce) const
{
    tiered_item_rep const& z = tiered_item(e_tier_stabilization_reserve);
    std::vector<double>::const_iterator band = std::upper_bound
        (z.bands().begin()
        ,z.bands().end()
        ,number_of_lives_inforce
        );
    return z.data()[band - z.bands().begin()];
}

//============================================================================
double tiered_charges::coi_retention(double number_of_lives_at_issue) const
{
    tiered_item_rep const& z = tiered_item(e_tier_coi_retention);
    std::vector<double>::const_iterator band = std::upper_bound
        (z.bands().begin()
        ,z.bands().end()
        ,number_of_lives_at_issue
        );
    return z.data()[band - z.bands().begin()];
}

//============================================================================
double tiered_charges::tiered_current_m_and_e(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_current_m_and_e);
    return tiered_rate<double>() (assets, z.bands(), z.data());
}

//============================================================================
double tiered_charges::tiered_guaranteed_m_and_e(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_guaranteed_m_and_e);
    return tiered_rate<double>() (assets, z.bands(), z.data());
}

//============================================================================
double tiered_charges::tiered_asset_based_compensation(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_asset_based_compensation);
    return tiered_rate<double>() (assets, z.bands(), z.data());
}

//============================================================================
double tiered_charges::tiered_investment_management_fee(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_investment_management_fee);
    return tiered_rate<double>() (assets, z.bands(), z.data());
}

//============================================================================
double tiered_charges::tiered_current_separate_account_load(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_current_separate_account_load);
    return tiered_rate<double>() (assets, z.bands(), z.data());
}

//============================================================================
double tiered_charges::tiered_guaranteed_separate_account_load(double assets) const
{
    tiered_item_rep const& z = tiered_item(e_tier_guaranteed_separate_account_load);
    return tiered_rate<double>() (assets, z.bands(), z.data());
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
double tiered_charges::minimum_tiered_spread_for_7702() const
{
    tiered_item_rep const& z = tiered_item(e_tier_current_separate_account_load);
    return *std::min_element(z.data().begin(), z.data().end());
}

namespace
{
    tiered_charges::tiered_enumerator premium_tax_table(e_state const& state)
        {
        enum_state const z = state.value();
        if(e_s_AK == z)
            {
            return tiered_charges::e_tier_ak_premium_tax;
            }
        else if(e_s_DE == z)
            {
            // TRICKY !! We'll eventually implement DE like this:
            //   return tiered_charges::e_tier_de_premium_tax;
            // But we haven't implemented DE's tiered premium tax yet,
            // so we treat it as any other state for now:
            return tiered_charges::e_tier_last;
            }
        else if(e_s_SD == z)
            {
            return tiered_charges::e_tier_sd_premium_tax;
            }
        else
            {
            return tiered_charges::e_tier_last;
            }
        }
} // Unnamed namespace.

//============================================================================
double tiered_charges::tiered_premium_tax
    (e_state const& state
    ,double         payment
    ,double         aggregate_payment
    ) const
{
    tiered_enumerator table = premium_tax_table(state);
    if(e_tier_last == table)
        {
        return 0.0;
        }
    else
        {
        tiered_item_rep const& z = tiered_item(table);
        return tiered_product<double>()
            (payment
            ,aggregate_payment
            ,z.bands()
            ,z.data()
            );
        }
}

//============================================================================
bool tiered_charges::premium_tax_is_tiered(e_state const& state) const
{
    return e_tier_last != premium_tax_table(state);
}

//============================================================================
double tiered_charges::minimum_tiered_premium_tax_rate(e_state const& state) const
{
    tiered_enumerator table = premium_tax_table(state);
    if(e_tier_last == table)
        {
        return 0.0;
        }
    else
        {
        tiered_item_rep const& z = tiered_item(table);
        LMI_ASSERT(!z.data().empty());
        return *std::min_element(z.data().begin(), z.data().end());
        }
}

//============================================================================
void tiered_charges::read(std::string const& filename)
{
    if(access(filename.c_str(), R_OK))
        {
        hobsons_choice()
            << "File '"
            << filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ; 
        }
    std::ifstream is(filename.c_str());

    is >> tiered_item(e_tier_stabilization_reserve           );
    is >> tiered_item(e_tier_coi_retention                   );
    is >> tiered_item(e_tier_current_m_and_e                 );
    is >> tiered_item(e_tier_guaranteed_m_and_e              );
    is >> tiered_item(e_tier_asset_based_compensation        );
    is >> tiered_item(e_tier_investment_management_fee       );
    is >> tiered_item(e_tier_current_separate_account_load   );
    is >> tiered_item(e_tier_guaranteed_separate_account_load);
    is >> tiered_item(e_tier_ak_premium_tax                  );
    is >> tiered_item(e_tier_de_premium_tax                  );
    is >> tiered_item(e_tier_sd_premium_tax                  );

    if(!is.good())
        {
        hobsons_choice()
            << "Unexpected end of tiered data file '"
            << filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
    std::string dummy;
    is >> dummy;
    if(!is.eof())
        {
        hobsons_choice()
            << "Data past expected end of tiered data file '"
            << filename
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void tiered_charges::write(std::string const& filename) const
{
    std::ofstream os(filename.c_str());
    if(!os.good())
        {
        warning()
            << "Cannot open tiered-data file '"
            << filename
            << "' for writing."
            << LMI_FLUSH
            ;
        }

    os << tiered_item(e_tier_stabilization_reserve           );
    os << tiered_item(e_tier_coi_retention                   );
    os << tiered_item(e_tier_current_m_and_e                 );
    os << tiered_item(e_tier_guaranteed_m_and_e              );
    os << tiered_item(e_tier_asset_based_compensation        );
    os << tiered_item(e_tier_investment_management_fee       );
    os << tiered_item(e_tier_current_separate_account_load   );
    os << tiered_item(e_tier_guaranteed_separate_account_load);
    os << tiered_item(e_tier_ak_premium_tax                  );
    os << tiered_item(e_tier_de_premium_tax                  );
    os << tiered_item(e_tier_sd_premium_tax                  );

    if(!os.good())
        {
        hobsons_choice()
            << "Unable to write fund file '"
            << filename
            << "'."
            << LMI_FLUSH
            ; 
        }
}

//============================================================================
void tiered_charges::write_tier_files()
{
    tiered_charges foo;

    foo.tiered_item(e_tier_stabilization_reserve           ).data_.push_back(5.0);
    foo.tiered_item(e_tier_stabilization_reserve           ).data_.push_back(3.0);
    foo.tiered_item(e_tier_stabilization_reserve           ).data_.push_back(1.7);
    foo.tiered_item(e_tier_stabilization_reserve           ).bands_.push_back(100.0);
    foo.tiered_item(e_tier_stabilization_reserve           ).bands_.push_back(200.0);
    foo.tiered_item(e_tier_stabilization_reserve           ).bands_.push_back(DBL_MAX);

    foo.tiered_item(e_tier_coi_retention                   ).data_.push_back(10.0);
    foo.tiered_item(e_tier_coi_retention                   ).data_.push_back(7.0);
    foo.tiered_item(e_tier_coi_retention                   ).data_.push_back(5.0);
    foo.tiered_item(e_tier_coi_retention                   ).bands_.push_back(50.0);
    foo.tiered_item(e_tier_coi_retention                   ).bands_.push_back(500.0);
    foo.tiered_item(e_tier_coi_retention                   ).bands_.push_back(DBL_MAX);

    foo.tiered_item(e_tier_current_m_and_e                 ).data_.push_back(0.0);
    foo.tiered_item(e_tier_current_m_and_e                 ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_guaranteed_m_and_e              ).data_.push_back(0.0);
    foo.tiered_item(e_tier_guaranteed_m_and_e              ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_asset_based_compensation        ).data_.push_back(0.0);
    foo.tiered_item(e_tier_asset_based_compensation        ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_investment_management_fee       ).data_.push_back(0.0);
    foo.tiered_item(e_tier_investment_management_fee       ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_current_separate_account_load   ).data_.push_back(0.0);
    foo.tiered_item(e_tier_current_separate_account_load   ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_guaranteed_separate_account_load).data_.push_back(0.0);
    foo.tiered_item(e_tier_guaranteed_separate_account_load).bands_.push_back(DBL_MAX);

    // For AK and SD, these are the actual rates as of 2003-09-09. Statutes:
    // AK 21.09.210(m)
    // SD 10-4-22(2) (see also 58-6-70)

    foo.tiered_item(e_tier_ak_premium_tax                  ).data_.push_back(0.0270);
    foo.tiered_item(e_tier_ak_premium_tax                  ).data_.push_back(0.0010);
    foo.tiered_item(e_tier_ak_premium_tax                  ).bands_.push_back(100000.0);
    foo.tiered_item(e_tier_ak_premium_tax                  ).bands_.push_back(DBL_MAX);

    foo.tiered_item(e_tier_de_premium_tax                  ).data_.push_back(0.0);
    foo.tiered_item(e_tier_de_premium_tax                  ).bands_.push_back(DBL_MAX);

    foo.tiered_item(e_tier_sd_premium_tax                  ).data_.push_back(0.0250);
    foo.tiered_item(e_tier_sd_premium_tax                  ).data_.push_back(0.0008);
    foo.tiered_item(e_tier_sd_premium_tax                  ).bands_.push_back(100000.0);
    foo.tiered_item(e_tier_sd_premium_tax                  ).bands_.push_back(DBL_MAX);

    foo.write(AddDataDir("sample.tir"));

    write_proprietary_tier_files();
}

