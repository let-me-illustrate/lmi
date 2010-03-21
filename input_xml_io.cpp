// MVC Model for life-insurance illustrations: xml I/O.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "input.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp" // lmi_array_size()

#include <algorithm>      // std::find(), std::min()
#include <stdexcept>

template class xml_serializable<Input>;

namespace
{
std::string full_name
    (std::string first_name
    ,std::string middle_name
    ,std::string last_name
    )
{
    std::string s(first_name);
    if(!s.empty() && !middle_name.empty())
        {
        s += " ";
        }
    s += middle_name;
    if(!s.empty() && !last_name.empty())
        {
        s += " ";
        }
    s += last_name;
    return s;
}
} // Unnamed namespace.

/// Serial number of this class's xml version.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20050114T1947Z
/// version 2: 20080813T0131Z
/// version 3: 20090302T0509Z [see important note below]
/// version 4: 20090330T0137Z
/// version 5: 20090526T1331Z
///
/// Important note concerning version 3. On or about 20090311, some
/// end users were given an off-cycle release that should have used
/// code tagged 'lmi-20090223T2040Z', but erroneously used an untagged
/// (and untested) version of HEAD.

int Input::class_version() const
{
    return 5;
}

//============================================================================
std::string Input::xml_root_name() const
{
    return "cell";
}

