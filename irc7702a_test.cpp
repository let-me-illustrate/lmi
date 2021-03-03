// 7702A--unit test.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ihs_irc7702a.hpp"

#include "round_to.hpp"
#include "test_tools.hpp"

round_to<double> const RoundNonMecPrem(2, r_downward);

// Test mec-at-issue argument.
void test00(Irc7702A z)
{
    std::vector<double> pmt_history;
    std::vector<double> bft_history;
    bft_history.push_back(100000.0);
    z.Initialize7702A
        (false       // a_Ignore
        ,true        // a_MecAtIssue
        ,45          // a_IssueAge
        ,100         // a_EndtAge
        ,0           // a_PolicyYear
        ,0           // a_PolicyMonth
        ,0           // a_ContractYear
        ,0           // a_ContractMonth
        ,0           // a_AVBeforeMatChg
        ,0           // a_LowestBft
        ,pmt_history // a_Pmts
        ,bft_history // a_Bfts
        );
    z.UpdateEOM7702A();
    LMI_TEST(z.IsMecAlready());
}

void test01(Irc7702A z)
{
    z.UpdateBOY7702A(0);
    z.UpdateBOM7702A(0);
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );

    // Two payments in the same period must be allowed so that the
    // necessary and unnecessary portions of a payment can be accepted
    // with a material change processed between them.
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );

    // Two payments in different months must of course be allowed.
    z.UpdateEOM7702A();
    z.UpdateBOM7702A(1);
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );

    // Dropping Bfts to zero should cause a MEC.
    // TODO ?? Should we assert that the first arg is > 0.0?
    // TODO ?? Is the second arg to UpdateBft7702A() really needed?
    // TODO ?? If it is, can we assert that it has the expected value?
    z.UpdateEOM7702A();
    LMI_TEST(!z.IsMecAlready());
    z.UpdateBft7702A
        (0.0
        ,0.0     // a_NewDB
        ,99999.9 // a_OldDB
        ,false   // a_IsInCorridor
        ,0.0     // a_NewSA
        ,0.0     // a_OldSA
        ,0.0     // a_CashValue
        );

    z.UpdateEOM7702A();
    LMI_TEST(z.IsMecAlready());
}

// Pay premium almost equal to specamt: expect MEC.
void test02(Irc7702A z)
{
    z.UpdateBOY7702A(0);
    z.UpdateBOM7702A(0);
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );
    z.UpdateBft7702A
        (0.0
        ,100000.0 // a_NewDB
        ,100000.0 // a_OldDB
        ,false    // a_IsInCorridor
        ,100000.0 // a_NewSA
        ,100000.0 // a_OldSA
        ,0.0      // a_CashValue
        );
    LMI_TEST(!z.IsMecAlready());
    z.UpdateEOM7702A();

    // Second month.
    z.UpdateBOM7702A(1);
    // Paying too much premium should cause a MEC.
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,99999.0  // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );
    z.UpdateEOM7702A();
    LMI_TEST(z.IsMecAlready());
}

// Decrease Bfts then increase Bfts.
void test03(Irc7702A z)
{
    z.UpdateBOY7702A(0);
    z.UpdateBOM7702A(0);
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );
    z.UpdateBft7702A
        (0.0
        ,99999.9 // a_NewDB
        ,99999.9 // a_OldDB
        ,false   // a_IsInCorridor
        ,99999.9 // a_NewSA
        ,99999.9 // a_OldSA
        ,0.0     // a_CashValue
        );
    z.UpdateEOM7702A();
    LMI_TEST(!z.IsMecAlready()); // TODO ?? Fails?

    // Second year, second month.
    z.UpdateBOY7702A(1);
    z.UpdateBOM7702A(1);
    z.UpdateBft7702A
        (0.0
        ,100000.1 // a_NewDB
        ,100000.1 // a_OldDB
        ,false    // a_IsInCorridor
        ,100000.1 // a_NewSA
        ,100000.1 // a_OldSA
        ,0.0      // a_CashValue
        );
    z.UpdatePmt7702A
        (0.0      // a_DeemedCashValue
        ,1000.0   // a_Payment
        ,false    // a_ThisPaymentIsUnnecessary
        ,1000.0   // a_TargetPrem
        ,.05      // a_LoadTarget
        ,.03      // a_LoadExcess
        ,0.0      // a_CashValue
        );
    z.UpdateEOM7702A();
    LMI_TEST(!z.IsMecAlready()); // TODO ?? Fails?
}

int test_main(int, char*[])
{
    std::vector<double> prem_rate;
    prem_rate.push_back(.055);
    prem_rate.push_back(.065);
    prem_rate.push_back(.075);

    std::vector<double> nsp;
    nsp.push_back(.1);
    nsp.push_back(.2);
    nsp.push_back(.3);

    Irc7702A z
        (mce_cvat
        ,mce_earlier_of_increase_or_unnecessary_premium
        ,false
        ,mce_allow_mec
        ,true
        ,true
        ,prem_rate
        ,nsp
        ,RoundNonMecPrem
        );

    test00(z);

    std::vector<double> pmt_history;
    std::vector<double> bft_history;
    bft_history.push_back(100000.0);
    z.Initialize7702A
        (false       // a_Ignore
        ,false       // a_MecAtIssue
        ,45          // a_IssueAge
        ,100         // a_EndtAge
        ,0           // a_PolicyYear
        ,0           // a_PolicyMonth
        ,0           // a_ContractYear
        ,0           // a_ContractMonth
        ,0           // a_AVBeforeMatChg
        ,100000.0    // a_LowestBft
        ,pmt_history // a_Pmts
        ,bft_history // a_Bfts
        );

    test01(z);
    test02(z);
    test03(z);

    return 0;
}
