// GPT server.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

// Known defects:
// grep for "NEED DECISION"

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_server7702.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "fenv_lmi.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_server7702io.hpp"
#include "ihs_x_type.hpp"

#include <exception>

//============================================================================
void EnterServer()
{
    fenv_initialize();
}

//============================================================================
Server7702Output RunServer7702FromStruct(Server7702Input a_Input)
{
    EnterServer();
    Server7702 contract(a_Input);
    contract.Process();
    return contract.GetOutput();
}

//============================================================================
Server7702::Server7702(Server7702Input& a_Input)
    :Input(a_Input)
    ,IsIssuedToday(false)
    ,IsPossibleAdjustableEvent(false)
{
}

//============================================================================
void Server7702::VerifyPrecision() const
{
    if(!fenv_validate())
        {
        throw server7702_precision_changed
            ("Floating-point precision changed: results are invalid."
            );
        }
}

//============================================================================
// Attempt processing, catch any exceptions, and emit output record
//
// I'm aware that many programmers would prefer to reserve C++
// exceptions for more catastrophic events, and some older textbooks
// would regard these exceptions as not sufficiently "exceptional."
// I have come around to another school of thought, which has as
// adherents some (not all) of the best programmers, and holds that
// exceptions may be used when a condition prevents further processing
// of the current task and it is necessary to jump directly to a
// possibly remote location to resume.
void Server7702::Process()
{
    try
        {
        PerformProcessing();
        VerifyPrecision();
        }
    catch(server7702_precision_changed const& e)
        {
        // TODO ?? Perhaps the control word should be changed and
        // processing restarted.
        Output.Status |= precision_changed;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(server7702_implausible_input const& e)
        {
        Output.Status |= implausible_input;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(server7702_inconsistent_input const& e)
        {
        Output.Status |= inconsistent_input;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(x_product_rule_violated const& e)
        {
        Output.Status |= product_rule_violated;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(server7702_adjustable_event_forbidden_at_issue const& e)
        {
        Output.Status |= adjustable_event_forbidden_at_issue;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(server7702_guideline_negative const& e)
        {
        Output.Status |= guideline_negative;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(server7702_misstatement_of_age_or_gender const& e)
        {
        Output.Status |= misstatement_of_age_or_gender;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(std::range_error const& e)
        {
        Output.Status |= implausible_input; // TODO ?? can we be more specific?
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }

    // Unknown error
    catch(std::exception const& e)
        {
        Output.Status |= unknown_error;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        // Since we don't know what the error is, we propagate
        // it back to the caller; we put a message on standard error,
        // but don't try to emit anything to standard output.
        throw;
        }
}

//============================================================================
void Server7702::PerformProcessing()
{
    Output.ContractNumber                   = Input.ContractNumber;
    Output.Status                           = 0;
    Output.AdjustableEventOccurred          = false;
    Output.GuidelineLevelPremium            = Input.InforceGlp;
    Output.GuidelineSinglePremium           = Input.InforceGsp;
    Output.GuidelineLevelPremiumPolicyA     = 0.0;
    Output.GuidelineSinglePremiumPolicyA    = 0.0;
    Output.GuidelineLevelPremiumPolicyB     = 0.0;
    Output.GuidelineSinglePremiumPolicyB    = 0.0;
    Output.GuidelineLevelPremiumPolicyC     = 0.0;
    Output.GuidelineSinglePremiumPolicyC    = 0.0;

    VerifyPlausibilityOfInput();

    DecideWhatToCalculate();

    if(IsIssuedToday)
        {
        ProcessNewIssue();
        }
    if(IsPossibleAdjustableEvent)
        {
        ProcessAdjustableEvent();
        }
}

//============================================================================
// Other conditions are tested elsewhere.
// TODO ?? We can add many similar conditions here.
void Server7702::VerifyPlausibilityOfInput() const
{
    if(Input.OldIssueAge < 0)
        {
        throw server7702_implausible_input
            (
            "Old issue age less than zero"
            );
        }
    if(99 < Input.OldIssueAge)
        {
        throw server7702_implausible_input
            (
            "Old issue age greater than 99"
            );
        }
    if(Input.NewIssueAge < 0)
        {
        throw server7702_implausible_input
            (
            "New issue age less than zero"
            );
        }
    if(99 < Input.NewIssueAge)
        {
        throw server7702_implausible_input
            (
            "New issue age greater than 99"
            );
        }
    if(Input.NewIssueAge != Input.OldIssueAge)
        {
        throw server7702_misstatement_of_age_or_gender
            (
            "New issue age different from old issue age"
            );
        }
    if(Input.NewGender != Input.OldGender)
        {
        throw server7702_misstatement_of_age_or_gender
            (
            "New gender different from old gender"
            );
        }
}

//============================================================================
void Server7702::DecideWhatToCalculate()
{
    // TODO ?? Is this not superfluous?
    if
        (
            Input.NewIssueAge               != Input.OldIssueAge
        ||  Input.NewGender                 != Input.OldGender
        )
        {
        // Consider change of insured as a reissue that probably violates.
        throw server7702_misstatement_of_age_or_gender
            (
            "Probable irremediable violation--consult actuarial department"
            );
        }

    // Casual, but strictly correct for all testdeck cases:
    IsIssuedToday = 0 == Input.InforceYear;

    IsPossibleAdjustableEvent =
// TODO ?? Why treat a taxable withdrawal as an adjustment event?
//            0.0                             != Input.PremsPaidDecrement
            Input.NewDbo                    != Input.OldDbo
        ||  (   Input.NewSpecAmt            != Input.OldSpecAmt
            &&  Input.NewBenefitAmount      != Input.OldBenefitAmount
            )
// TODO ?? NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewQabTermAmt             != Input.OldQabTermAmt
// TODO ?? No adj event if term and SA change but DB remains constant, but
// TODO ?? NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewSmoking                != Input.OldSmoking
// 7702 mortality basis is the same for preferred vs. standard
// Assume nothing else (e.g. loads) varies by that either
//      ||  Input.NewUnderwritingClass      != Input.OldUnderwritingClass
// Assume state premium tax passthrough is ignored for GPT
//      ||  Input.NewStateOfJurisdiction    != Input.OldStateOfJurisdiction
// Assume WP is completely ignored
//      ||  Input.NewWaiverOfPremiumInForce != Input.OldWaiverOfPremiumInForce
//      ||  Input.NewWaiverOfPremiumRating  != Input.OldWaiverOfPremiumRating
        ||  Input.NewAccidentalDeathInForce != Input.OldAccidentalDeathInForce
// Assume ADD rating is ignored
//      ||  Input.NewAccidentalDeathRating  != Input.OldAccidentalDeathRating
// Assume table rating is ignored
//      ||  Input.NewSubstandardTable       != Input.OldSubstandardTable
// Assume flat extras are ignored
//      ||  Input.NewFlatExtra              != Input.OldFlatExtra
        ;

    if(IsIssuedToday && IsPossibleAdjustableEvent)
        {
        throw server7702_adjustable_event_forbidden_at_issue("");
        }
}

//============================================================================
// Set GLP and GSP at issue afer validating input.
void Server7702::ProcessNewIssue()
{
    bool okay =
            Input.InforceYear               == 0
        &&  Input.InforceGlp                == 0.0
        &&  Input.InforceGsp                == 0.0
        &&  Input.OldGender                 == Input.NewGender
        &&  Input.OldUnderwritingClass      == Input.NewUnderwritingClass
        &&  Input.OldSmoking                == Input.NewSmoking
        &&  Input.OldIssueAge               == Input.NewIssueAge
        &&  Input.OldStateOfJurisdiction    == Input.NewStateOfJurisdiction
        &&  Input.OldSpecAmt                == Input.NewSpecAmt
        &&  Input.OldDbo                    == Input.NewDbo
        ;

    // TODO ?? It would be better to spell them all out.
    if(!okay)
        {
        throw server7702_inconsistent_input
            ("A precondition for issuing a new contract was not satisfied"
            );
        }

    SetDoleBentsenValuesA();
    Output.GuidelineLevelPremium    = Output.GuidelineLevelPremiumPolicyA;
    Output.GuidelineSinglePremium   = Output.GuidelineSinglePremiumPolicyA;
// TODO ?? NEED DECISION Only if an adj event really occurred, as defined.
    LMI_ASSERT(false == Output.AdjustableEventOccurred);
}

//============================================================================
// Set new GLP and GSP following an adjustable event, after validating input.
void Server7702::ProcessAdjustableEvent()
{
// TODO ??  Input.OldBenefitAmount = ?;
//  GuidelineLevelPremium
//  GuidelineSinglePremium

    // ? Is this not superfluous?
    if
        (
            Input.NewIssueAge               != Input.OldIssueAge
        ||  Input.NewGender                 != Input.OldGender
        )
        {
        throw std::logic_error
            (
            "Untrapped misstatement of age or gender"
            );
        }

    Output.GuidelineLevelPremiumPolicyA  = Input.InforceGlp;
    Output.GuidelineSinglePremiumPolicyA = Input.InforceGsp;

    SetDoleBentsenValuesBC();
    Output.GuidelineLevelPremium =
            Output.GuidelineLevelPremiumPolicyA
        +   Output.GuidelineLevelPremiumPolicyB
        -   Output.GuidelineLevelPremiumPolicyC
        ;
    Output.GuidelineSinglePremium =
            Output.GuidelineSinglePremiumPolicyA
        +   Output.GuidelineSinglePremiumPolicyB
        -   Output.GuidelineSinglePremiumPolicyC
        ;
    Output.AdjustableEventOccurred  = true;
}

//============================================================================
void Server7702::SetDoleBentsenValuesA()
{
    BasicValues basic_values_A
        (Input.ProductName
        ,Input.OldGender                .value()
        ,Input.OldUnderwritingClass     .value()
        ,Input.OldSmoking               .value()
        ,Input.OldIssueAge
        ,Input.GroupUnderwritingType    .value()
        ,Input.OldStateOfJurisdiction   .value()
        ,Input.OldSpecAmt
        ,Input.OldDbo                   .value()
        ,Input.OldAccidentalDeathInForce
        ,Input.OldTarget
        );

    Output.GuidelineLevelPremiumPolicyA = basic_values_A.Irc7702_->CalculateGLP
        (0
        ,Input.OldBenefitAmount
        ,Input.OldSpecAmt
        ,Input.OldSpecAmt
        ,Input.OldDbo.value()
        );

    Output.GuidelineSinglePremiumPolicyA = basic_values_A.Irc7702_->CalculateGSP
        (0
        ,Input.OldBenefitAmount
        ,Input.OldSpecAmt
        ,Input.OldSpecAmt
        );
}

//============================================================================
void Server7702::SetDoleBentsenValuesBC()
{
    BasicValues basic_values_B
        (Input.ProductName
        ,Input.NewGender                .value()
        ,Input.NewUnderwritingClass     .value()
        ,Input.NewSmoking               .value()
        ,Input.NewIssueAge
        ,Input.GroupUnderwritingType    .value()
        ,Input.NewStateOfJurisdiction   .value()
        ,Input.NewSpecAmt
        ,Input.NewDbo                   .value()
        ,Input.NewAccidentalDeathInForce
        ,Input.NewTarget
        );

    Output.GuidelineLevelPremiumPolicyB = basic_values_B.Irc7702_->CalculateGLP
        (Input.InforceYear
        ,Input.NewBenefitAmount
        ,Input.NewSpecAmt
        ,Input.NewSpecAmt
        ,Input.NewDbo.value()
        );

    Output.GuidelineSinglePremiumPolicyB = basic_values_B.Irc7702_->CalculateGSP
        (Input.InforceYear
        ,Input.NewBenefitAmount
        ,Input.NewSpecAmt
        ,Input.NewSpecAmt
        );

    BasicValues basic_values_C
        (Input.ProductName
        ,Input.OldGender                .value()
        ,Input.OldUnderwritingClass     .value()
        ,Input.OldSmoking               .value()
        ,Input.OldIssueAge
        ,Input.GroupUnderwritingType    .value()
        ,Input.OldStateOfJurisdiction   .value()
        ,Input.OldSpecAmt
        ,Input.OldDbo                   .value()
        ,Input.OldAccidentalDeathInForce
        ,Input.OldTarget
        );

    Output.GuidelineLevelPremiumPolicyC = basic_values_C.Irc7702_->CalculateGLP
        (Input.InforceYear
        ,Input.OldBenefitAmount
        ,Input.OldSpecAmt
        ,Input.OldSpecAmt
        ,Input.OldDbo.value()
        );

    Output.GuidelineSinglePremiumPolicyC = basic_values_C.Irc7702_->CalculateGSP
        (Input.InforceYear
        ,Input.OldBenefitAmount
        ,Input.OldSpecAmt
        ,Input.OldSpecAmt
        );
}

