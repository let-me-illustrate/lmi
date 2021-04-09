// MVC Model for life-insurance illustrations: xml I/O.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "input.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "global_settings.hpp"
#include "map_lookup.hpp"
#include "oecumenic_enumerations.hpp"
#include "value_cast.hpp"

#include <algorithm>                    // min()
#include <stdexcept>
#include <utility>                      // pair

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

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20050114T1947Z
/// version 2: 20080813T0131Z
/// version 3: 20090302T0509Z [see important note below]
/// version 4: 20090330T0137Z
/// version 5: 20090526T1331Z
/// version 6: 20100719T1349Z
/// version 7: 20120808T2130Z
/// version 8: 20150316T0422Z
/// version 9: 20180628T1936Z
///
/// Important note concerning version 3. On or about 20090311, some
/// end users were given an off-cycle release that should have used
/// code tagged 'lmi-20090223T2040Z', but erroneously used an untagged
/// (and untested) version of HEAD.

int Input::class_version() const
{
    return 9;
}

std::string const& Input::xml_root_name() const
{
    static std::string const s("cell");
    return s;
}

/// See this function's general documentation in the base class.
///
/// No xml file written by lmi ever contained 'FilingApprovalState'.
/// It appears only in certain admin-system extracts, as an alias for
/// 'StateOfJurisdiction'. That alias was chosen to favor unambiguity
/// over clarity.

bool Input::is_detritus(std::string const& s) const
{
    static std::vector<std::string> const v
        {"AgentFirstName"                   // Single name instead.
        ,"AgentLastName"                    // Single name instead.
        ,"AgentMiddleName"                  // Single name instead.
        ,"AssumedCaseNumberOfLives"         // Withdrawn.
        ,"CaseAssumedAssets"                // Withdrawn.
        ,"CorporationPremiumTableNumber"    // Never implemented.
        ,"CorporationTaxpayerId"            // Would violate privacy.
        ,"CurrentCoiGrading"                // Withdrawn.
        ,"DateOfRetirement"                 // Withdrawn.
        ,"DeathBenefitOptionFromIssue"      // Withdrawn.
        ,"DeathBenefitOptionFromRetirement" // Withdrawn.
        ,"DeprecatedSolveFromWhich"         // Renamed (without 'Deprecated'-).
        ,"DeprecatedSolveTgtAtWhich"        // Renamed (without 'Deprecated'-).
        ,"DeprecatedSolveToWhich"           // Renamed (without 'Deprecated'-).
        ,"DeprecatedUseDOB"                 // Renamed (without 'Deprecated'-).
        ,"DeprecatedUseDOR"                 // Withdrawn.
        ,"ExperienceRatingInitialKFactor"   // Withdrawn.
        ,"ExperienceReserveRate"            // Withdrawn.
        ,"External1035ExchangeBasis"        // Renamed to 'External1035ExchangeTaxBasis'.
        ,"FilingApprovalState"              // Alias for 'StateOfJurisdiction'.
        ,"FirstName"                        // Single name instead.
        ,"Franchise"                        // Renamed to 'MasterContractNumber'.
        ,"IndividualPaymentAmount"          // Withdrawn.
        ,"IndividualPaymentMode"            // Withdrawn.
        ,"IndividualPaymentToAge"           // Withdrawn.
        ,"IndividualPaymentToAlternative"   // Withdrawn.
        ,"IndividualPaymentToDuration"      // Withdrawn.
        ,"InforceCumulativePayments"        // Renamed to 'InforceCumulativeNoLapsePayments'.
        ,"InforceDataSource"                // Supplanted by 'data_source' attribute.
        ,"InforceDcvDeathBenefit"           // Misbegotten.
        ,"InforceExperienceReserve"         // Renamed before implementation.
        ,"InforceNetExperienceReserve"      // Withdrawn.
        ,"InforceYtdNetCoiCharge"           // Withdrawn.
        ,"InsuredPremiumTableNumber"        // Never implemented.
        ,"Internal1035ExchangeBasis"        // Renamed to 'Internal1035ExchangeTaxBasis'.
        ,"LastName"                         // Single name instead.
        ,"LoanAmount"                       // Withdrawn.
        ,"LoanFromAge"                      // Withdrawn.
        ,"LoanFromAlternative"              // Withdrawn.
        ,"LoanFromDuration"                 // Withdrawn.
        ,"LoanToAge"                        // Withdrawn.
        ,"LoanToAlternative"                // Withdrawn.
        ,"LoanToDuration"                   // Withdrawn.
        ,"MiddleName"                       // Single name instead.
        ,"NetMortalityChargeHistory"        // Renamed before implementation.
        ,"OffshoreCorridorFactor"           // Withdrawn.
        ,"OverrideExperienceReserveRate"    // Withdrawn.
        ,"PartialMortalityTable"            // Never implemented.
        ,"PayLoanInterestInCash"            // Never implemented.
        ,"PolicyDate"                       // Never implemented.
        ,"PolicyLevelFlatExtra"             // Never implemented; poor name.
        ,"PolicyNumber"                     // Renamed to 'ContractNumber'.
        ,"PremiumHistory"                   // Renamed to 'Inforce7702AAmountsPaidHistory'.
        ,"SocialSecurityNumber"             // Withdrawn: would violate privacy.
        ,"SolveBasis"                       // Renamed to 'SolveExpenseGeneralAccountBasis'.
        ,"SolveBeginTime"                   // Renamed to 'SolveBeginAge'.
        ,"SolveEndTime"                     // Renamed to 'SolveEndAge'.
        ,"SolveTargetCashSurrenderValue"    // Renamed to 'SolveTargetValue'.
        ,"SolveTargetTime"                  // Renamed to 'SolveTargetAge'.
        ,"SupplementalSpecifiedAmount"      // Renamed to 'SupplementalAmount'.
        ,"SpecamtHistory"                   // Merged into 'SpecifiedAmount'.
        ,"SpecifiedAmountFromIssue"         // Withdrawn.
        ,"SpecifiedAmountFromRetirement"    // Withdrawn.
        ,"TermProportion"                   // Disused: cf. 'TermRiderProportion'.
        ,"UseExperienceRating"              // Withdrawn.
        ,"UseOffshoreCorridorFactor"        // Withdrawn.
        ,"WithdrawalAmount"                 // Withdrawn.
        ,"WithdrawalFromAge"                // Withdrawn.
        ,"WithdrawalFromAlternative"        // Withdrawn.
        ,"WithdrawalFromDuration"           // Withdrawn.
        ,"WithdrawalToAge"                  // Withdrawn.
        ,"WithdrawalToAlternative"          // Withdrawn.
        ,"WithdrawalToDuration"             // Withdrawn.
        ,"YearsOfZeroDeaths"                // Withdrawn.
        };
    return contains(v, s);
}

