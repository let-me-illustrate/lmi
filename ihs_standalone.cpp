// GPT server kludge.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ihs_standalone.cpp,v 1.9 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define STANDALONE

#include "ihs_standalone.hpp"

#include "actuarial_table.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_server7702.hpp"
#include "loads.hpp"
#include "math_functors.hpp"
#include "round_to.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

//============================================================================
GPT_BasicValues::GPT_BasicValues
    (E_Product const&   a_ProductName
    ,e_gender const&    a_Gender
    ,e_class const&     a_UnderwritingClass
    ,e_smoking const&   a_Smoker
    ,int                a_IssueAge
    ,e_uw_basis const&  a_UnderwritingBasis
    ,e_state const&     a_StateOfJurisdiction
    ,double             a_FaceAmount
    ,e_dbopt const&     a_DBOpt
    ,bool               a_ADDInForce
    )
    :ProductName         (a_ProductName)
    ,Gender              (a_Gender)
    ,UnderwritingClass   (a_UnderwritingClass)
    ,Smoker              (a_Smoker)
    ,IssueAge            (a_IssueAge)
    ,UnderwritingBasis   (a_UnderwritingBasis)
    ,StateOfJurisdiction (a_StateOfJurisdiction)
    ,FaceAmount          (a_FaceAmount)
    ,DBOpt               (a_DBOpt)
    ,ADDInForce          (a_ADDInForce)
{
    Init();
}

//============================================================================
void GPT_BasicValues::Init()
{
    TDatabase local_database
        (ProductName
        ,Gender
        ,UnderwritingClass
        ,Smoker
        ,IssueAge
        ,UnderwritingBasis
        ,StateOfJurisdiction
        );

    if(IssueAge < local_database.Query(DB_MinIssAge))
        {
        throw server7702_product_rule_violated
            (
            std::string("Issue age less than minimum")
            );
        }
    if(local_database.Query(DB_MaxIssAge) < IssueAge)
        {
        throw server7702_product_rule_violated
            (
            std::string("Issue age greater than maximum")
            );
        }

    // TODO ?? It would be good to test other rules too.

    Length = static_cast<int>(local_database.Query(DB_EndtAge)) -IssueAge;

#error TODO ?? Fix this--it should go through the rounding GUI.
    round_to<double> const round_int_rate(8, r_to_nearest);
    // TODO ?? Something like this is wanted:
//    round_to<double> const& round_int_rate
//        (Values.GetRoundingRules().round_interest_rate()
//        );

    Loads local_loads
        (Length
        // TODO ?? BOLI products that amortize premium loads will
        // need to set this flag. An implementation is provided
        // in class Loads.
        ,false  // AmortizePremLoad
        ,0.0    // a_ScalarExtraCompLoad // TODO ?? Fix this.
        ,0.0    // a_ScalarExtraAssetComp // TODO ?? Fix this.
        ,local_database
        ,round_int_rate // TODO ?? Fix this.
        );

//  StreamableRoundingRules = new StreamableRoundingRules
//      (AddDataDir(ProductData->GetRoundingFilename())
//      );
//  StratifiedCharges_ = new stratified_charges
//      (AddDataDir(ProductData->GetTierFilename())
//      );

    std::vector<double> q_irc_7702 = actuarial_table
//      ("data/qx_cso"
        (AddDataDir("qx_cso")
        ,local_database.Query(DB_IRC7702QTable)
        ,IssueAge
        ,Length
        );

    a7pp7702A = actuarial_table
//      ("data/mm_lcm"
        (AddDataDir("mm_lcm")
        ,local_database.Query(DB_TAMRA7PayTable)
        ,IssueAge
        ,Length
        );

    TargetPremiumType =
        static_cast<e_modal_prem_type>(local_database.Query(DB_TgtPremType));

    // ET !! std::vector<double> mly7702qc = q_irc_7702 (12.0 - q_irc_7702);
    std::vector<double> mly7702qc(q_irc_7702);
    std::transform(mly7702qc.begin(), mly7702qc.end(), mly7702qc.begin(),
          std::bind1st(std::minus<double>(), 12.0)
          );
    std::transform
        (q_irc_7702.begin()
        ,q_irc_7702.end()
        ,mly7702qc.begin()
        ,mly7702qc.begin()
        ,std::divides<double>()
        );

    // A kludge
    double i12_7702 = i_upper_12_over_12_from_i<double>()(0.04);

    std::vector<double> mly7702ic(Length, i12_7702);

    std::vector<double> mly7702ig;
    local_database.Query(mly7702ig, DB_NAARDiscount);

    // ET !! mly7702ig = -1.0 + 1.0 / mly7702ig;
    std::transform(mly7702ig.begin(), mly7702ig.end(), mly7702ig.begin(),
          std::bind1st(std::divides<double>(), 1.0)
          );
    std::transform(mly7702ig.begin(), mly7702ig.end(), mly7702ig.begin(),
          std::bind2nd(std::minus<double>(), 1.0)
          );

    // Monthly charge per $1 of SA includes both
    // monthly load on SA and
    // ADD charge
    std::vector<double> local_mly_charge_sa
        (local_loads.specified_amount_load(e_currbasis)
        );
    if(ADDInForce)
        {
        mAddRates = actuarial_table
            (AddDataDir("mm_lcm")
            ,local_database.Query(DB_ADDTable)
            ,IssueAge
            ,Length
            );
        // ET !! local_mly_charge_sa += mAddRates;
        std::transform
            (local_mly_charge_sa.begin()
            ,local_mly_charge_sa.end()
            ,mAddRates.begin()
            ,local_mly_charge_sa.begin()
            ,std::plus<double>()
            );
        }

    Irc7702_ = new Irc7702
            (*this
            ,e_gpt  // Input->DefnLifeIns
            ,IssueAge
            ,mly7702qc  // MortalityRates_->Irc7702Q()
            ,mly7702ic
            ,mly7702ig
            // TODO ?? NEED DECISION on definition of face amount
            ,FaceAmount // TODO ?? Must include term amount.
            ,DBOpt
            ,local_loads.annual_policy_fee(e_currbasis)
            ,local_loads.monthly_policy_fee(e_currbasis)
            ,local_mly_charge_sa
            ,local_loads.target_total_load(e_currbasis)
            ,local_loads.excess_total_load(e_currbasis)
            );

// if database inconsistent, then
//      throw server7702_inconsistent_input();
}

