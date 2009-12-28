// GPT server I/O.
//
// Copyright (C) 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_server7702io.cpp,v 1.5 2008-12-27 02:56:44 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_server7702io.hpp"

#include <iomanip>
#include <ios>
#include <istream>
#include <ostream>
#include <string>

// TODO ?? NEED DECISION on i/o representation.
//#include <limits>
namespace
{
    static int const prec = 20;
//      std::numeric_limits<double>::digits10;
}

// Sample input record (concatenate the two lines):
// 123456789 1 0 0 0 10000 0 .02 0 1000000 1000000 0 0 0 1000000
// 45 45 0 0 0 0 0 0 1000000 1000000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

// TODO ?? It would be better to perform range checking on bool input.

//============================================================================
std::istream& operator>> (std::istream& is, Server7702Input& z)
{
    is >> z.UniqueIdentifier;
    is >> z.IsIssuedToday;
    is >> z.Duration;
    is >> z.GrossNontaxableWithdrawal;
    is >> z.Premium;
    is >> z.DecreaseRequiredByContract;
    is >> z.ProductName;
    is >> z.UnderwritingBasis;
    is >> z.PremTaxLoadRate;
    is >> z.TieredAssetChargeRate;
    is >> z.LeastBenefitAmountEver;
    is >> z.TargetPremium;
    is >> z.OldGuidelineLevelPremium;
    is >> z.OldGuidelineSinglePremium;
    is >> z.NewIssueAge;
    is >> z.OldIssueAge;
    is >> z.NewGender;
    is >> z.OldGender;
    is >> z.NewSmoker;
    is >> z.OldSmoker;
    is >> z.NewUnderwritingClass;
    is >> z.OldUnderwritingClass;
    is >> z.NewStateOfJurisdiction;
    is >> z.OldStateOfJurisdiction;
    is >> z.NewDeathBenefitOption;
    is >> z.OldDeathBenefitOption;
    is >> z.NewBenefitAmount;
    is >> z.OldBenefitAmount;
    is >> z.NewSpecifiedAmount;
    is >> z.OldSpecifiedAmount;
    is >> z.NewTermAmount;
    is >> z.OldTermAmount;
    is >> z.NewWaiverOfPremiumInForce;
    is >> z.OldWaiverOfPremiumInForce;
    is >> z.NewPremiumsWaived;
    is >> z.OldPremiumsWaived;
    is >> z.NewWaiverOfPremiumRating;
    is >> z.OldWaiverOfPremiumRating;
    is >> z.NewAccidentalDeathInForce;
    is >> z.OldAccidentalDeathInForce;
    is >> z.NewAccidentalDeathRating;
    is >> z.OldAccidentalDeathRating;
    is >> z.NewTableRating;
    is >> z.OldTableRating;

    // I give the names with a suffix "0" here,
    // but they need to be repeated with "1", "2",
    // and so on for as many flats as permitted.
    // TODO ?? NEED DECISION how many is that? --assume it's 3
    is >> z.NewPermanentFlatAmount0;
    is >> z.OldPermanentFlatAmount0;
    is >> z.NewPermanentFlatAmount1;
    is >> z.OldPermanentFlatAmount1;
    is >> z.NewPermanentFlatAmount2;
    is >> z.OldPermanentFlatAmount2;
    is >> z.NewTemporaryFlatAmount0;
    is >> z.OldTemporaryFlatAmount0;
    is >> z.NewTemporaryFlatAmount1;
    is >> z.OldTemporaryFlatAmount1;
    is >> z.NewTemporaryFlatAmount2;
    is >> z.OldTemporaryFlatAmount2;
    is >> z.NewTemporaryFlatDuration0;
    is >> z.OldTemporaryFlatDuration0;
    is >> z.NewTemporaryFlatDuration1;
    is >> z.OldTemporaryFlatDuration1;
    is >> z.NewTemporaryFlatDuration2;
    is >> z.OldTemporaryFlatDuration2;

    return is;
}

