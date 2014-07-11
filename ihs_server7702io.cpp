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

Server7702Input::Server7702Input()
{
    // Do nothing.
}

Server7702Input::Server7702Input(gpt_input const& z)
{
    ContractNumber             = z["ContractNumber"].str();
    InforceYear                = exact_cast<tnr_duration>(z["InforceYear"])->value();
    PremsPaidDecrement         = exact_cast<tnr_nonnegative_double>(z["PremsPaidDecrement"])->value();
    Payment                    = exact_cast<tnr_nonnegative_double>(z["Payment"])->value();
    ProductName                = z["ProductName"].str();
    GroupUnderwritingType      = exact_cast<mce_uw_basis>(z["GroupUnderwritingType"])->value();
    InforceGlp                 = exact_cast<tnr_unrestricted_double>(z["InforceGlp"])->value();
    InforceGsp                 = exact_cast<tnr_unrestricted_double>(z["InforceGsp"])->value();
    NewIssueAge                = exact_cast<tnr_age>(z["IssueAge"])->value();
    OldIssueAge                = NewIssueAge;
    NewGender                  = exact_cast<mce_gender>(z["NewGender"])->value();
    OldGender                  = exact_cast<mce_gender>(z["OldGender"])->value();
    NewSmoking                 = exact_cast<mce_smoking>(z["NewSmoking"])->value();
    OldSmoking                 = exact_cast<mce_smoking>(z["OldSmoking"])->value();
    NewUnderwritingClass       = exact_cast<mce_class>(z["UnderwritingClass"])->value();
    OldUnderwritingClass       = NewUnderwritingClass;
    NewStateOfJurisdiction     = exact_cast<mce_state>(z["StateOfJurisdiction"])->value();
    OldStateOfJurisdiction     = NewStateOfJurisdiction;
    NewDbo                     = exact_cast<mce_dbopt_7702>(z["NewDbo"])->value();
    OldDbo                     = exact_cast<mce_dbopt_7702>(z["OldDbo"])->value();
    NewBenefitAmount           = exact_cast<tnr_nonnegative_double>(z["NewDeathBft"])->value();
    OldBenefitAmount           = exact_cast<tnr_nonnegative_double>(z["OldDeathBft"])->value();
    NewSpecAmt                 = exact_cast<tnr_nonnegative_double>(z["NewSpecAmt"])->value();
    OldSpecAmt                 = exact_cast<tnr_nonnegative_double>(z["OldSpecAmt"])->value();
    NewQabTermAmt              = exact_cast<tnr_nonnegative_double>(z["NewQabTermAmt"])->value();
    OldQabTermAmt              = exact_cast<tnr_nonnegative_double>(z["OldQabTermAmt"])->value();
    NewWaiverOfPremiumInForce  = false;
    OldWaiverOfPremiumInForce  = false;
    NewWaiverOfPremiumRating   = "None";
    OldWaiverOfPremiumRating   = "None";
    NewAccidentalDeathInForce  = false;
    OldAccidentalDeathInForce  = false;
    NewAccidentalDeathRating   = "None";
    OldAccidentalDeathRating   = "None";
    NewSubstandardTable        = exact_cast<mce_table_rating>(z["NewSubstandardTable"])->value();
    OldSubstandardTable        = exact_cast<mce_table_rating>(z["OldSubstandardTable"])->value();
    NewTarget                  = exact_cast<tnr_nonnegative_double>(z["NewTarget"])->value();
    OldTarget                  = exact_cast<tnr_nonnegative_double>(z["OldTarget"])->value();
}

Server7702Input::operator gpt_input() const
{
    gpt_input z;

    z["ContractNumber"]        = ContractNumber;
    z["InforceYear"]           = value_cast<std::string>(InforceYear);
    // For class gpt_input, 'InforceAsOfDate' is primary by default,
    // so it needs to be set here.
    calendar_date d = add_years
        (exact_cast<tnr_date>(z["EffectiveDate"])->value()
        ,exact_cast<tnr_duration>(z["InforceYear"])->value()
        ,true
        );
    z["InforceAsOfDate"] = value_cast<std::string>(d);
    z["PremsPaidDecrement"]    = value_cast<std::string>(PremsPaidDecrement);
    z["Payment"]               = value_cast<std::string>(Payment);
    z["ProductName"]           = ProductName;
    z["GroupUnderwritingType"] = GroupUnderwritingType.str();
    z["InforceGlp"]            = value_cast<std::string>(InforceGlp);
    z["InforceGsp"]            = value_cast<std::string>(InforceGsp);
    z["IssueAge"]              = value_cast<std::string>(NewIssueAge);
    LMI_ASSERT(OldIssueAge == NewIssueAge);
    z["NewGender"]             = NewGender.str();
    z["OldGender"]             = OldGender.str();
    z["NewSmoking"]            = NewSmoking.str();
    z["OldSmoking"]            = OldSmoking.str();
    z["UnderwritingClass"]     = NewUnderwritingClass.str();
    LMI_ASSERT(OldUnderwritingClass == NewUnderwritingClass);
    z["StateOfJurisdiction"]   = NewStateOfJurisdiction.str();
    LMI_ASSERT(OldStateOfJurisdiction == NewStateOfJurisdiction);
    z["NewDbo"]                = NewDbo.str();
    z["OldDbo"]                = OldDbo.str();
    z["NewDeathBft"]           = value_cast<std::string>(NewBenefitAmount);
    z["OldDeathBft"]           = value_cast<std::string>(OldBenefitAmount);
    z["NewSpecAmt"]            = value_cast<std::string>(NewSpecAmt);
    z["OldSpecAmt"]            = value_cast<std::string>(OldSpecAmt);
    // It's unclear whether Server7702Input's [Old|New]QabTermAmt
    // members were intended to represent an integrated term rider or
    // a QAB. That doesn't really matter, because these amounts were
    // never used in the old GPT calculations.
    z["NewQabTermAmt"]         = value_cast<std::string>(NewQabTermAmt);
    z["OldQabTermAmt"]         = value_cast<std::string>(OldQabTermAmt);
    z["NewSubstandardTable"]   = NewSubstandardTable.str();
    z["OldSubstandardTable"]   = OldSubstandardTable.str();
    z["NewTarget"]             = value_cast<std::string>(NewTarget);
    z["OldTarget"]             = value_cast<std::string>(OldTarget);

    return z;
}

