// GPT server I/O.
//
// Copyright (C) 1998, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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
    int              UniqueIdentifier; // an arbitrary number that identifies the contract uniquely. The contract number may be used, but remember that the server maintains no database of actual contracts.
    bool             IsIssuedToday; // true if the contract is issued or reissued today. Used to prevent adjustable events at issue, which must not occur.
    int              Duration; // number of policy years completed since issue (so it starts at 0).
    double           GrossNontaxableWithdrawal; // the nontaxable portion of partial surrenders, including any withdrawal fees, plus involuntary withdrawals to restrict NAAR for reinsurance, plus amounts returned to preserve §7702A status.
    double           Premium; // gross payments, including those paid by a waiver benefit, before subtracting any "GrossNontaxableWithdrawal" on the same date, but net of any charges for non-qualified additional benefits that are not prefunded.
    double           DecreaseRequiredByContract; // amount of decrease in specified amount required by the contract's terms, as for example in a decreasing term contract: unsupported, so use 0.0 for now.
    std::string      ProductName; // the only permissible values are those for which data files exist.
    mce_uw_basis     UnderwritingBasis; // permissible values are Medical, Paramedical, Nonmedical, Simplified_issue, and Guaranteed_issue, and I assume this is set at issue and can never change.
    double           PremTaxLoadRate; // the rate actually charged as a premium load. Example: 0.02 means a 2% load. Used to determine whether any load reflected in §7702 calculations has changed.
    double           TieredAssetChargeRate; // the tiered rate actually charged against assets, reflecting the current tier. Example: 0.0050 means fifty basis points. Used to determine whether any load reflected in §7702 calculations has changed.
    double           LeastBenefitAmountEver; // the lowest face amount ever used in §7702 calculations since the contract was issued; equal at issue to DB. The server will always calculate this but never store it. The client will always store it but never calculate it, except to set it equal to DB at issue.

    // The names of the next several parameters begin with the prefix "Old" to indicate that they are evaluated before the day's transactions. Should an adjustable event occur, they describe the state of the contract before any transaction that might have caused the adjustable event. At issue, "Old" values are the contract's issue parameters.
    double           OldGuidelineLevelPremium; // as previously calculated by the server; 0.0 at issue.
    double           OldGuidelineSinglePremium; // as previously calculated by the server; 0.0 at issue.

    // Each of the remaining parameters must be supplied in both a "new" and an "old" flavor, reflecting the state of the contract respectively before and after the day's transactions. For economy of presentation I don't repeat the list here with "Old" and "New" prefixes, but we'll have to do that when specifying the record layout.
    int              NewIssueAge; // as defined in the GPT specs.
    int              OldIssueAge;
    mce_gender       NewGender; // male, female, or unisex.
    mce_gender       OldGender;
    mce_smoking      NewSmoker; // smoker, nonsmoker, or unismoke.
    mce_smoking      OldSmoker;

    mce_class        NewUnderwritingClass; // permissible values are Preferred, Standard, and Rated.
    mce_class        OldUnderwritingClass;
    mce_state        NewStateOfJurisdiction; // capitalized two-letter postal abbreviation for the state that determines the premium tax rate.
    mce_state        OldStateOfJurisdiction;

    mce_dbopt_7702   NewDeathBenefitOption; // DBO--A or B only.
    mce_dbopt_7702   OldDeathBenefitOption;
    double           NewBenefitAmount; // 'death benefit' as defined in 7702: we are using the amount payable by reason of death.
    double           OldBenefitAmount; // the benefit amount before today's transactions; at issue, the amount described in the section on dumpins.
    double           NewSpecifiedAmount; // SA.
    double           OldSpecifiedAmount;
    double           NewTermAmount; // benefit amount of any term rider; 0.0 if none.
    double           OldTermAmount;
    bool             NewWaiverOfPremiumInForce; // true if waiver benefit in force, otherwise false.
    bool             OldWaiverOfPremiumInForce;
    bool             NewPremiumsWaived; // true if waiver benefit in disability status, otherwise false.
    bool             OldPremiumsWaived;
    mce_table_rating NewWaiverOfPremiumRating; // substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldWaiverOfPremiumRating;
    bool             NewAccidentalDeathInForce; // true if accidental death benefit in force, otherwise false.
    bool             OldAccidentalDeathInForce;
    mce_table_rating NewAccidentalDeathRating; // substandard rating for premium waiver; I don't know what the permissible values are.
    mce_table_rating OldAccidentalDeathRating;
    mce_table_rating NewTableRating; // I think the permissible values are
    mce_table_rating OldTableRating;

    // The remaining parameters specify flat extras.
    // The distinction between temporary and permanent flats is important. I give the names with a suffix "0" here, but they need to be repeated with "1", "2", and so on for as many flats as permitted (how many is that?). As noted above, each has both an "old" and a "new" flavor.
    //
    // amount of the flat extra, expressed as dollars per thousand per year to conform to universal usage.
    double           NewPermanentFlatAmount0;
    double           OldPermanentFlatAmount0;
    double           NewPermanentFlatAmount1;
    double           OldPermanentFlatAmount1;
    double           NewPermanentFlatAmount2;
    double           OldPermanentFlatAmount2;
    // amount of the flat extra, expressed as dollars per thousand per year to conform to universal usage.
    double           NewTemporaryFlatAmount0;
    double           OldTemporaryFlatAmount0;
    double           NewTemporaryFlatAmount1;
    double           OldTemporaryFlatAmount1;
    double           NewTemporaryFlatAmount2;
    double           OldTemporaryFlatAmount2;
    // number of years the flat extra applies, measured from the issue date.
    int              NewTemporaryFlatDuration0;
    int              OldTemporaryFlatDuration0;
    int              NewTemporaryFlatDuration1;
    int              OldTemporaryFlatDuration1;
    int              NewTemporaryFlatDuration2;
    int              OldTemporaryFlatDuration2;

    double           TargetPremium; // the target premium. The client will always calculate and store it.
};

std::istream& operator>> (std::istream& is, Server7702Input& z);
std::ostream& operator<< (std::ostream& os, Server7702Input const& z);

struct Server7702Output
{
    int              UniqueIdentifier; // the same identifier supplied as input.
    int              Status;
    bool             AdjustableEventOccurred;
    double           GuidelineLevelPremium; // the new GLP.
    double           GuidelineSinglePremium; // the new GSP.
    double           GuidelineLevelPremiumPolicyA; // the GLP for notional policy A-the policy as it was at the last adjustable event, or at issue if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyA; // the GSP for notional policy A-the policy as it was at the last adjustable event, or at issue if there has been no adjustable event.
    double           GuidelineLevelPremiumPolicyB; // the GLP for notional policy B; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyB; // the GSP for notional policy B; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineLevelPremiumPolicyC; // the GLP for notional policy C; 0.0 at issue or if there has been no adjustable event.
    double           GuidelineSinglePremiumPolicyC; // the GSP for notional policy C; 0.0 at issue or if there has been no adjustable event.
};

std::istream& operator>> (std::istream& is, Server7702Output& z);
std::ostream& operator<< (std::ostream& os, Server7702Output const& z);

#endif // ihs_server7702io_hpp

