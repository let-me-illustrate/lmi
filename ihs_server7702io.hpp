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

#include "mc_enum.hpp"
#include "mc_enum_types.hpp"

#include <iosfwd>
#include <string>

struct Server7702Input
{
    std::string      UniqueIdentifier;           //  an arbitrary string that identifies the contract uniquely. The contract number may be used, but remember that the server maintains no database of actual contracts.
    bool             IsIssuedToday;              //  true if the contract is issued or reissued today. Used to prevent adjustable events at issue, which must not occur.
    int              Duration;                   //  number of policy years completed since issue (so it starts at 0).
    double           GrossNontaxableWithdrawal;  // UNUSED IN OLD TESTDECK the nontaxable portion of partial surrenders, including any withdrawal fees, plus involuntary withdrawals to restrict NAAR for reinsurance, plus amounts returned to preserve §7702A status.
    double           Premium;                    // UNUSED IN OLD TESTDECK gross payments, including those paid by a waiver benefit, before subtracting any "GrossNontaxableWithdrawal" on the same date, but net of any charges for non-qualified additional benefits that are not prefunded.
    double           DecreaseRequiredByContract; // UNUSED amount of decrease in specified amount required by the contract's terms, as for example in a decreasing term contract: unsupported, so use 0.0 for now.
    std::string      ProductName;                //  the only permissible values are those for which data files exist.
    mce_uw_basis     UnderwritingBasis;          //  permissible values are Medical, Paramedical, Nonmedical, Simplified_issue, and Guaranteed_issue, and I assume this is set at issue and can never change.
    double           PremTaxLoadRate;            //  the rate actually charged as a premium load. Example: 0.02 means a 2% load. Used to determine whether any load reflected in §7702 calculations has changed.
    double           TieredAssetChargeRate;      // UNUSED the tiered rate actually charged against assets, reflecting the current tier. Example: 0.0050 means fifty basis points. Used to determine whether any load reflected in §7702 calculations has changed.
    double           LeastBenefitAmountEver;     // UNUSED the lowest face amount ever used in §7702 calculations since the contract was issued; equal at issue to DB. The server will always calculate this but never store it. The client will always store it but never calculate it, except to set it equal to DB at issue.

    // The names of the next several parameters begin with the prefix "Old" to indicate that they are evaluated before the day's transactions. Should an adjustable event occur, they describe the state of the contract before any transaction that might have caused the adjustable event. At issue, "Old" values are the contract's issue parameters.
    double           OldGuidelineLevelPremium;   //  as previously calculated by the server; 0.0 at issue.
    double           OldGuidelineSinglePremium;  //  as previously calculated by the server; 0.0 at issue.

    // Each of the remaining parameters must be supplied in both a "new" and an "old" flavor, reflecting the state of the contract respectively before and after the day's transactions. For economy of presentation I don't repeat the list here with "Old" and "New" prefixes, but we'll have to do that when specifying the record layout.
    int              NewIssueAge;                //  as defined in the GPT specs.
    int              OldIssueAge;                // UNUSED IN OLD TESTDECK
    mce_gender       NewGender;                  //  male, female, or unisex.
    mce_gender       OldGender;                  // UNUSED IN OLD TESTDECK
    mce_smoking      NewSmoker;                  //  smoker, nonsmoker, or unismoke.
    mce_smoking      OldSmoker;

    mce_class        NewUnderwritingClass;       //  permissible values are Preferred, Standard, and Rated.
    mce_class        OldUnderwritingClass;       // UNUSED IN OLD TESTDECK
    mce_state        NewStateOfJurisdiction;     //  capitalized two-letter postal abbreviation for the state that determines the premium tax rate.
    mce_state        OldStateOfJurisdiction;     // UNUSED IN OLD TESTDECK

    mce_dbopt_7702   NewDeathBenefitOption;      //  DBO--A or B only.
    mce_dbopt_7702   OldDeathBenefitOption;
    double           NewBenefitAmount;           //  'death benefit' as defined in 7702: we are using the amount payable by reason of death.
    double           OldBenefitAmount;           //  the benefit amount before today's transactions; at issue, the amount described in the section on dumpins.
    double           NewSpecifiedAmount;         //  SA.
    double           OldSpecifiedAmount;
    double           NewTermAmount;              //  benefit amount of any term rider; 0.0 if none.
    double           OldTermAmount;
    bool             NewWaiverOfPremiumInForce;  // UNUSED true if waiver benefit in force, otherwise false.
    bool             OldWaiverOfPremiumInForce;  // UNUSED
    bool             NewPremiumsWaived;          // UNUSED true if waiver benefit in disability status, otherwise false.
    bool             OldPremiumsWaived;          // UNUSED
    mce_table_rating NewWaiverOfPremiumRating;   // UNUSED substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldWaiverOfPremiumRating;   // UNUSED
    bool             NewAccidentalDeathInForce;  //  true if accidental death benefit in force, otherwise false.
    bool             OldAccidentalDeathInForce;
    mce_table_rating NewAccidentalDeathRating;   // UNUSED substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldAccidentalDeathRating;   // UNUSED
    mce_table_rating NewTableRating;             // UNUSED
    mce_table_rating OldTableRating;             // UNUSED

    // The remaining parameters specify flat extras.
    // The distinction between temporary and permanent flats is important. I give the names with a suffix "0" here, but they need to be repeated with "1", "2", and so on for as many flats as permitted (how many is that?). As noted above, each has both an "old" and a "new" flavor.
    //
    // amount of the flat extra, expressed as dollars per thousand per year to conform to universal usage.
    double           NewPermanentFlatAmount0;    // UNUSED
    double           OldPermanentFlatAmount0;    // UNUSED
    double           NewPermanentFlatAmount1;    // UNUSED
    double           OldPermanentFlatAmount1;    // UNUSED
    double           NewPermanentFlatAmount2;    // UNUSED
    double           OldPermanentFlatAmount2;    // UNUSED
    // amount of the flat extra, expressed as dollars per thousand per year to conform to universal usage.
    double           NewTemporaryFlatAmount0;    // UNUSED
    double           OldTemporaryFlatAmount0;    // UNUSED
    double           NewTemporaryFlatAmount1;    // UNUSED
    double           OldTemporaryFlatAmount1;    // UNUSED
    double           NewTemporaryFlatAmount2;    // UNUSED
    double           OldTemporaryFlatAmount2;    // UNUSED
    // number of years the flat extra applies, measured from the issue date.
    int              NewTemporaryFlatDuration0;  // UNUSED
    int              OldTemporaryFlatDuration0;  // UNUSED
    int              NewTemporaryFlatDuration1;  // UNUSED
    int              OldTemporaryFlatDuration1;  // UNUSED
    int              NewTemporaryFlatDuration2;  // UNUSED
    int              OldTemporaryFlatDuration2;  // UNUSED

    double           TargetPremium; // the target premium. The client will always calculate and store it.
};

std::istream& operator>> (std::istream& is, Server7702Input& z);
std::ostream& operator<< (std::ostream& os, Server7702Input const& z);

struct Server7702Output
{
    std::string      UniqueIdentifier;              // the same identifier supplied as input.
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

std::istream& operator>> (std::istream& is, Server7702Output& z);
std::ostream& operator<< (std::ostream& os, Server7702Output const& z);

#endif // ihs_server7702io_hpp