//============================================================================
std::ostream& operator<< (std::ostream& os, Server7702Input const& z)
{
    os << std::setiosflags(std::ios_base::fixed) << std::setprecision(prec);

    os << z.UniqueIdentifier;
    os << ' ' << z.IsIssuedToday;
    os << ' ' << z.Duration;
    os << ' ' << z.GrossNontaxableWithdrawal;
    os << ' ' << z.Premium;
    os << ' ' << z.DecreaseRequiredByContract;
    os << ' ' << z.ProductName;
    os << ' ' << z.UnderwritingBasis;
    os << ' ' << z.PremTaxLoadRate;
    os << ' ' << z.TieredAssetChargeRate;
    os << ' ' << z.LeastBenefitAmountEver;
    os << ' ' << z.TargetPremium;
    os << ' ' << z.OldGuidelineLevelPremium;
    os << ' ' << z.OldGuidelineSinglePremium;
    os << ' ' << z.NewIssueAge;
    os << ' ' << z.OldIssueAge;
    os << ' ' << z.NewGender;
    os << ' ' << z.OldGender;
    os << ' ' << z.NewSmoker;
    os << ' ' << z.OldSmoker;
    os << ' ' << z.NewUnderwritingClass;
    os << ' ' << z.OldUnderwritingClass;
    os << ' ' << z.NewStateOfJurisdiction;
    os << ' ' << z.OldStateOfJurisdiction;
    os << ' ' << z.NewDeathBenefitOption;
    os << ' ' << z.OldDeathBenefitOption;
    os << ' ' << z.NewBenefitAmount;
    os << ' ' << z.OldBenefitAmount;
    os << ' ' << z.NewSpecifiedAmount;
    os << ' ' << z.OldSpecifiedAmount;
    os << ' ' << z.NewTermAmount;
    os << ' ' << z.OldTermAmount;
    os << ' ' << z.NewWaiverOfPremiumInForce;
    os << ' ' << z.OldWaiverOfPremiumInForce;
    os << ' ' << z.NewPremiumsWaived;
    os << ' ' << z.OldPremiumsWaived;
    os << ' ' << z.NewWaiverOfPremiumRating;
    os << ' ' << z.OldWaiverOfPremiumRating;
    os << ' ' << z.NewAccidentalDeathInForce;
    os << ' ' << z.OldAccidentalDeathInForce;
    os << ' ' << z.NewAccidentalDeathRating;
    os << ' ' << z.OldAccidentalDeathRating;
    os << ' ' << z.NewTableRating;
    os << ' ' << z.OldTableRating;

    // I give the names with a suffix "0" here,
    // but they need to be repeated with "1", "2",
    // and so on for as many flats as permitted
    // (how many is that?).
    os << ' ' << z.NewPermanentFlatAmount0;
    os << ' ' << z.OldPermanentFlatAmount0;
    os << ' ' << z.NewPermanentFlatAmount1;
    os << ' ' << z.OldPermanentFlatAmount1;
    os << ' ' << z.NewPermanentFlatAmount2;
    os << ' ' << z.OldPermanentFlatAmount2;
    os << ' ' << z.NewTemporaryFlatAmount0;
    os << ' ' << z.OldTemporaryFlatAmount0;
    os << ' ' << z.NewTemporaryFlatAmount1;
    os << ' ' << z.OldTemporaryFlatAmount1;
    os << ' ' << z.NewTemporaryFlatAmount2;
    os << ' ' << z.OldTemporaryFlatAmount2;
    os << ' ' << z.NewTemporaryFlatDuration0;
    os << ' ' << z.OldTemporaryFlatDuration0;
    os << ' ' << z.NewTemporaryFlatDuration1;
    os << ' ' << z.OldTemporaryFlatDuration1;
    os << ' ' << z.NewTemporaryFlatDuration2;
    os << ' ' << z.OldTemporaryFlatDuration2;

    os << std::endl;
    return os;
}

//============================================================================
std::istream& operator>> (std::istream& is, Server7702Output& z)
{
    is >> z.UniqueIdentifier;
    is >> z.Status;
    is >> z.AdjustableEventOccurred;
    is >> z.GuidelineLevelPremium;
    is >> z.GuidelineSinglePremium;
    is >> z.GuidelineLevelPremiumPolicyA;
    is >> z.GuidelineSinglePremiumPolicyA;
    is >> z.GuidelineLevelPremiumPolicyB;
    is >> z.GuidelineSinglePremiumPolicyB;
    is >> z.GuidelineLevelPremiumPolicyC;
    is >> z.GuidelineSinglePremiumPolicyC;

    return is;
}

//============================================================================
std::ostream& operator<< (std::ostream& os, Server7702Output const& z)
{
//      << setw(prec + 1 + prec)
//      << setfill('0')
//  os.width(prec + 1 + prec);
//  os.fill('0');
// TODO ?? NEED DECISION Do they want everything aligned?
// TODO ?? NEED DECISION If so, what width for each item?

    os << std::setiosflags(std::ios_base::fixed) << std::setprecision(prec);

    os << z.UniqueIdentifier;
    os << ' ' << z.Status;
    os << ' ' << z.AdjustableEventOccurred;
    os << ' ' << z.GuidelineLevelPremium;
    os << ' ' << z.GuidelineSinglePremium;
    os << ' ' << z.GuidelineLevelPremiumPolicyA;
    os << ' ' << z.GuidelineSinglePremiumPolicyA;
    os << ' ' << z.GuidelineLevelPremiumPolicyB;
    os << ' ' << z.GuidelineSinglePremiumPolicyB;
    os << ' ' << z.GuidelineLevelPremiumPolicyC;
    os << ' ' << z.GuidelineSinglePremiumPolicyC;

    os << std::endl;
    return os;
}

