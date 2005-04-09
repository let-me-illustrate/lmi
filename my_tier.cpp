// Product-specific fund data.
//
// Copyright (C) 2001, 2005 Gregory W. Chicares.
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

// $Id: my_tier.cpp,v 1.1 2005-04-09 16:20:19 chicares Exp $

// This file is a template for embedding product-specific data. Doing
// that creates a derived work covered by the GPL. But you may prefer
// not to publish your data, for instance because it is proprietary.
// In that case, the GPL does not permit you to distribute the derived
// work at all. But read the second paragraph of section 0 of the GPL
// carefully: it permits you to run your modified version of the
// program--and to distribute its output, which is not a derived work
// because it's merely your data, trivially cast in a format suitable
// for use with lmi. You can therefore distribute the files created by
// your modified version of this program, but not that program itself.
// Those files are all you need: distributing the program itself isn't
// necessary anyway.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "tiered_charges.hpp"

#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()

#include <cfloat> // DBL_MAX

//============================================================================
void tiered_charges::write_proprietary_tier_files()
{
    // Sample policy form.
    tiered_charges foo;

    foo.tiered_item(e_tier_stabilization_reserve           ).data_.push_back(1.0);
    foo.tiered_item(e_tier_stabilization_reserve           ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_coi_retention                   ).data_.push_back(1.0);
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
    foo.tiered_item(e_tier_ak_premium_tax                  ).data_.push_back(0.027);
    foo.tiered_item(e_tier_ak_premium_tax                  ).data_.push_back(0.001);
    foo.tiered_item(e_tier_ak_premium_tax                  ).bands_.push_back(100000.0);
    foo.tiered_item(e_tier_ak_premium_tax                  ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_de_premium_tax                  ).data_.push_back(0.0);
    foo.tiered_item(e_tier_de_premium_tax                  ).bands_.push_back(DBL_MAX);
    foo.tiered_item(e_tier_sd_premium_tax                  ).data_.push_back(0.025);
    foo.tiered_item(e_tier_sd_premium_tax                  ).data_.push_back(0.0008);
    foo.tiered_item(e_tier_sd_premium_tax                  ).bands_.push_back(100000.0);
    foo.tiered_item(e_tier_sd_premium_tax                  ).bands_.push_back(DBL_MAX);

    foo.write(AddDataDir("sample.tir"));

    // Another policy form....
}

