// GPT server I/O.
//
// Copyright (C) 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_server7702io.hpp"

#include "assert_lmi.hpp"
#include "value_cast.hpp"

#include <iomanip>
#include <ios>
#include <istream>
#include <ostream>
#include <sstream>
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
    // Use of std::stringstream here is a kludge. That doesn't matter
    // because this obsolescent code will soon be removed.
    std::string s;
    std::stringstream ss;

    is >> z.UniqueIdentifier;
    is >> z.IsIssuedToday;
    is >> z.Duration;
    is >> z.GrossNontaxableWithdrawal;
    is >> z.Premium;
    is >> z.DecreaseRequiredByContract;
    is >> z.ProductName;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.UnderwritingBasis;
    is >> z.PremTaxLoadRate;
    is >> z.TieredAssetChargeRate;
    is >> z.LeastBenefitAmountEver;
//    is >> z.TargetPremium; // Apparently this came at the end long ago.
    is >> z.OldGuidelineLevelPremium;
    is >> z.OldGuidelineSinglePremium;
    is >> z.NewIssueAge;
    is >> z.OldIssueAge;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewGender;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldGender;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewSmoker;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldSmoker;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewUnderwritingClass;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldUnderwritingClass;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewStateOfJurisdiction;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldStateOfJurisdiction;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewDeathBenefitOption;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldDeathBenefitOption;
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
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewWaiverOfPremiumRating;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldWaiverOfPremiumRating;
    is >> z.NewAccidentalDeathInForce;
    is >> z.OldAccidentalDeathInForce;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewAccidentalDeathRating;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldAccidentalDeathRating;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.NewTableRating;
    s.clear(); ss.clear(); ss.str(""); is >> std::ws; is >> s; ss << s; ss >> z.OldTableRating;

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

    is >> z.TargetPremium; // Apparently this came at the end long ago.

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

    os << ' ' << z.TargetPremium;

    os << std::endl;
    return os;
}

Server7702Input::Server7702Input()
{
    // Do nothing.
}

Server7702Input::Server7702Input(gpt_input const& z)
{
    UniqueIdentifier           = z["ContractNumber"].str();
    Duration                   = exact_cast<tnr_duration>(z["InforceYear"])->value();
    IsIssuedToday              = 0 == Duration; // Casual, but strictly correct for all testdeck cases.
    GrossNontaxableWithdrawal  = exact_cast<tnr_nonnegative_double>(z["PremsPaidDecrement"])->value();
    Premium                    = exact_cast<tnr_nonnegative_double>(z["Payment"])->value();
    DecreaseRequiredByContract = 0.0;
    ProductName                = z["ProductName"].str();
    UnderwritingBasis          = exact_cast<mce_uw_basis>(z["GroupUnderwritingType"])->value();
    PremTaxLoadRate            = 0.0;
    TieredAssetChargeRate      = 0.0;
    LeastBenefitAmountEver     = 0.0;
    OldGuidelineLevelPremium   = exact_cast<tnr_unrestricted_double>(z["InforceGlp"])->value();
    OldGuidelineSinglePremium  = exact_cast<tnr_unrestricted_double>(z["InforceGsp"])->value();
    NewIssueAge                = exact_cast<tnr_age>(z["IssueAge"])->value();
    OldIssueAge                = NewIssueAge;
    NewGender                  = exact_cast<mce_gender>(z["NewGender"])->value();
    OldGender                  = exact_cast<mce_gender>(z["OldGender"])->value();
    NewSmoker                  = exact_cast<mce_smoking>(z["NewSmoking"])->value();
    OldSmoker                  = exact_cast<mce_smoking>(z["OldSmoking"])->value();
    NewUnderwritingClass       = exact_cast<mce_class>(z["UnderwritingClass"])->value();
    OldUnderwritingClass       = NewUnderwritingClass;
    NewStateOfJurisdiction     = exact_cast<mce_state>(z["StateOfJurisdiction"])->value();
    OldStateOfJurisdiction     = NewStateOfJurisdiction;
    NewDeathBenefitOption      = exact_cast<mce_dbopt_7702>(z["NewDbo"])->value();
    OldDeathBenefitOption      = exact_cast<mce_dbopt_7702>(z["OldDbo"])->value();
    NewBenefitAmount           = exact_cast<tnr_nonnegative_double>(z["NewDeathBft"])->value();
    OldBenefitAmount           = exact_cast<tnr_nonnegative_double>(z["OldDeathBft"])->value();
    NewSpecifiedAmount         = exact_cast<tnr_nonnegative_double>(z["NewSpecAmt"])->value();
    OldSpecifiedAmount         = exact_cast<tnr_nonnegative_double>(z["OldSpecAmt"])->value();
    NewTermAmount              = exact_cast<tnr_nonnegative_double>(z["NewQabTermAmt"])->value();
    OldTermAmount              = exact_cast<tnr_nonnegative_double>(z["OldQabTermAmt"])->value();
    NewWaiverOfPremiumInForce  = false;
    OldWaiverOfPremiumInForce  = false;
    NewPremiumsWaived          = false;
    OldPremiumsWaived          = false;
    NewWaiverOfPremiumRating   = "None";
    OldWaiverOfPremiumRating   = "None";
    NewAccidentalDeathInForce  = false;
    OldAccidentalDeathInForce  = false;
    NewAccidentalDeathRating   = "None";
    OldAccidentalDeathRating   = "None";
    NewTableRating             = exact_cast<mce_table_rating>(z["NewSubstandardTable"])->value();
    OldTableRating             = exact_cast<mce_table_rating>(z["OldSubstandardTable"])->value();
    NewPermanentFlatAmount0    = 0.0;
    OldPermanentFlatAmount0    = 0.0;
    NewPermanentFlatAmount1    = 0.0;
    OldPermanentFlatAmount1    = 0.0;
    NewPermanentFlatAmount2    = 0.0;
    OldPermanentFlatAmount2    = 0.0;
    NewTemporaryFlatAmount0    = 0.0;
    OldTemporaryFlatAmount0    = 0.0;
    NewTemporaryFlatAmount1    = 0.0;
    OldTemporaryFlatAmount1    = 0.0;
    NewTemporaryFlatAmount2    = 0.0;
    OldTemporaryFlatAmount2    = 0.0;
    NewTemporaryFlatDuration0  = 0;
    OldTemporaryFlatDuration0  = 0;
    NewTemporaryFlatDuration1  = 0;
    OldTemporaryFlatDuration1  = 0;
    NewTemporaryFlatDuration2  = 0;
    OldTemporaryFlatDuration2  = 0;
    LMI_ASSERT(z["OldTarget"] == z["NewTarget"]);
    TargetPremium              = exact_cast<tnr_nonnegative_double>(z["NewTarget"])->value();
}