// INPUT !! In 'cell.{rnc,xsd}, elements
//   ExperienceRatingInitialKFactor
//   ExperienceReserveRate
//   InforceNetExperienceReserve
//   InforceYtdNetCoiCharge
//   OverrideExperienceReserveRate
//   UseExperienceRating
// are obsolete and should be removed in a future version.

void Input::redintegrate_ex_ante
    (int                file_version
    ,std::string const& name
    ,std::string      & value
    ) const
{
    if(class_version() == file_version)
        {
        // INPUT !! Next time the class version is updated,
        // remove this 'goto' and its label.
        goto jumper;
        return;
        }

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
            value = "Allow MEC";
            }
        if("DeprecatedSolveTgtAtWhich" == name)
            {
            value =
                  ("TgtAtRetirement" == value) ? "Retirement"
                : ("TgtAtYear"       == value) ? "Year"
                : ("TgtAtAge"        == value) ? "Age"
                : ("TgtAtMaturity"   == value) ? "Maturity"
                : throw std::runtime_error(value + ": unexpected solve-at duration.")
                ;
            }
        if("DeprecatedSolveFromWhich" == name)
            {
            value =
                  ("FromIssue"      == value) ? "Issue"
                : ("FromYear"       == value) ? "Year"
                : ("FromAge"        == value) ? "Age"
                : ("FromRetirement" == value) ? "Retirement"
                : throw std::runtime_error(value + ": unexpected solve-from duration.")
                ;
            }
        if("DeprecatedSolveToWhich" == name)
            {
            value =
                  ("ToRetirement"    == value) ? "Retirement"
                : ("ToYear"          == value) ? "Year"
                : ("ToAge"           == value) ? "Age"
                : ("ToMaturity"      == value) ? "Maturity"
                : throw std::runtime_error(value + ": unexpected solve-to duration.")
                ;
            }
        if("SolveBasis" == name)
            {
            value =
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
            value =
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
            value =
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
            value =
                  ("SolveForEndt"     == value) ? "Endowment"
                : ("SolveForTarget"   == value) ? "Target CSV"
                : ("SolveForTaxBasis" == value) ? "CSV = tax basis"
                : ("SolveForNonMec"   == value) ? "Avoid MEC"
                : throw std::runtime_error(value + ": unexpected solve goal.")
                ;
            }

        if("GeneralAccountRateType" == name)
            {
            value =
                  ("CredRate"  == value) ? "Credited rate"
                : ("NetRate"   == value) ? "Credited rate"
                : ("GrossRate" == value) ? "Earned rate"
                : throw std::runtime_error(value + ": unexpected general-account type.")
                ;
            }

        if("SeparateAccountRateType" == name)
            {
            value =
                  ("CredRate"  == value) ? "Net rate"
                : ("NetRate"   == value) ? "Net rate"
                : ("GrossRate" == value) ? "Gross rate"
                : throw std::runtime_error(value + ": unexpected separate-account type.")
                ;
            }

        if("LoanRateType" == name)
            {
            value =
                  ("Fixed" == value) ? "Fixed loan rate"
                : ("VLR"   == value) ? "Variable loan rate"
                : throw std::runtime_error(value + ": unexpected loan-rate type.")
                ;
            }
        }

    if(file_version < 8)
        {
        // Prior to 2018-06, 'InputFundManagementFee' was entered in
        // basis points, rather than as a pure number. Presumably few
        // if any new private-placement input files have been saved
        // since version 8's inception; for any that have, this field
        // will be limited to unity, and any saved value of one basis
        // point or greater will therefore be translated to 10000 bp,
        // resulting in a -100% net rate and a noticeably conservative
        // separate-account value.
        if("InputFundManagementFee" == name)
            {
            value = value_cast<std::string>(value_cast<double>(value) / 10000.0);
            }
        }

    if(file_version < 10)
        {
  jumper:
        // Prior to 2019-01-30, weird "Zero" columns were available
        // for some supplemental reports. The manual report that used
        // them having been automated, they've been withdrawn. For
        // backward compatibility, they're silently ignored wherever
        // they occurred in old input files.
        if
            (  contains(name, "SupplementalReportColumn")
            && contains(value, "Zero")
            )
            {
            value = "[none]";
            }
        // Prior to 2021-01-31, experience-rating columns were offered
        // for some supplemental reports, but have been withdrawn. For
        // backward compatibility, they're silently ignored wherever
        // they occurred in old input files.
        if
            (   contains(name, "SupplementalReportColumn")
            &&
                (  contains(value, "ExperienceReserve")
                || contains(value, "NetCOICharge")
                || contains(value, "ProjectedCoiCharge")
                || contains(value, "KFactor")
                )
            )
            {
            value = "[none]";
            }
        }
}

