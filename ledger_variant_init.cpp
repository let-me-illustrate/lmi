// Ledger data that vary by basis--initialization.
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

#include "ledger_variant.hpp"

#include "basic_values.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"

void LedgerVariant::Init
    (BasicValues const& bv
    ,mcenum_gen_basis   gen_basis
    ,mcenum_sep_basis   sep_basis
    )
{
    Init(); // Zero out (almost) everything to start.

    GenBasis_ = gen_basis;
    SepBasis_ = sep_basis;

//  EOYDeathBft     =
//  AcctVal         =
//  CSVNet          =
//  CV7702          =
//  COICharge       =
//  RiderCharges    =
//  ExpenseCharges  =
    MlySAIntRate               = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_monthly_rate
        );
    MlyGAIntRate               = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_monthly_rate
        );
    MlyHoneymoonValueRate      = bv.InterestRates_->HoneymoonValueRate
        (GenBasis_
        ,mce_monthly_rate
        );
    MlyPostHoneymoonRate       = bv.InterestRates_->PostHoneymoonGenAcctRate
        (GenBasis_
        ,mce_monthly_rate
        );
    AnnSAIntRate               = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_annual_rate
        );
    AnnGAIntRate               = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_annual_rate
        );
    AnnHoneymoonValueRate      = bv.InterestRates_->HoneymoonValueRate
        (GenBasis_
        ,mce_annual_rate
        );
    AnnPostHoneymoonRate       = bv.InterestRates_->PostHoneymoonGenAcctRate
        (GenBasis_
        ,mce_annual_rate
        );

//  PrefLoanBalance =
//  TotalLoanBalance=
//  AvgDeathBft     =
//  SurrChg         =
//  TermPurchased   =
//  BaseDeathBft    =
//  ProjectedCoiCharge =
//  KFactor         =

    InitAnnLoanCredRate = bv.InterestRates_->RegLnCredRate
        (GenBasis_
        ,mce_annual_rate
        )[0];

    InitAnnGenAcctInt = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_annual_rate
        )
        [0]
        ;

    InitAnnSepAcctGrossInt = bv.InterestRates_->SepAcctGrossRate(SepBasis_)[0];

    InitAnnSepAcctNetInt = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_annual_rate
        )
        [0]
        ;

    InitTgtPremHiLoadRate = bv.Loads_->target_premium_load_maximum_premium_tax()[bv.yare_input_.InforceYear];
    InitMlyPolFee         = dblize(bv.Loads_->monthly_policy_fee(GenBasis_)     [bv.yare_input_.InforceYear]);

    FullyInitialized = true;
}
