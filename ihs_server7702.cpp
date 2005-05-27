// GPT server.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_server7702.cpp,v 1.4 2005-05-27 10:37:06 chicares Exp $

// Known defects:
// grep for "NEED DECISION"

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_server7702.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "calendar_date.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_resetfpu.hpp"
#include "ihs_rnddata.hpp"
#include "ihs_server7702io.hpp"
#include "ihs_x_type.hpp"

#include <cfloat>       // DBL_EPSILON
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

//============================================================================
int main()
{
    if(2.2204460492503131e-16 != DBL_EPSILON)
    {
    fatal_error()
        << "Internal validity test failed.\n Please tell the"
        << " developers to patch the compiler as described here:\n"
        << "http://www.geocrawler.com/archives/3/6013/2001/4/100/5548198/"
        << LMI_FLUSH
        ;
    throw std::logic_error();
// TODO ?? We'd like to skip the explicit message and just throw something,
// but we don't know where it would be caught for a shared library versus
// an application binary.
    }

    InitializeServer7702();
    // Read from std input, process, and write to std output
    return RunServer7702();
// TODO ?? NEED DECISION What should this return?
}

//============================================================================
// TODO ?? It seems this never gets called.
#ifndef LMI_MSW
int main()
#else // LMI_MSW
extern "C" int LMI_EXPIMP __stdcall DllEntryPoint(HINSTANCE, uint32 a_Reason, void*)
#endif // LMI_MSW
{
#ifndef LMI_MSW
    std::cout << "main() called\n";
#else // LMI_MSW
    std::cout << "DllEntryPoint() called\n";
    switch(a_Reason)
        {
        case DLL_PROCESS_ATTACH:
            {
            InitializeServer7702();
            std::cout << "DllEntryPoint() called with DLL_PROCESS_ATTACH\n";
            }
            break;
        default:
            ;   // do nothing
        }
#endif // LMI_MSW
  return true;
}

//============================================================================
void EnterServer()
{
    SetIntelDefaultNdpControlWord();
}

/* erase
//============================================================================
bool LeaveServer()
{
    volatile unsigned short int cw;
    __asm__ volatile("fstcw %0" : "=m" (*&cw));
    bool ndp_ok = IntelDefaultNdpControlWord() == cw;
    if(!ndp_ok)
        {
        std::cerr
            << "Floating point precision changed during run, presumably "
            << "by another process. Shut down other processes, "
            << "discard output, and rerun. Consider migrating to a "
            << "more reliable operating system.\n"
            ;
        }
    return ndp_ok;
}
*/

//============================================================================
// TODO ?? Should we make the directory an optional argument?
extern "C" void LMI_EXPIMP InitializeServer7702()
{
    // Data directory where tables etc. are stored
// TODO ?? This is obsolete; need a replacement. Either let main()
// take care of it, or copy main()'s initialization code here.
//    DataDir::Get("./");
}