//============================================================================
GPT_BasicValues::GPT_BasicValues(GPT_BasicValues const& rhs)
    :ProductName            (rhs.ProductName)
    ,Gender                 (rhs.Gender)
    ,UnderwritingClass      (rhs.UnderwritingClass)
    ,Smoker                 (rhs.Smoker)
    ,IssueAge               (rhs.IssueAge)
    ,UnderwritingBasis      (rhs.UnderwritingBasis)
    ,StateOfJurisdiction    (rhs.StateOfJurisdiction)
    ,FaceAmount             (rhs.FaceAmount)
    ,DBOpt                  (rhs.DBOpt)
{
    Init();
}

//============================================================================
GPT_BasicValues& GPT_BasicValues::operator=(GPT_BasicValues const& obj)
{
    if(this != &obj)
        {
        Init();
        }
    return *this;
}

//============================================================================
GPT_BasicValues::~GPT_BasicValues()
{
// TODO ?? Use smart pointers instead?
    delete Irc7702_;
}

//============================================================================
// DBOpt is ignored for now, but some product designs will use it someday
double GPT_BasicValues::GetTgtPrem
    (int            Year
    ,double         SpecAmt
    ,e_dbopt const& // DBOpt TODO ?? Unused for now.
    ,e_mode  const& // Mode TODO ?? Unused for now.
    ) const
{
    return GetModalTgtPrem
        (Year
        ,e_annual
        ,SpecAmt
        );
}

//============================================================================
double GPT_BasicValues::GetModalTgtPrem
    (int           Year
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    e_modal_prem_type const PremType =
        static_cast<e_modal_prem_type>(TargetPremiumType);
    return GetModalPrem(Year, Mode, SpecAmt, PremType);
}

//============================================================================
double GPT_BasicValues::GetModalPrem
    (int           Year
    ,e_mode const& Mode
    ,double        SpecAmt
    ,e_modal_prem_type const& PremType
    ) const
{
    if(e_monthly_deduction == PremType)
        {
        return GetModalPrem_MlyDed(Year, Mode, SpecAmt);
        }
    else if(e_modal_nonmec == PremType)
        {
        return GetModalPrem_MaxNonMec(Year, Mode, SpecAmt);
        }
    else
        {
        fatal_error()
            << "Case '"
            << PremType
            << "' not found."
            << LMI_FLUSH
            ;
        }
    return 0.0;
}

//============================================================================
double GPT_BasicValues::GetModalPrem_MaxNonMec
    (int           // Year TODO ?? Unused for now.
    ,e_mode const& Mode
    ,double        SpecAmt
    ) const
{
    // always use initial spec amt and mode--fixed at issue
    // round down--mustn't violate 7702A
    double temp = a7pp7702A[0];
//  double temp = MortalityRates_->SevenPayRates()[0];
    // always use initial spec amt and mode--fixed at issue
    // round down--mustn't violate 7702A
#error TODO ?? Fix this--it should go through the rounding GUI.
    round_to<double> round_max_premium(8, RoundItNearest); // TODO ?? Fix this.
    // TODO ?? Something like this is wanted:
//    round_to<double> const& round_int_rate
//        (Values.GetRoundingRules().round_interest_rate()
//        );
    return round_max_premium(temp * SpecAmt / Mode);
}

//============================================================================
// TODO ?? Not tested, but few products do it this way
// TODO ?? No arguments are used for now.
double GPT_BasicValues::GetModalPrem_MlyDed
    (int            // Year
    ,e_mode const&  // Mode
    ,double         // SpecAmt
    ) const
{
    fatal_error()
        << "Products with special target premium calculations not yet supported."
        << LMI_FLUSH
        ;
    return 1.0;
}