Server7702Input::operator gpt_input() const
{
    gpt_input z;

    z["ContractNumber"]        = UniqueIdentifier;
    z["InforceYear"]           = value_cast<std::string>(Duration);
    // For class gpt_input, 'InforceAsOfDate' is primary by default,
    // so it needs to be set here.
    calendar_date d = add_years
        (exact_cast<tnr_date>(z["EffectiveDate"])->value()
        ,exact_cast<tnr_duration>(z["InforceYear"])->value()
        ,true
        );
    z["InforceAsOfDate"] = value_cast<std::string>(d);
    z["PremsPaidDecrement"]    = value_cast<std::string>(GrossNontaxableWithdrawal);
    z["Payment"]               = value_cast<std::string>(Premium);
    z["ProductName"]           = ProductName;
    z["GroupUnderwritingType"] = UnderwritingBasis.str();
    z["InforceGlp"]            = value_cast<std::string>(OldGuidelineLevelPremium);
    z["InforceGsp"]            = value_cast<std::string>(OldGuidelineSinglePremium);
    z["IssueAge"]              = value_cast<std::string>(NewIssueAge);
    LMI_ASSERT(OldIssueAge == NewIssueAge);
    z["NewGender"]             = NewGender.str();
    z["OldGender"]             = OldGender.str();
    z["NewSmoking"]            = NewSmoker.str();
    z["OldSmoking"]            = OldSmoker.str();
    z["UnderwritingClass"]     = NewUnderwritingClass.str();
    LMI_ASSERT(OldUnderwritingClass == NewUnderwritingClass);
    z["StateOfJurisdiction"]   = NewStateOfJurisdiction.str();
    LMI_ASSERT(OldStateOfJurisdiction == NewStateOfJurisdiction);
    z["NewDbo"]                = NewDeathBenefitOption.str();
    z["OldDbo"]                = OldDeathBenefitOption.str();
    z["NewDeathBft"]           = value_cast<std::string>(NewBenefitAmount);
    z["OldDeathBft"]           = value_cast<std::string>(OldBenefitAmount);
    z["NewSpecAmt"]            = value_cast<std::string>(NewSpecifiedAmount);
    z["OldSpecAmt"]            = value_cast<std::string>(OldSpecifiedAmount);
    // It's unclear whether Server7702Input's [Old|New]TermAmount
    // members were intended to represent an integrated term rider or
    // a QAB. That doesn't really matter, because these amounts were
    // never used in the old GPT calculations.
    z["NewQabTermAmt"]         = value_cast<std::string>(NewTermAmount);
    z["OldQabTermAmt"]         = value_cast<std::string>(OldTermAmount);
    z["NewSubstandardTable"]   = NewTableRating.str();
    z["OldSubstandardTable"]   = OldTableRating.str();
    // 'TargetPremium' should have had "old" and "new" variants.
    z["OldTarget"]             = value_cast<std::string>(TargetPremium);
    z["NewTarget"]             = value_cast<std::string>(TargetPremium);

    return z;
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