//============================================================================
// Read from std input, process, and write to std output
int RunServer7702()
{
    EnterServer();
    // Input record (we'll use it over and over)
    Server7702Input input;
    try
        {
        while(std::cin >> input)
            {
            // Separate construction from initialization
            Server7702 contract(input);
            contract.Process();
            std::cout << contract.GetOutput();
            while('\n' == std::cin.peek())
                {
                std::cin.get();
                }
            if(std::cin.eof())
                {
                return EXIT_SUCCESS;
                }
            }
        }
    // Catch exceptions that are thrown during input
    catch(std::exception& x)
        {
        std::cerr << input.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(...)
        {
        std::cerr << "Untrapped exception" << '\n';
        }
    return EXIT_FAILURE;
}

//============================================================================
// Read from C struct, and return a different C struct
extern "C" Server7702Output LMI_EXPIMP RunServer7702FromStruct(Server7702Input a_Input)
{
    EnterServer();
    Server7702 contract(a_Input);
    contract.Process();
    return contract.GetOutput();
}

//============================================================================
// Read from C string, and put result in a C string.
// The caller must allocate sufficient space for the result; at present,
// that means 444 bytes.
extern "C" void LMI_EXPIMP RunServer7702FromString(char* i, char* o)
{
    EnterServer();
    Server7702Input input;
    try
        {
        std::istringstream is(i);
        is >> input;
        Server7702 contract(input);
        contract.Process();
        std::ostringstream os;
        os << contract.GetOutput();
        std::strcpy(o, os.str().c_str());
        }
    // Catch exceptions that are thrown during input
    catch(std::exception& x)
        {
        std::cerr << input.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(...)
        {
        std::cerr << "Untrapped exception" << '\n';
        }
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
    if(IntelDefaultNdpControlWord() != NdpControlWord())
        {
        std::ostringstream error;
        error
            << "Floating point control word was initially "
            << std::hex << IntelDefaultNdpControlWord()
            << ", but has been changed to "
            << std::hex << NdpControlWord()
            << " while this program was running, presumably "
            << "due to another process. Shut down the rogue process, "
            << "discard output, and rerun. Consider migrating to an "
            << "operating system like GNU/Linux that virtualizes the
            << "floating-point processor."
            ;
        throw server7702_precision_changed(error.str());
        }
    // Reset precision so that we can continue
    EnterServer();
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
    catch(server7702_precision_changed& x)
        {
        Output.Status |= precision_changed;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(server7702_implausible_input& x)
        {
        Output.Status |= implausible_input;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(server7702_inconsistent_input& x)
        {
        Output.Status |= inconsistent_input;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(x_product_rule_violated& x)
        {
        Output.Status |= product_rule_violated;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(server7702_adjustable_event_forbidden_at_issue& x)
        {
        Output.Status |= adjustable_event_forbidden_at_issue;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(server7702_guideline_negative& x)
        {
        Output.Status |= guideline_negative;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(server7702_misstatement_of_age_or_gender& x)
        {
        Output.Status |= misstatement_of_age_or_gender;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }
    catch(std::range_error& x)
        {
        Output.Status |= implausible_input; // TODO ?? can we be more specific?
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        }

    // Unknown error
    catch(std::exception& x)
        {
        Output.Status |= unknown_error;
        std::cerr << Output.UniqueIdentifier << " error: " << x.what() << '\n';
        // Since we don't know what the error is, we propagate
        // it back to the caller; we put a message on standard error,
        // but don't try to emit anything to standard output.
        throw;
        }
}

//============================================================================
void Server7702::PerformProcessing()
{
    Output.UniqueIdentifier                 = Input.UniqueIdentifier;
    Output.Status                           = 0;
    Output.AdjustableEventOccurred          = false;
    Output.GuidelineLevelPremium            = Input.OldGuidelineLevelPremium;
    Output.GuidelineSinglePremium           = Input.OldGuidelineSinglePremium;
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

    if(0.0 != Input.DecreaseRequiredByContract)
        {
        throw std::logic_error
            (
            "Contractually required decrease not implemented"
            );
        }
    if(Input.NewBenefitAmount < Input.LeastBenefitAmountEver)
        {
        throw server7702_implausible_input
            (
            "New benefit amount less than least benefit amount ever"
            );
        }
    if(Input.OldBenefitAmount < Input.LeastBenefitAmountEver)
        {
        throw server7702_implausible_input
            (
            "Old benefit amount less than least benefit amount ever"
            );
        }
    if(Input.NewSpecifiedAmount < Input.LeastBenefitAmountEver)
        {
        throw server7702_implausible_input
            (
            "New specified amount less than least benefit amount ever"
            );
        }
    if(Input.OldSpecifiedAmount < Input.LeastBenefitAmountEver)
        {
        throw server7702_implausible_input
            (
            "Old specified amount less than least benefit amount ever"
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

    IsIssuedToday = Input.IsIssuedToday;

    IsPossibleAdjustableEvent =
// TODO ?? Why treat a taxable withdrawal as an adjustment event?
//            0.0                             != Input.GrossNontaxableWithdrawal
            true                            == Input.DecreaseRequiredByContract
        ||  Input.NewDeathBenefitOption     != Input.OldDeathBenefitOption
        ||  (   Input.NewSpecifiedAmount    != Input.OldSpecifiedAmount
            &&  Input.NewBenefitAmount      != Input.OldBenefitAmount
            )
// TODO ?? NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewTermAmount             != Input.OldTermAmount
// TODO ?? No adj event if term and SA change but DB remains constant, but
// TODO ?? NEED DECISION whether it's a SA or DB change that causes adj event
        ||  Input.NewSmoker                 != Input.OldSmoker
// 7702 mortality basis is the same for preferred vs. standard
// Assume nothing else (e.g. loads) varies by that either
//      ||  Input.NewUnderwritingClass      != Input.OldUnderwritingClass
// Assume state premium tax passthrough is ignored for GPT
//      ||  Input.NewStateOfJurisdiction    != Input.OldStateOfJurisdiction
// Assume WP is completely ignored
//      ||  Input.NewWaiverOfPremiumInForce != Input.OldWaiverOfPremiumInForce
//      ||  Input.NewPremiumsWaived         != Input.OldPremiumsWaived
//      ||  Input.NewWaiverOfPremiumRating  != Input.OldWaiverOfPremiumRating
        ||  Input.NewAccidentalDeathInForce != Input.OldAccidentalDeathInForce
// Assume ADD rating is ignored
//      ||  Input.NewAccidentalDeathRating  != Input.OldAccidentalDeathRating
// Assume table rating is ignored
//      ||  Input.NewTableRating            != Input.OldTableRating
// Assume flat extras are ignored
//      ||  Input.NewPermanentFlatAmount0   != Input.OldPermanentFlatAmount0
//      ||  Input.NewTemporaryFlatAmount0   != Input.OldTemporaryFlatAmount0
//      ||  Input.NewTemporaryFlatDuration0 != Input.OldTemporaryFlatDuration0
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
            Input.Duration                  == 0
        &&  Input.OldGuidelineLevelPremium  == 0.0
        &&  Input.OldGuidelineSinglePremium == 0.0
        &&  Input.OldGender                 == Input.NewGender
        &&  Input.OldUnderwritingClass      == Input.NewUnderwritingClass
        &&  Input.OldSmoker                 == Input.NewSmoker
        &&  Input.OldIssueAge               == Input.NewIssueAge
        &&  Input.OldStateOfJurisdiction    == Input.NewStateOfJurisdiction
        &&  Input.OldSpecifiedAmount        == Input.NewSpecifiedAmount
        &&  Input.OldDeathBenefitOption     == Input.NewDeathBenefitOption
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
// TODO ?? And throw something here.
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

    Output.GuidelineLevelPremiumPolicyA = Input.OldGuidelineLevelPremium;
    Output.GuidelineSinglePremiumPolicyA = Input.OldGuidelineSinglePremium;

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
    std::auto_ptr<BasicValues> basic_values_A
        (new BasicValues
            (Input.ProductName
            ,Input.OldGender
            ,Input.OldUnderwritingClass
            ,Input.OldSmoker
            ,Input.OldIssueAge
            ,Input.UnderwritingBasis
            ,Input.OldStateOfJurisdiction
            ,Input.OldSpecifiedAmount
            ,Input.OldDeathBenefitOption
            ,Input.OldAccidentalDeathInForce
            ,Input.TargetPremium
            )
        );

    Output.GuidelineLevelPremiumPolicyA = basic_values_A->Irc7702_->CalculateGLP
        (0
        ,Input.OldBenefitAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldDeathBenefitOption
        );

    Output.GuidelineSinglePremiumPolicyA = basic_values_A->Irc7702_->CalculateGSP
        (0
        ,Input.OldBenefitAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldSpecifiedAmount
        );
}

//============================================================================
void Server7702::SetDoleBentsenValuesBC()
{
    std::auto_ptr<BasicValues> basic_values_B
        (new BasicValues
            (Input.ProductName
            ,Input.NewGender
            ,Input.NewUnderwritingClass
            ,Input.NewSmoker
            ,Input.NewIssueAge
            ,Input.UnderwritingBasis
            ,Input.NewStateOfJurisdiction
            ,Input.NewSpecifiedAmount
            ,Input.NewDeathBenefitOption
            ,Input.NewAccidentalDeathInForce
            ,Input.TargetPremium
            )
        );

    Output.GuidelineLevelPremiumPolicyB = basic_values_B->Irc7702_->CalculateGLP
        (Input.Duration
        ,Input.NewBenefitAmount
        ,Input.NewSpecifiedAmount
        ,Input.NewSpecifiedAmount
        ,Input.NewDeathBenefitOption
        );

    Output.GuidelineSinglePremiumPolicyB = basic_values_B->Irc7702_->CalculateGSP
        (Input.Duration
        ,Input.NewBenefitAmount
        ,Input.NewSpecifiedAmount
        ,Input.NewSpecifiedAmount
        );

    std::auto_ptr<BasicValues> basic_values_C
        (new BasicValues
            (Input.ProductName
            ,Input.OldGender
            ,Input.OldUnderwritingClass
            ,Input.OldSmoker
            ,Input.OldIssueAge
            ,Input.UnderwritingBasis
            ,Input.OldStateOfJurisdiction
            ,Input.OldSpecifiedAmount
            ,Input.OldDeathBenefitOption
            ,Input.OldAccidentalDeathInForce
            ,Input.TargetPremium
            )
        );

    Output.GuidelineLevelPremiumPolicyC = basic_values_C->Irc7702_->CalculateGLP
        (Input.Duration
        ,Input.OldBenefitAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldDeathBenefitOption
        );

    Output.GuidelineSinglePremiumPolicyC = basic_values_C->Irc7702_->CalculateGSP
        (Input.Duration
        ,Input.OldBenefitAmount
        ,Input.OldSpecifiedAmount
        ,Input.OldSpecifiedAmount
        );
}

