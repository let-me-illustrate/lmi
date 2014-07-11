// GPT server I/O.
//
// Copyright (C) 1998, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#ifndef ihs_server7702io_hpp
#define ihs_server7702io_hpp

#include "config.hpp"

#include "gpt_input.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"

#include <string>

struct Server7702Input
{
    Server7702Input();
    Server7702Input(gpt_input const&);
    operator gpt_input() const;

    std::string      ContractNumber;             //  an arbitrary string that identifies the contract uniquely. The contract number may be used, but remember that the server maintains no database of actual contracts.
    int              InforceYear;                //  number of policy years completed since issue (so it starts at 0).
    double           PremsPaidDecrement;         // UNUSED IN OLD TESTDECK the nontaxable portion of partial surrenders, including any withdrawal fees, plus involuntary withdrawals to restrict NAAR for reinsurance, plus amounts returned to preserve §7702A status.
    double           Payment;                    // UNUSED IN OLD TESTDECK gross payments, including those paid by a waiver benefit, before subtracting any "PremsPaidDecrement" on the same date, but net of any charges for non-qualified additional benefits that are not prefunded.
    std::string      ProductName;                //  the only permissible values are those for which data files exist.
    mce_uw_basis     GroupUnderwritingType;      //  permissible values are Medical, Paramedical, Nonmedical, Simplified_issue, and Guaranteed_issue, and I assume this is set at issue and can never change.

    // The names of the next several parameters begin with the prefix "Old" to indicate that they are evaluated before the day's transactions. Should an adjustable event occur, they describe the state of the contract before any transaction that might have caused the adjustable event. At issue, "Old" values are the contract's issue parameters.
    double           InforceGlp;                 //  as previously calculated by the server; 0.0 at issue.
    double           InforceGsp;                 //  as previously calculated by the server; 0.0 at issue.

    // Each of the remaining parameters must be supplied in both a "new" and an "old" flavor, reflecting the state of the contract respectively before and after the day's transactions. For economy of presentation I don't repeat the list here with "Old" and "New" prefixes, but we'll have to do that when specifying the record layout.
    int              NewIssueAge;                //  as defined in the GPT specs.
    int              OldIssueAge;                // UNUSED IN OLD TESTDECK
    mce_gender       NewGender;                  //  male, female, or unisex.
    mce_gender       OldGender;                  // UNUSED IN OLD TESTDECK
    mce_smoking      NewSmoking;                 //  smoker, nonsmoker, or unismoke.
    mce_smoking      OldSmoking;

    mce_class        NewUnderwritingClass;       //  permissible values are Preferred, Standard, and Rated.
    mce_class        OldUnderwritingClass;       // UNUSED IN OLD TESTDECK
    mce_state        NewStateOfJurisdiction;     //  capitalized two-letter postal abbreviation for the state that determines the premium tax rate.
    mce_state        OldStateOfJurisdiction;     // UNUSED IN OLD TESTDECK

    mce_dbopt_7702   NewDbo;                     //  DBO--A or B only.
    mce_dbopt_7702   OldDbo;
    double           NewBenefitAmount;           //  'death benefit' as defined in 7702: we are using the amount payable by reason of death.
    double           OldBenefitAmount;           //  the benefit amount before today's transactions; at issue, the amount described in the section on dumpins.
    double           NewSpecAmt;                 //  SA.
    double           OldSpecAmt;
    double           NewQabTermAmt;              //  benefit amount of any term rider; 0.0 if none.
    double           OldQabTermAmt;
    bool             NewWaiverOfPremiumInForce;  // UNUSED true if waiver benefit in force, otherwise false.
    bool             OldWaiverOfPremiumInForce;  // UNUSED
    mce_table_rating NewWaiverOfPremiumRating;   // UNUSED substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldWaiverOfPremiumRating;   // UNUSED
    bool             NewAccidentalDeathInForce;  //  true if accidental death benefit in force, otherwise false.
    bool             OldAccidentalDeathInForce;
    mce_table_rating NewAccidentalDeathRating;   // UNUSED substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldAccidentalDeathRating;   // UNUSED
    mce_table_rating NewSubstandardTable;        // UNUSED
    mce_table_rating OldSubstandardTable;        // UNUSED
    double           NewTarget;                  // the target premium. The client will always calculate and store it.
    double           OldTarget;
};

struct Server7702Output
{
    std::string      ContractNumber;                // the same identifier supplied as input.
    int              Status;
    bool             AdjustableEventOccurred;
    double           GuidelineLevelPremium;         // the new GLP.
    double           GuidelineSinglePremium;        // the new GSP.
    double           GuidelineLevelPremiumPolicyA;  // the GLP for notional policy A-the policy as it was at the last adjustable event, or at issue if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyA; // the GSP for notional policy A-the policy as it was at the last adjustable event, or at issue if there has been no adjustable event.
    double           GuidelineLevelPremiumPolicyB;  // the GLP for notional policy B; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyB; // the GSP for notional policy B; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineLevelPremiumPolicyC;  // the GLP for notional policy C; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyC; // the GSP for notional policy C; 0.0 at issue or if there has been no adjustable event.
};

#endif // ihs_server7702io_hpp