void Input::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& detritus_map
    ,std::list<std::string>             const& residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    if(file_version < 9)
        {
        // Version 9 renamed these elements. Because they are used
        // soon hereafter, they must be renamed early--between
        // redintegrate_ex_ante() and redintegrate_ex_post(),
        // as it were.
        LMI_ASSERT(contains(residuary_names, "SolveTargetAge"));
        LMI_ASSERT(contains(residuary_names, "SolveBeginAge"));
        LMI_ASSERT(contains(residuary_names, "SolveEndAge"));
        if(contains(detritus_map, "SolveTargetTime"))
            {
            SolveTargetAge = map_lookup(detritus_map, "SolveTargetTime");
            }
        if(contains(detritus_map, "SolveBeginTime"))
            {
            SolveBeginAge  = map_lookup(detritus_map, "SolveBeginTime");
            }
        if(contains(detritus_map, "SolveEndTime"))
            {
            SolveEndAge    = map_lookup(detritus_map, "SolveEndTime");
            }
        }

    if(0 == file_version)
        {
        // An older version with no distinct 'file_version' didn't
        // have 'DefinitionOfMaterialChange', whose default value is
        // unacceptable for GPT.
        if(contains(residuary_names, "DefinitionOfMaterialChange"))
            {
            DefinitionOfMaterialChange =
                mce_gpt == DefinitionOfLifeInsurance
                ? mce_adjustment_event
                : mce_earlier_of_increase_or_unnecessary_premium
                ;
            }

        operator[]("AgentName"  ) = full_name
            (map_lookup(detritus_map, "AgentFirstName" )
            ,map_lookup(detritus_map, "AgentMiddleName")
            ,map_lookup(detritus_map, "AgentLastName"  )
            );
        operator[]("InsuredName") = full_name
            (map_lookup(detritus_map, "FirstName"      )
            ,map_lookup(detritus_map, "MiddleName"     )
            ,map_lookup(detritus_map, "LastName"       )
            );
        }

    if(file_version < 2)
        {
        // 'UseCurrentDeclaredRate' was introduced 20071017T1454Z; its
        // default value of "Yes" would break backward compatibility.
        if(contains(residuary_names, "UseCurrentDeclaredRate"))
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
        //   SolveTargetTime [since renamed to SolveTargetAge]
        //   SolveEndTime    [since renamed to SolveEndAge]
        // didn't work correctly with contemporary versions of the
        // program. Users had to change them in order to make solves
        // work correctly. For saved cases with unchanged defaults,
        // limiting the two offending variables to the maturity
        // duration produces a result consonant with the palpable
        // intention of the quondam defaults.
        //
        SolveTargetYear = std::min(years_to_maturity(), SolveTargetAge.value());
        SolveBeginYear  =                               SolveBeginAge .value() ;
        SolveEndYear    = std::min(years_to_maturity(), SolveEndAge   .value());

        SolveTargetAge = issue_age() + SolveTargetYear.value();
        SolveBeginAge  = issue_age() + SolveBeginYear .value();
        SolveEndAge    = issue_age() + SolveEndYear   .value();
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

    // Defectively, some admin-system extracts lack various elements.
    // All deficient extracts happen to lack <AgentPhone>: they don't
    // even have an empty <AgentPhone/>; but that field is present in
    // every input file ever saved by any version of lmi.
    bool const deficient_extract = contains(residuary_names, "AgentPhone");

    // One state governs everything except premium tax, which may be
    // paid to a different state, e.g. on cases with more than five
    // hundred lives with a common (employer) issue state; see:
    //   http://www.naic.org/documents/frs_summit_presentations_03.pdf
    //   http://www.naic.org/documents/committees_e_app_blanks_adopted_2007-42BWG_Modified.pdf
    // Prior to version 6, a single state was used for all purposes,
    // and it was determined as either employee or employer state,
    // depending on the value of DB_PremTaxState. The determination is
    // complex (what if a case was issued with 498 lives and then
    // grew to 502?), and starting with version 6 both states are
    // input fields.
    //
    // 'StateOfJurisdiction' has always been in lmi, but was never
    // meaningfully used prior to version 6.
    //
    // 'FilingApprovalState' and 'PremiumTaxState' were unknown before
    // version 6, and would not ordinarily occur in older versions.
    // However, certain admin-system extracts that are always marked
    // as version 5 were to have been modified to add these two fields
    // (yet were not so modified).
    if(file_version < 6)
        {
        bool const b0 = contains(detritus_map, "FilingApprovalState");
        bool const b1 = !contains(residuary_names, "PremiumTaxState");
        if(b0 || b1)
            {
            alarum() << "Unexpected 'state' fields." << LMI_FLUSH; // deficient_extract
            LMI_ASSERT(b0 && b1 && 5 == file_version);
            StateOfJurisdiction = map_lookup(detritus_map, "FilingApprovalState");
            }
        else
            {
            product_database db
                (ProductName          .value()
                ,Gender               .value()
                ,UnderwritingClass    .value()
                ,Smoking              .value()
                ,IssueAge             .value()
                ,GroupUnderwritingType.value()
                ,mce_s_CT // Dummy initialization.
                );

            // Deemed state must not depend on itself.
            if(db.varies_by_state(DB_PremTaxState))
                {
                alarum()
                    << "Database invalid: circular dependency."
                    << " State of jurisdiction depends on itself."
                    << LMI_FLUSH
                    ;
                }

            switch(db.query<int>(DB_PremTaxState))
                {
                case oe_ee_state:
                    {
                    StateOfJurisdiction = State;
                    PremiumTaxState     = State;
                    }
                    break;
                case oe_er_state:
                    {
                    StateOfJurisdiction = CorporationState;
                    PremiumTaxState     = CorporationState;
                    }
                    break;
                default:
                    {
                    alarum()
                        << "Cannot determine state of jurisdiction."
                        << LMI_FLUSH
                        ;
                    }
                    break;
                }
            }
        }

    // For "deficient" extracts, these required elements are missing.
    if(file_version < 6 && !deficient_extract)
        {
        LMI_ASSERT(contains(residuary_names, "SolveFromWhich"));
        LMI_ASSERT(contains(residuary_names, "SolveTgtAtWhich"));
        LMI_ASSERT(contains(residuary_names, "SolveToWhich"));
        LMI_ASSERT(contains(residuary_names, "UseDOB"));
        SolveFromWhich  = map_lookup(detritus_map, "DeprecatedSolveFromWhich");
        SolveTgtAtWhich = map_lookup(detritus_map, "DeprecatedSolveTgtAtWhich");
        SolveToWhich    = map_lookup(detritus_map, "DeprecatedSolveToWhich");
        UseDOB          = map_lookup(detritus_map, "DeprecatedUseDOB");
        }

    if(file_version < 7)
        {
        // Version 7 renamed these elements.
        LMI_ASSERT(contains(residuary_names, "ContractNumber"));
        LMI_ASSERT(contains(residuary_names, "External1035ExchangeTaxBasis"));
        LMI_ASSERT(contains(residuary_names, "Inforce7702AAmountsPaidHistory"));
        LMI_ASSERT(contains(residuary_names, "InforceCumulativeNoLapsePayments"));
        LMI_ASSERT(contains(residuary_names, "Internal1035ExchangeTaxBasis"));
        LMI_ASSERT(contains(residuary_names, "MasterContractNumber"));
        LMI_ASSERT(contains(residuary_names, "SolveExpenseGeneralAccountBasis"));
        ContractNumber                   = map_lookup(detritus_map, "PolicyNumber");
        External1035ExchangeTaxBasis     = map_lookup(detritus_map, "External1035ExchangeBasis");
        InforceCumulativeNoLapsePayments = map_lookup(detritus_map, "InforceCumulativePayments");
        Internal1035ExchangeTaxBasis     = map_lookup(detritus_map, "Internal1035ExchangeBasis");
        MasterContractNumber             = map_lookup(detritus_map, "Franchise");
        // Some (but not all) variants of version 0 lacked
        // 'PremiumHistory'; all later versions should include it.
        if(!contains(detritus_map, "PremiumHistory"))
            {
            LMI_ASSERT(0 == file_version);
            }
        else
            {
            Inforce7702AAmountsPaidHistory  = map_lookup(detritus_map, "PremiumHistory");
            }
        // "Deficient" extracts lack 'SolveBasis'.
        if(!deficient_extract)
            {
            SolveExpenseGeneralAccountBasis = map_lookup(detritus_map, "SolveBasis");
            }
        }

    if(file_version < 7)
        {
        // Prior to version 7, 'InforceCumulativePayments' was used
        // for no-lapse, GPT, and ROP, so set them all equal here
        // for backward compatibility. This matters little for new
        // business, but is so cheap that it may as well be done
        // unconditionally.
        InforceCumulativeGptPremiumsPaid = InforceCumulativeNoLapsePayments.value();
        InforceCumulativeRopPayments     = InforceCumulativeNoLapsePayments.value();
        }

    if(file_version < 7 && contains(detritus_map, "SpecamtHistory"))
        {
        // Merge obsolete 'SpecamtHistory' into 'SpecifiedAmount'.
        //
        // Prior to version 7, 'SpecamtHistory' and 'SpecifiedAmount'
        // were distinct. Some version-0 files had the history entity,
        // but others did not; if it's not present, then of course it
        // cannot be merged.
        //
        // Function must_overwrite_specamt_with_obsolete_history(),
        // called below, requires 'InforceYear' and 'InforceMonth',
        // which some "deficient" extracts omit. DoTransmogrify()
        // sets those members downstream (at least before any
        // illustration is produced), but the "obsolete history"
        // function needs them now. This requires version 5, which
        // introduced 'InforceAsOfDate'; no "deficient" extract
        // should have an earlier version.
        //
        // set_inforce_durations_from_dates() does the same and more;
        // but, because it does more, it can't simply be called here.
        if(deficient_extract && EffectiveDate.value() != InforceAsOfDate.value())
            {
            LMI_ASSERT(4 < file_version);
            std::pair<int,int> ym0 = years_and_months_since
                (EffectiveDate  .value()
                ,InforceAsOfDate.value()
                ,true
                );
            InforceYear  = ym0.first;
            InforceMonth = ym0.second;
            }
        // Requiring 'deficient_extract' here wouldn't be right,
        // because an extract file that has been modified and saved
        // is no longer detectably "deficient".
        //
        // This idiom for distinguishing inforce from new business is
        // avoided in new code, but retained in backward-compatibility
        // code that, by its nature, should rarely be modernized.
        if(0 != InforceYear || 0 != InforceMonth)
            {
            if
                (must_overwrite_specamt_with_obsolete_history
                    (SpecifiedAmount.value()
                    ,map_lookup(detritus_map, "SpecamtHistory")
                    )
                )
                {
                SpecifiedAmount = map_lookup(detritus_map, "SpecamtHistory");
                }
            }
        }

    if(file_version < 7 && EffectiveDate.value() != InforceAsOfDate.value())
        {
        // Version 7 introduced 'InforceSpecAmtLoadBase'; previously,
        // the first element of 'SpecamtHistory' had been used in its
        // place, which would have disregarded the load limit as well
        // as any term rider.

        // This product_database object could be constructed outside
        // this inner block if it ever becomes useful for any other
        // purpose. It is not obvious whether it can be merged with
        // the earlier instantiation in this function, which uses a
        // dummy value for StateOfJurisdiction.
        product_database db
            (ProductName          .value()
            ,Gender               .value()
            ,UnderwritingClass    .value()
            ,Smoking              .value()
            ,IssueAge             .value()
            ,GroupUnderwritingType.value()
            ,StateOfJurisdiction  .value()
            );
        RealizeSpecifiedAmount();
        InforceSpecAmtLoadBase = std::min
            (TermRiderAmount.value() + SpecifiedAmountRealized_[0].value()
            ,db.query<double>(DB_SpecAmtLoadLimit)
            );
        }

    if(file_version < 9)
        {
        // Version 9 renamed these elements.
        // This one has existed since before the lmi epoch, but some
        // old vendor extracts defectively failed to provide it.
        LMI_ASSERT(contains(residuary_names, "SolveTargetValue"));
        if(!contains(detritus_map, "SolveTargetCashSurrenderValue"))
            {
            LMI_ASSERT(file_version < 7);
            }
        else
            {
            SolveTargetValue = map_lookup(detritus_map, "SolveTargetCashSurrenderValue");
            }
        // This one has existed only since version 8.
        LMI_ASSERT(contains(residuary_names, "SupplementalAmount"));
        SupplementalAmount =
            (file_version < 8)
            ? ""
            : map_lookup(detritus_map, "SupplementalSpecifiedAmount")
            ;
        }
}

void Input::redintegrate_ad_terminum()
{
    if(EffectiveDateToday.value() && !global_settings::instance().regression_testing())
        {
        EffectiveDate = calendar_date();
        }
}
