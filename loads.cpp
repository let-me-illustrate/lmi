// Loads and expense charges.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: loads.cpp,v 1.2 2005-02-12 12:59:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "loads.hpp"

#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "math_functors.hpp"

#include <stdexcept>

//============================================================================
Loads::Loads(TDatabase const& database)
{
    Init(database);
}

//============================================================================
void Loads::Init(TDatabase const& database)
{
    monthly_policy_fee_   .resize(n_illreg_bases);
    target_premium_load_  .resize(n_illreg_bases);
    excess_premium_load_  .resize(n_illreg_bases);
    specified_amount_load_.resize(n_illreg_bases);

    database.Query(monthly_policy_fee_   [e_guarbasis], DB_GuarPolFee     );
    database.Query(target_premium_load_  [e_guarbasis], DB_GuarPremLoadTgt);
    database.Query(excess_premium_load_  [e_guarbasis], DB_GuarPremLoadExc);
    database.Query(specified_amount_load_[e_guarbasis], DB_GuarSpecAmtLoad);

    database.Query(monthly_policy_fee_   [e_currbasis], DB_CurrPolFee     );
    database.Query(target_premium_load_  [e_currbasis], DB_CurrPremLoadTgt);
    database.Query(excess_premium_load_  [e_currbasis], DB_CurrPremLoadExc);
    database.Query(specified_amount_load_[e_currbasis], DB_CurrSpecAmtLoad);

    // Calculate midpoint as mean of current and guaranteed.
    // A different average might be used instead.
    if(is_subject_to_ill_reg(database.Query(DB_LedgerType)))
        {
        monthly_policy_fee_   [e_mdptbasis].resize(database.length());
        std::transform
            (monthly_policy_fee_[e_guarbasis].begin()
            ,monthly_policy_fee_[e_guarbasis].end()
            ,monthly_policy_fee_[e_currbasis].begin()
            ,monthly_policy_fee_[e_mdptbasis].begin()
            ,mean<double>()
            );
        target_premium_load_  [e_mdptbasis].resize(database.length());
        std::transform
            (target_premium_load_[e_guarbasis].begin()
            ,target_premium_load_[e_guarbasis].end()
            ,target_premium_load_[e_currbasis].begin()
            ,target_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        excess_premium_load_  [e_mdptbasis].resize(database.length());
        std::transform
            (excess_premium_load_[e_guarbasis].begin()
            ,excess_premium_load_[e_guarbasis].end()
            ,excess_premium_load_[e_currbasis].begin()
            ,excess_premium_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        specified_amount_load_[e_mdptbasis].resize(database.length());
        std::transform
            (specified_amount_load_[e_guarbasis].begin()
            ,specified_amount_load_[e_guarbasis].end()
            ,specified_amount_load_[e_currbasis].begin()
            ,specified_amount_load_[e_mdptbasis].begin()
            ,mean<double>()
            );
        }

    premium_tax_load_.push_back(0.0);
    dac_tax_load_    .push_back(0.0);
}

