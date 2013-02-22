// Guideline-premium-test server.
//
// Copyright (C) 1998, 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#ifndef ihs_server7702_hpp
#define ihs_server7702_hpp

#include "config.hpp"

#include "ihs_server7702io.hpp"
#include "so_attributes.hpp"

#include <stdexcept>
#include <string>

int RunServer7702();

extern "C"
{
    void LMI_SO InitializeServer7702();
    Server7702Output LMI_SO RunServer7702FromStruct(Server7702Input a_Input);
}

class Server7702
{
  public:
    Server7702(Server7702Input& a_Input);
    void Process();
    Server7702Output const& GetOutput() const   {return Output;}

  private:
    void VerifyPrecision() const;
    void PerformProcessing();
    void VerifyPlausibilityOfInput() const;
    void DecideWhatToCalculate();
    void ProcessNewIssue();
    void ProcessAdjustableEvent();

    // Calculate GLP and GSP.
    //
    // When an adjustable event occurs, we need three sets
    // of {GLP, GSP} to perform the calculations described
    // in the "Dole-Bentsen colloquy" aka the A+B-C method.
    //
    // At issue, we need only one set of {GLP, GSP}. We
    // can avoid needless duplication of code by running
    // just the "A" portion of the A+B-C method. Note that
    // this reuse requires that "old" parameters equal "new"
    // parameters for a new issue, since "A" is defined in
    // terms of the "old" policy just before an adjustable
    // event occurs.
    void SetDoleBentsenValuesA();
    void SetDoleBentsenValuesBC();

    Server7702Input const& Input;
    Server7702Output Output;

    bool IsIssuedToday;
    bool IsPossibleAdjustableEvent;
};

enum
    {unknown_error                          = 0x0001
    ,precision_changed                      = 0x0002
    ,implausible_input                      = 0x0004
    ,inconsistent_input                     = 0x0008
    ,product_rule_violated                  = 0x0010
    ,adjustable_event_forbidden_at_issue    = 0x0020
    ,guideline_negative                     = 0x0040
    ,misstatement_of_age_or_gender          = 0x0080
    };

// We have no specific exception class for unknown_error
// precisely because it's unknown. We return it when we
// catch class exception after trying all the more
// specific classes.

class server7702_precision_changed
    :public std::runtime_error
{
  public:
    server7702_precision_changed(std::string const& what_arg)
    :std::runtime_error("Internal precision changed: " + what_arg)
    {}
};

class server7702_implausible_input
    :public std::runtime_error
{
  public:
    server7702_implausible_input(std::string const& what_arg)
    :std::runtime_error("Implausible input: " + what_arg)
    {}
};

class server7702_inconsistent_input
    :public std::runtime_error
{
  public:
    server7702_inconsistent_input(std::string const& what_arg)
    :std::runtime_error("Inconsistent input: " + what_arg)
    {}
};

class server7702_adjustable_event_forbidden_at_issue
    :public std::runtime_error
{
  public:
    server7702_adjustable_event_forbidden_at_issue(std::string const& what_arg)
    :std::runtime_error("Adjustable event forbidden at issue: " + what_arg)
    {}
};

class server7702_guideline_negative
    :public std::runtime_error
{
  public:
    server7702_guideline_negative(std::string const& what_arg)
    :std::runtime_error("Guideline premium is negative: " + what_arg)
    {}
};

class server7702_misstatement_of_age_or_gender
    :public std::runtime_error
{
  public:
    server7702_misstatement_of_age_or_gender(std::string const& what_arg)
    :std::runtime_error("Misstatement of age or gender: " + what_arg)
    {}
};

#endif // ihs_server7702_hpp

