// GPT server.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// Known defects:
// grep for "NEED DECISION"

#include "pchfile.hpp"

#include "ihs_server7702.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "fenv_lmi.hpp"
#include "gpt_input.hpp"
#include "ihs_irc7702.hpp"

#include <exception>

enum
    {unknown_error                          = 0x0001
    ,precision_changed                      = 0x0002
    ,implausible_input                      = 0x0004
    ,inconsistent_input                     = 0x0008
    ,adjustable_event_forbidden_at_issue    = 0x0010
    ,guideline_negative                     = 0x0020
    ,misstatement_of_age_or_gender          = 0x0040
    ,range_error                            = 0x0080
    ,runtime_error                          = 0x0100
    };

//============================================================================
void EnterServer()
{
    fenv_initialize();
}

//============================================================================
Server7702Output RunServer7702FromStruct(gpt_input a_Input)
{
    EnterServer();
    Server7702 contract(a_Input);
    contract.Process();
    return contract.GetOutput();
}

//============================================================================
Server7702::Server7702(gpt_input& a_Input)
    :Input                     {a_Input}
    ,IsIssuedToday             {false}
    ,IsPossibleAdjustableEvent {false}
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
        // If an x87 build of a GPT server were to be released, then
        // perhaps the control word should be changed and processing
        // restarted.
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
        Output.Status |= range_error;
        warning() << Output.ContractNumber << " error: " << e.what() << LMI_FLUSH;
        }
    catch(std::runtime_error const& e)
        {
        Output.Status |= runtime_error;
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
    Output.ContractNumber                   = Input.ContractNumber.value();
    Output.Status                           = 0;
    Output.AdjustableEventOccurred          = false;
    Output.GuidelineLevelPremium            = Input.InforceGlp    .value();
    Output.GuidelineSinglePremium           = Input.InforceGsp    .value();
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
void Server7702::VerifyPlausibilityOfInput() const
{
    if(Input.IssueAge.value() < 0)
        {
        throw server7702_implausible_input
            (
            "Issue age less than zero"
            );
        }
    if(99 < Input.IssueAge.value())
        {
        throw server7702_implausible_input
            (
            "Issue age greater than 99"
            );
        }
//  if(Input.NewIssueAge != Input.OldIssueAge) // Not differentiated.
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
    // TAXATION !! Is this not superfluous?
    if
        (
//          Input.NewIssueAge               != Input.OldIssueAge // Not differentiated.
            Input.NewGender                 != Input.OldGender
        )
        {
        // Consider change of insured as a reissue that probably violates.
        throw server7702_misstatement_of_age_or_gender
            (
            "Probable irremediable violation--consult actuarial department"
            );
        }

    IsIssuedToday = Input.EffectiveDate == Input.InforceAsOfDate;

    IsPossibleAdjustableEvent =
// TAXATION !! Why treat a taxable withdrawal as an adjustment event?
//            0.0                             != Input.PremsPaidDecrement
            Input.NewDbo                    != Input.OldDbo
        ||  (   Input.NewSpecAmt            != Input.OldSpecAmt
            &&  Input.NewDeathBft           != Input.OldDeathBft
            )
// TAXATION !! NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewQabTermAmt             != Input.OldQabTermAmt
// TAXATION !! No adj event if term and SA change but DB remains constant, but
// TAXATION !! NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewSmoking                != Input.OldSmoking
// 7702 mortality basis is the same for preferred vs. standard
// Assume nothing else (e.g. loads) varies by that either
//      ||  Input.NewUnderwritingClass      != Input.OldUnderwritingClass
// Assume state premium tax passthrough is ignored for GPT
//      ||  Input.NewStateOfJurisdiction    != Input.OldStateOfJurisdiction
// Assume WP is completely ignored
//      ||  Input.NewWaiverOfPremiumInForce != Input.OldWaiverOfPremiumInForce
//      ||  Input.NewWaiverOfPremiumRating  != Input.OldWaiverOfPremiumRating
// Ignore ADD for now
//      ||  Input.NewAccidentalDeathInForce != Input.OldAccidentalDeathInForce
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
        &&  Input.EffectiveDate             == Input.InforceAsOfDate
        &&  Input.InforceGlp                == 0.0
        &&  Input.InforceGsp                == 0.0
        &&  Input.OldGender                 == Input.NewGender
//      &&  Input.OldUnderwritingClass      == Input.NewUnderwritingClass   // Not differentiated.
        &&  Input.OldSmoking                == Input.NewSmoking
//      &&  Input.OldIssueAge               == Input.NewIssueAge            // Not differentiated.
//      &&  Input.OldStateOfJurisdiction    == Input.NewStateOfJurisdiction // Not differentiated.
        &&  Input.OldSpecAmt                == Input.NewSpecAmt
        &&  Input.OldDbo                    == Input.NewDbo
        ;

    if(!okay)
        {
        throw server7702_inconsistent_input
            ("A precondition for issuing a new contract was not satisfied"
            );
        }

    SetDoleBentsenValuesA();
    Output.GuidelineLevelPremium    = Output.GuidelineLevelPremiumPolicyA;
    Output.GuidelineSinglePremium   = Output.GuidelineSinglePremiumPolicyA;
// TAXATION !! NEED DECISION Only if an adj event really occurred, as defined.
    LMI_ASSERT(false == Output.AdjustableEventOccurred);
}

//============================================================================
// Set new GLP and GSP following an adjustable event, after validating input.
void Server7702::ProcessAdjustableEvent()
{
// TAXATION !!  Input.OldDeathBft = ?;
//  GuidelineLevelPremium
//  GuidelineSinglePremium

    // ? Is this not superfluous?
    if
        (
//          Input.NewIssueAge               != Input.OldIssueAge // Not differentiated.
            Input.NewGender                 != Input.OldGender
        )
        {
        throw std::logic_error
            (
            "Untrapped misstatement of age or gender"
            );
        }

    Output.GuidelineLevelPremiumPolicyA  = Input.InforceGlp.value();
    Output.GuidelineSinglePremiumPolicyA = Input.InforceGsp.value();

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
        (Input.ProductName              .value()
        ,Input.OldGender                .value()
        ,Input.UnderwritingClass        .value()
        ,Input.OldSmoking               .value()
        ,Input.IssueAge                 .value()
        ,Input.GroupUnderwritingType    .value()
        ,Input.StateOfJurisdiction      .value()
        ,Input.OldSpecAmt               .value()
        ,Input.OldDbo                   .value()
        ,false // Input.OldAccidentalDeathInForce
        ,Input.OldTarget                .value()
        );

    Output.GuidelineLevelPremiumPolicyA = basic_values_A.Irc7702_->CalculateGLP
        (0
        ,Input.OldDeathBft .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldDbo      .value()
        );

    Output.GuidelineSinglePremiumPolicyA = basic_values_A.Irc7702_->CalculateGSP
        (0
        ,Input.OldDeathBft .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldSpecAmt  .value()
        );
}

//============================================================================
void Server7702::SetDoleBentsenValuesBC()
{
    BasicValues basic_values_B
        (Input.ProductName              .value()
        ,Input.NewGender                .value()
        ,Input.UnderwritingClass        .value()
        ,Input.NewSmoking               .value()
        ,Input.IssueAge                 .value()
        ,Input.GroupUnderwritingType    .value()
        ,Input.StateOfJurisdiction      .value()
        ,Input.NewSpecAmt               .value()
        ,Input.NewDbo                   .value()
        ,false // Input.NewAccidentalDeathInForce
        ,Input.NewTarget                .value()
        );

    Output.GuidelineLevelPremiumPolicyB = basic_values_B.Irc7702_->CalculateGLP
        (Input.InforceYear .value()
        ,Input.NewDeathBft .value()
        ,Input.NewSpecAmt  .value()
        ,Input.NewSpecAmt  .value()
        ,Input.NewDbo      .value()
        );

    Output.GuidelineSinglePremiumPolicyB = basic_values_B.Irc7702_->CalculateGSP
        (Input.InforceYear .value()
        ,Input.NewDeathBft .value()
        ,Input.NewSpecAmt  .value()
        ,Input.NewSpecAmt  .value()
        );

    BasicValues basic_values_C
        (Input.ProductName              .value()
        ,Input.OldGender                .value()
        ,Input.UnderwritingClass        .value()
        ,Input.OldSmoking               .value()
        ,Input.IssueAge                 .value()
        ,Input.GroupUnderwritingType    .value()
        ,Input.StateOfJurisdiction      .value()
        ,Input.OldSpecAmt               .value()
        ,Input.OldDbo                   .value()
        ,false // Input.OldAccidentalDeathInForce
        ,Input.OldTarget                .value()
        );

    Output.GuidelineLevelPremiumPolicyC = basic_values_C.Irc7702_->CalculateGLP
        (Input.InforceYear .value()
        ,Input.OldDeathBft .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldDbo      .value()
        );

    Output.GuidelineSinglePremiumPolicyC = basic_values_C.Irc7702_->CalculateGSP
        (Input.InforceYear .value()
        ,Input.OldDeathBft .value()
        ,Input.OldSpecAmt  .value()
        ,Input.OldSpecAmt  .value()
        );
}