/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool Input::is_detritus(std::string const& s) const
{
    static std::string const a[] =
        {"AgentFirstName"                // Single name instead.
        ,"AgentLastName"                 // Single name instead.
        ,"AgentMiddleName"               // Single name instead.
        ,"AssumedCaseNumberOfLives"      // Withdrawn.
        ,"CaseAssumedAssets"             // Withdrawn.
        ,"CorporationPremiumTableNumber" // Never implemented.
        ,"CorporationTaxpayerId"         // Would violate privacy.
        ,"CurrentCoiGrading"             // Withdrawn.
        ,"FirstName"                     // Single name instead.
        ,"InforceDcvDeathBenefit"        // Misbegotten.
        ,"InforceExperienceReserve"      // Renamed before implementation.
        ,"InsuredPremiumTableNumber"     // Never implemented.
        ,"LastName"                      // Single name instead.
        ,"MiddleName"                    // Single name instead.
        ,"NetMortalityChargeHistory"     // Renamed before implementation.
        ,"PartialMortalityTable"         // Never implemented.
        ,"PayLoanInterestInCash"         // Never implemented.
        ,"PolicyDate"                    // Never implemented.
        ,"PolicyLevelFlatExtra"          // Never implemented; poor name.
        ,"SocialSecurityNumber"          // Withdrawn: would violate privacy.
        ,"TermProportion"                // 'TermRiderProportion' instead.
        ,"YearsOfZeroDeaths"             // Withdrawn.
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return v.end() != std::find(v.begin(), v.end(), s);
}

/// Provide for backward compatibility before assigning values.

std::string Input::redintegrate_ex_ante
    (int                file_version
    ,std::string const& name
    ,std::string const& value
    ) const
{
    if(class_version() == file_version)
        {
        return value;
        }

    std::string new_value(value);

    // Prior to version 3, 'SolveType' distinguished:
    //   mce_solve_wd           --> !WithdrawToBasisThenLoan
    //   mce_solve_wd_then_loan -->  WithdrawToBasisThenLoan
    // but in version 4 that superfluous distinction was
    // removed. 'WithdrawToBasisThenLoan' needn't be altered
    // here because the material-implications above had
    // already been asserted in a prior revision. It is not
    // certain whether this distinction was present in the
    // erroneous version 3.

    if(file_version < 4)
        {
        if
            (  "AvoidMecMethod" == name
            &&  (
                    "Increase specified amount" == value
                ||  "Increase_specified_amount" == value
                )
            )
            {
            if(!global_settings::instance().regression_testing())
                {
                warning()
                    << "The obsolete 'Increase specified amount'"
                    << " MEC-avoidance strategy is no longer supported."
                    << " Consider using a non-MEC solve instead."
                    << LMI_FLUSH
                    ;
                }
            new_value = "Allow MEC";
            }
        if("DeprecatedSolveTgtAtWhich" == name)
            {
            new_value =
                  ("TgtAtRetirement" == value) ? "Retirement"
                : ("TgtAtYear"       == value) ? "Year"
                : ("TgtAtAge"        == value) ? "Age"
                : ("TgtAtMaturity"   == value) ? "Maturity"
                : throw std::runtime_error(value + ": unexpected solve-at duration.")
                ;
            }
        if("DeprecatedSolveFromWhich" == name)
            {
            new_value =
                  ("FromIssue"      == value) ? "Issue"
                : ("FromYear"       == value) ? "Year"
                : ("FromAge"        == value) ? "Age"
                : ("FromRetirement" == value) ? "Retirement"
                : throw std::runtime_error(value + ": unexpected solve-from duration.")
                ;
            }
        if("DeprecatedSolveToWhich" == name)
            {
            new_value =
                  ("ToRetirement"    == value) ? "Retirement"
                : ("ToYear"          == value) ? "Year"
                : ("ToAge"           == value) ? "Age"
                : ("ToMaturity"      == value) ? "Maturity"
                : throw std::runtime_error(value + ": unexpected solve-to duration.")
                ;
            }
        if("SolveBasis" == name)
            {
            new_value =
                  ("Current basis"    == value) ? "Current"
                : ("Current_basis"    == value) ? "Current"
                : ("Guaranteed basis" == value) ? "Guaranteed"
                : ("Guaranteed_basis" == value) ? "Guaranteed"
                : ("Midpoint basis"   == value) ? "Midpoint"
                : ("Midpoint_basis"   == value) ? "Midpoint"
                : throw std::runtime_error(value + ": unexpected deduction basis.")
                ;
            }

        if("SolveSeparateAccountBasis" == name)
            {
            new_value =
                  ("Input %"         == value) ? "Hypothetical"
                : ("Input_%"         == value) ? "Hypothetical"
                : ("Zero %"          == value) ? "Zero"
                : ("Zero_%"          == value) ? "Zero"
                : ("Half of input %" == value) ? "Half of hypothetical"
                : ("Half_of_input_%" == value) ? "Half of hypothetical"
                : throw std::runtime_error(value + ": unexpected separate-account basis.")
                ;
            }

        if("SolveType" == name)
            {
            new_value =
                  ("SolveNone"       == value) ? "No solve"
                : ("SolveSpecAmt"    == value) ? "Specified amount"
                : ("SolveEePrem"     == value) ? "Employee premium"
                : ("SolveErPrem"     == value) ? "Employer premium"
                : ("SolveLoan"       == value) ? "Loan"
                : ("SolveWD"         == value) ? "Withdrawal"
                : ("SolveWDThenLoan" == value) ? "Withdrawal"
                : throw std::runtime_error(value + ": unexpected solve type.")
                ;
            }

        if("SolveTarget" == name)
            {
            new_value =
                  ("SolveForEndt"     == value) ? "Endowment"
                : ("SolveForTarget"   == value) ? "Target CSV"
                : ("SolveForTaxBasis" == value) ? "CSV = tax basis"
                : ("SolveForNonMec"   == value) ? "Avoid MEC"
                : throw std::runtime_error(value + ": unexpected solve goal.")
                ;
            }

        if("GeneralAccountRateType" == name)
            {
            new_value =
                  ("CredRate"  == value) ? "Credited rate"
                : ("NetRate"   == value) ? "Credited rate"
                : ("GrossRate" == value) ? "Earned rate"
                : throw std::runtime_error(value + ": unexpected general-account type.")
                ;
            }

        if("SeparateAccountRateType" == name)
            {
            new_value =
                  ("CredRate"  == value) ? "Net rate"
                : ("NetRate"   == value) ? "Net rate"
                : ("GrossRate" == value) ? "Gross rate"
                : throw std::runtime_error(value + ": unexpected separate-account type.")
                ;
            }

        if("LoanRateType" == name)
            {
            new_value =
                  ("Fixed" == value) ? "Fixed loan rate"
                : ("VLR"   == value) ? "Variable loan rate"
                : throw std::runtime_error(value + ": unexpected loan-rate type.")
                ;
            }
        }

    return new_value;
}

/// Provide for backward compatibility after assigning values.

void Input::redintegrate_ex_post
    (int                                file_version
    ,std::map<std::string, std::string> detritus_map
    ,std::list<std::string>             residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    if(0 == file_version)
        {
        // An older version with no distinct 'file_version' didn't
        // have 'DefinitionOfMaterialChange', whose default value is
        // unacceptable for GPT.
        if
            (residuary_names.end() != std::find
                (residuary_names.begin()
                ,residuary_names.end()
                ,std::string("DefinitionOfMaterialChange")
                )
            )
            {
            DefinitionOfMaterialChange =
                mce_gpt == DefinitionOfLifeInsurance
                ? mce_adjustment_event
                : mce_earlier_of_increase_or_unnecessary_premium
                ;
            }

        operator[]("AgentName"  ) = full_name
            (detritus_map["AgentFirstName" ]
            ,detritus_map["AgentMiddleName"]
            ,detritus_map["AgentLastName"  ]
            );
        operator[]("InsuredName") = full_name
            (detritus_map["FirstName"      ]
            ,detritus_map["MiddleName"     ]
            ,detritus_map["LastName"       ]
            );
        }

    if(file_version < 2)
        {
        // 'UseCurrentDeclaredRate' was introduced 20071017T1454Z; its
        // default value of "Yes" would break backward compatibility.
        if
            (residuary_names.end() != std::find
                (residuary_names.begin()
                ,residuary_names.end()
                ,std::string("UseCurrentDeclaredRate")
                )
            )
            {
            UseCurrentDeclaredRate = "No";
            }

        // 'LastCoiReentryDate' was introduced 20071017T1454Z; its
        // default value may be inappropriate for files saved earlier.
        LastCoiReentryDate = std::min
            (LastCoiReentryDate.value()
            ,add_years(EffectiveDate.value(), InforceYear.value(), true)
            );
        }

    if(1 == file_version)
        {
        // Solve 'Year' values were saved in solve 'Time' entities,
        // apparently in this version only.
        //
        // However, default values for
        //   SolveTargetTime
        //   SolveEndTime
        // didn't work correctly with contemporary versions of the
        // program. Users had to change them in order to make solves
        // work correctly. For saved cases with unchanged defaults,
        // limiting the two offending variables to the maturity
        // duration produces a result consonant with the palpable
        // intention of the quondam defaults.
        //
        SolveTargetYear = std::min(years_to_maturity(), SolveTargetTime.value());
        SolveBeginYear  =                               SolveBeginTime .value() ;
        SolveEndYear    = std::min(years_to_maturity(), SolveEndTime   .value());

        SolveTargetTime = issue_age() + SolveTargetYear.value();
        SolveBeginTime  = issue_age() + SolveBeginYear .value();
        SolveEndTime    = issue_age() + SolveEndYear   .value();
        }

    if(file_version < 5)
        {
        InforceAsOfDate = add_years_and_months
            (EffectiveDate.value()
            ,InforceYear  .value()
            ,InforceMonth .value()
            ,true
            );
        LastMaterialChangeDate = add_years_and_months
            (EffectiveDate.value()
            ,InforceYear  .value() - InforceContractYear .value()
            ,InforceMonth .value() - InforceContractMonth.value()
            ,true
            );
        }
}

/// Perform any required after-the-fact fixup.

void Input::redintegrate_ad_terminum()
{
    if(EffectiveDateToday.value() && !global_settings::instance().regression_testing())
        {
        EffectiveDate = calendar_date();
        }
}

