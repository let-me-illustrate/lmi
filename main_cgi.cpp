// Common gateway interface using gnu cgicc: a simplistic demo.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

// This is a derived work based on Stephen F. Booth's
//   cgicc-3.1.4/demo/test.cpp
// which bears copyright notice
//   GNU cgicc Copyright (C) 1996, 1997, 1998, 1999, 2000 Stephen F. Booth
// and was released under the same license as this file, except with
// permission to use a later version of the GPL.
//
// Gregory W. Chicares modified it in the years shown above to adapt
// it to the life-insurance-illustrations problem domain, and for
// other reasons evident in the repository or explained in 'ChangeLog'.
// Any defect should not reflect on Stephen F. Booth's reputation.

#include "pchfile.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "main_common.hpp"
#include "mc_enum_type_enums.hpp"       // mcenum_emission
#include "miscellany.hpp"
#include "platform_dependent.hpp"       // putenv() [GWC]
#include "ssize_lmi.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <cgicc/Cgicc.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/CgiUtils.h>             // gLogFile
#include <cgicc/HTMLClasses.h>
#if defined USING_CURRENT_CGICC
#   include <cgicc/HTTPHTMLHeader.h>    // cgicc-3.2.3
#else  // !defined USING_CURRENT_CGICC
#   include <cgicc/HTTPHeaders.h>
#endif // !defined USING_CURRENT_CGICC

#include <cstring>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>                      // ws
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#if defined HAVE_SYS_TIME_H && HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif // defined HAVE_SYS_TIME_H && HAVE_SYS_TIME_H

// To use logging, the variable gLogFile MUST be defined, and it _must_
// be an ofstream.
std::ofstream gLogFile;

// Function prototypes
void ShowEnvironment(cgicc::CgiEnvironment const& env);
void ShowInput(cgicc::Cgicc const& formData);
void ShowOutput(cgicc::Cgicc const& formData);
void ShowIllusOutput(Input const&);
void ShowCensusOutput(Input const&, std::string const&, bool);

// SOMEDAY !! It would be nicer to use gnu getopt here.
int try_main(int argc, char* argv[])
{
  try {
    global_settings::instance().set_data_directory("/opt/lmi/data");

    gLogFile.rdbuf()->open
        (configurable_settings::instance().cgi_bin_log_filename().c_str()
        ,ios_out_trunc_binary()
        );

    if(argc == 2 && argv[1] == std::string("--capture"))
        {
#       if defined LMI_POSIX
        int exit_code = std::system("env > settings");
#       else
        int exit_code = std::system("set > settings");
#       endif // defined LMI_POSIX
        if(EXIT_SUCCESS != exit_code)
            {
            std::cerr << "Failed to execute 'set' command;";
            return EXIT_FAILURE;
            }
        std::ofstream os("stdin.txt", ios_out_trunc_binary());
        os << std::cin.rdbuf();
        std::cout
            << "Environment and QUERY_STRING captured for later use\n"
            << "in debugging. File 'settings' can be turned into a\n"
            << "command script in an OS-dependent way. After executing\n"
            << "the script, run the program, redirected its input from\n"
            << "file 'stdin.txt'."
            ;
        // We've read stdin already, and it can't be rewound, so
        // just exit.
        return EXIT_SUCCESS;
        }
    static char const content_string[] =
        "ProductName=sample"
        "&IssueAge=45"
        "&RetirementAge=65"
        "&Gender=Female"
        "&UnderwritingClass=Preferred"
        "&Smoking=Smoker"
        "&WaiverOfPremiumBenefit=No"
        "&AccidentalDeathBenefit=No"
        "&SolveType=Employee premium"
        "&SolveBeginYear=0"
        "&SolveEndYear=10"
        "&SolveTarget=Target CSV"
        "&SolveTargetValue=1000000"
        "&SolveTargetYear=20"
        "&SolveExpenseGeneralAccountBasis=Current"
        "&SpecifiedAmount=1000000"
        "&DeathBenefitOption=a"
        "&Payment=0"
        "&PaymentMode=annual"
        "&Dumpin=0"
        "&1035ExchangeAmount=0"
        "&1035ExchangeIsMec=No"
        "&1035ExchangeBasis=0"
        "&GeneralAccountRate=.055"
        "&GeneralAccountRateType=Credited rate"
        "&LoanRate=.055"
        "&LoanRateType=Fixed loan rate"
        "&Comments="
        "&Census="
        "&ShowOutput=on"
        "&submit=Submit"
        ;
    // Write 'content_string' to a file suitable for piping into this
    // program for standalone test.
    if(argc == 2 && argv[1] == std::string("--write_content_string"))
        {
        std::ofstream os("cgi.test.in", ios_out_trunc_binary());
        os << content_string;
        return EXIT_SUCCESS;
        }
    // Set CONTENT_LENGTH and REQUEST_METHOD for standalone test.
    if(argc == 2 && argv[1] == std::string("--enable_test"))
        {
        std::string s("CONTENT_LENGTH=");
        s += value_cast<std::string>(std::strlen(content_string));
        static char content_length_string[sizeof "CONTENT_LENGTH=FFFFFFFFFFFFFFFF\0"];
        std::strcpy(content_length_string, s.c_str());
        putenv(content_length_string);
        putenv(const_cast<char*>("REQUEST_METHOD=POST"));
        }

    // Create a new cgicc::Cgicc object containing all the CGI data
    cgicc::Cgicc cgi;

    // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
    std::cout << cgicc::HTTPHTMLHeader() << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << '\n';
    std::cout << cgicc::html().set("lang", "EN").set("dir", "LTR") << '\n';

    // Set up the page's header and title.
    // I will put in lfs to ease reading of the produced HTML.
    std::cout << cgicc::head() << '\n';

    // Output the style sheet portion of the header
    std::cout << cgicc::style() << cgicc::comment() << "[suppressed]" << '\n';
    std::cout << "BODY { color: black; background-color: white; }\n";
    std::cout << "HR.half { width: 60%; align: center; }\n";
    std::cout << "SPAN.red, STRONG.red { color: red; }\n";
    std::cout << "DIV.smaller { font-size: small; }\n";
    std::cout << "DIV.notice { border: solid thin; padding: 1em; margin: 1em 0; "
         << "background: #DDD; }\n";
    std::cout << "SPAN.blue { color: blue; }\n";
    std::cout << "COL.title { color: white; background-color: black; ";
    std::cout << "font-weight: bold; text-align: center; }\n";
    std::cout << "COL.data { background-color: #DDD; text-align: left; }\n";
    std::cout << "TD.data, TR.data { background-color: #DDD; text-align: left; }\n";
    std::cout << "TD.grayspecial { background-color: #DDD; text-align: left; }\n";
    std::cout << "TD.ltgray, TR.ltgray { background-color: #DDD; }\n";
    std::cout << "TD.dkgray, TR.dkgray { background-color: #BBB; }\n";
    std::cout << "COL.black, TD.black, TD.title, TR.title { color: white; "
         << "background-color: black; font-weight: bold; text-align: center; }\n";
    std::cout << "COL.gray, TD.gray { background-color: #DDD; text-align: center; }\n";
    std::cout << "TABLE.cgi { left-margin: auto; right-margin: auto; width: 90%; }\n";

    std::cout << cgicc::comment() << cgicc::style() << '\n';

    std::cout << cgicc::title() << "Let me illustrate...cgi v" << cgi.getVersion()
         << " Test Results"
         << cgicc::title() << '\n';
    std::cout << cgicc::meta().set("name", "author").set("content", "Gregory W. Chicares")
         << '\n';

    std::cout << cgicc::head() << '\n';

    // Start the HTML body
    std::cout << cgicc::body() << '\n';

    std::cout << cgicc::h1() << "Let me illustrate... cgi v" << cgi.getVersion()
     << " Test Results" << cgicc::h1() << '\n';

    // Get a pointer to the environment
    cgicc::CgiEnvironment const& env = cgi.getEnvironment();

    // Generic thank you message
    std::cout << cgicc::comment() << "This page generated by GNU cgicc for "
         << env.getRemoteHost() << cgicc::comment() << '\n';
    std::cout << cgicc::h4() << "Thanks for using Let me illustrate..."
         << ", " << env.getRemoteHost()
         << '(' << env.getRemoteAddr() << ")!" << cgicc::h4() << '\n';

    // If the user wants to save the submission, do it
    if(cgi.queryCheckbox("save")) {
      cgi.save("save");
      std::cout << cgicc::p(cgicc::h2("Data Saved")) << '\n';

      std::cout << cgicc::div().set("class", "notice") << '\n';
      std::cout << "Your data has been saved, and may be restored (by anyone) "
           << "via the same form.\n" << cgicc::div() << cgicc::p() << '\n';
    }

    // If the user wants to restore from the last submission, do it
    if(cgi.queryCheckbox("restore")) {
      cgi.restore("save");
      std::cout << cgicc::p(cgicc::h2("Data Restored")) << '\n';

      std::cout << cgicc::div().set("class", "notice") << '\n';
      std::cout << "The data displayed has been restored from a file on disk.\n"
           << cgicc::div() << cgicc::p() << '\n';
    }

    // If the user requested a dump of the environment,
    // create a simple table showing the values of the
    // environment variables
    if(cgi.queryCheckbox("ShowEnvironment"))
      ShowEnvironment(env);

    // If the user requested, print out the raw form data from
    // the vector of FormEntries.  This will contain every
    // element in the list.
    // This is one of two ways to get at form data, the other
    // being the use of cgicc::Cgicc's getElement() methods.
    if(cgi.queryCheckbox("ShowInput"))
      ShowInput(cgi);

    // If the user requested data via cgicc::Cgicc's getElement() methods, do it.
    // This is different than the use of the list of FormEntries
    // because it requires prior knowledge of the name of form elements.
    // Usually they will be known, but you never know.
    if(cgi.queryCheckbox("ShowOutput"))
      ShowOutput(cgi);

    // Now print out a footer with some fun info
    std::cout << cgicc::p() << cgicc::div().set("align","center");
    std::cout << cgicc::a("Back to form").set("href", cgi.getEnvironment().getReferrer())
         << '\n';
    std::cout << cgicc::div() << cgicc::br() << cgicc::hr(cgicc::set("class","half")) << '\n';

    // Information on cgicc
    std::cout << cgicc::div().set("align","center").set("class","smaller") << '\n';
    std::cout << "GNU cgi" << cgicc::span("cc").set("class","red") << " v";
    std::cout << cgi.getVersion();
    std::cout << " by " << cgicc::a("Stephen F. Booth")
                       .set("href", "http://home.earthlink.net/~sfbooth/")
         << cgicc::br() << '\n';
    std::cout << "Compiled at " << cgi.getCompileTime();
    std::cout << " on " << cgi.getCompileDate() << cgicc::br() << '\n';

    // End of document
    std::cout << cgicc::div() << '\n';
    std::cout << cgicc::body() << cgicc::html() << '\n';

    // No chance for failure in this example
    return EXIT_SUCCESS;
  }

  // Did any errors occur?
  catch(std::exception const& e) {

    // This is a dummy exception handler, as it doesn't really do
    // anything except print out information.

    // Reset all the HTML elements that might have been used to
    // their initial state so we get valid output
    cgicc::html::reset();    cgicc::head::reset();        cgicc::body::reset();
    cgicc::title::reset();   cgicc::h1::reset();          cgicc::h4::reset();
    cgicc::comment::reset(); cgicc::td::reset();          cgicc::tr::reset();
    cgicc::table::reset();   cgicc::div::reset();         cgicc::p::reset();
    cgicc::a::reset();       cgicc::h2::reset();          cgicc::colgroup::reset();

    // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
    std::cout << cgicc::HTTPHTMLHeader() << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << '\n';
    std::cout << cgicc::html().set("lang","EN").set("dir","LTR") << '\n';

    // Set up the page's header and title.
    // I will put in lfs to ease reading of the produced HTML.
    std::cout << cgicc::head() << '\n';

    // Output the style sheet portion of the header
    std::cout << cgicc::style() << cgicc::comment() << "[suppressed]" << '\n';
    std::cout << "BODY { color: black; background-color: white; }\n";
    std::cout << "HR.half { width: 60%; align: center; }\n";
    std::cout << "SPAN.red, STRONG.red { color: red; }\n";
    std::cout << "DIV.notice { border: solid thin; padding: 1em; margin: 1em 0; "
         << "background: #DDD; }\n";

    std::cout << cgicc::comment() << cgicc::style() << '\n';

    std::cout << cgicc::title("GNU cgicc exception") << '\n';
    std::cout << cgicc::meta().set("name", "author")
                  .set("content", "Stephen F. Booth") << '\n';
    std::cout << cgicc::head() << '\n';

    std::cout << cgicc::body() << '\n';

    std::cout << cgicc::h1() << "GNU cgi" << cgicc::span("cc", cgicc::set("class","red"))
         << " caught an exception" << cgicc::h1() << '\n';

    std::cout << cgicc::div().set("align","center").set("class","notice") << '\n';

    std::cout << cgicc::h2(e.what()) << '\n';

    // End of document
    std::cout << cgicc::div() << '\n';
    std::cout << cgicc::hr().set("class","half") << '\n';
    std::cout << cgicc::body() << cgicc::html() << '\n';

    return EXIT_SUCCESS;
  }
  catch(...) {
    std::cerr << "Unknown exception;";
    return EXIT_FAILURE;
  }
}

// Print out a table of the CgiEnvironment
void
ShowEnvironment(cgicc::CgiEnvironment const& env)
{
  // This is just a brain-dead dump of information.
  // Almost all of this code is for HTML formatting
  std::cout << cgicc::h2("Environment information from CgiEnvironment") << '\n';

  std::cout << cgicc::div().set("align","center") << '\n';

  std::cout << cgicc::table().set("border","0").set("rules","none").set("frame","void")
                 .set("cellspacing","2").set("cellpadding","2")
                 .set("class","cgi") << '\n';
  std::cout << cgicc::colgroup().set("span","2") << '\n';
  std::cout << cgicc::col().set("align","center").set("class","title").set("span","1")
       << '\n';
  std::cout << cgicc::col().set("align","left").set("class","data").set("span","1")
       << '\n';
  std::cout << cgicc::colgroup() << '\n';

  std::cout << cgicc::tr() << cgicc::td("Request Method").set("class","title")
       << cgicc::td(env.getRequestMethod()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Path Info").set("class","title")
       << cgicc::td(env.getPathInfo()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Path Translated").set("class","title")
       << cgicc::td(env.getPathTranslated()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Script Name").set("class","title")
       << cgicc::td(env.getScriptName()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("HTTP Referrer").set("class","title")
       << cgicc::td(env.getReferrer()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("HTTP Cookie").set("class","title")
       << cgicc::td(env.getCookies()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Query String").set("class","title")
       << cgicc::td(env.getQueryString()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Content Length").set("class","title")
       << cgicc::td().set("class","data") << env.getContentLength()
       << cgicc::td() << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Post Data").set("class","title")
       << cgicc::td().set("class","data")
       << cgicc::pre(env.getPostData()).set("class","data") << cgicc::td()
       << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Remote Host").set("class","title")
       << cgicc::td(env.getRemoteHost()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Remote Address").set("class","title")
       << cgicc::td(env.getRemoteAddr()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Authorization Type").set("class","title")
       << cgicc::td(env.getAuthType()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Remote User").set("class","title")
       << cgicc::td(env.getRemoteUser()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Remote Identification").set("class","title")
       << cgicc::td(env.getRemoteIdent()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Content Type").set("class","title")
       << cgicc::td(env.getContentType()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("HTTP Accept").set("class","title")
       << cgicc::td(env.getAccept()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("User Agent").set("class","title")
       << cgicc::td(env.getUserAgent()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Server Software").set("class","title")
       << cgicc::td(env.getServerSoftware()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Server Name").set("class","title")
       << cgicc::td(env.getServerName()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Gateway Interface").set("class","title")
       << cgicc::td(env.getGatewayInterface()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Server Protocol").set("class","title")
       << cgicc::td(env.getServerProtocol()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Server Port").set("class","title")
       << cgicc::td().set("class","data") << env.getServerPort()
       << cgicc::td() << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("HTTPS").set("class","title")
       << cgicc::td().set("class","data") << (env.usingHTTPS() ? "true" : "false")
       << cgicc::td() << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Redirect Request").set("class","title")
       << cgicc::td(env.getRedirectRequest()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Redirect URL").set("class","title")
       << cgicc::td(env.getRedirectURL()).set("class","data") << cgicc::tr() << '\n';
  std::cout << cgicc::tr() << cgicc::td("Redirect Status").set("class","title")
       << cgicc::td(env.getRedirectStatus()).set("class","data") << cgicc::tr() << '\n';

  std::cout << cgicc::table() << cgicc::div() << '\n';
}

std::string GetValue(cgicc::Cgicc const& cgi, std::string const& name)
{
    cgicc::const_form_iterator data = cgi.getElement(name);
    if(data != cgi.getElements().end())
        {
        // iterator refers to a valid element
        return **data;
        }
    else
        {
        return "0";
        }
}

// Print out the value of every form element
void
ShowInput(cgicc::Cgicc const& data)
{
  std::cout << cgicc::h2("Form Data via vector") << '\n';

  std::cout << cgicc::div().set("align","center") << '\n';

  std::cout << cgicc::table().set("border","0").set("rules","none").set("frame","void")
                 .set("cellspacing","2").set("cellpadding","2")
                 .set("class","cgi") << '\n';
  std::cout << cgicc::colgroup().set("span","2") << '\n';
  std::cout << cgicc::col().set("align","center").set("span","2") << '\n';
  std::cout << cgicc::colgroup() << '\n';

  std::cout << cgicc::tr().set("class","title") << cgicc::td("Element Name")
       << cgicc::td("Element Value") << cgicc::tr() << '\n';

  // Iterate through the vector, and print out each value
  for(auto const& j : data.getElements())
      {
      std::cout << cgicc::tr().set("class","data") << cgicc::td(j.getName())
         << cgicc::td(j.getValue()) << cgicc::tr() << '\n';
      }
  std::cout << cgicc::table() << cgicc::div() << '\n';
}

// Print illustration.
void ShowOutput(cgicc::Cgicc const& data)
{
    Timer timer;
    Input input;

    // Explicitly set certain defaults.
    input["IssueAge"]                        = "45";
    input["RetirementAge"]                   = "65";
    input["Gender"]                          = "Female";
    input["Smoking"]                         = "Smoker";
    input["UnderwritingClass"]               = "Preferred";
    input["WaiverOfPremiumBenefit"]          = "No";
    input["AccidentalDeathBenefit"]          = "No";
    input["SolveTgtAtWhich"]                 = "Year";
    input["SolveToWhich"]                    = "Year";
    input["UseCurrentDeclaredRate"]          = "No";

    // Read input values.
    if(!is_antediluvian_fork())
        {
        input["ProductName"]                 = GetValue(data, "ProductName"                    );
        }
    input["IssueAge"]                        = GetValue(data, "IssueAge"                       );
    input["RetirementAge"]                   = GetValue(data, "RetirementAge"                  );
    input["Gender"]                          = GetValue(data, "Gender"                         );
    input["UnderwritingClass"]               = GetValue(data, "UnderwritingClass"              );
    input["Smoking"]                         = GetValue(data, "Smoking"                        );
    input["WaiverOfPremiumBenefit"]          = GetValue(data, "WaiverOfPremiumBenefit"         );
    input["AccidentalDeathBenefit"]          = GetValue(data, "AccidentalDeathBenefit"         );
    input["SolveType"]                       = GetValue(data, "SolveType"                      );
    input["SolveBeginYear"]                  = GetValue(data, "SolveBeginYear"                 );
    input["SolveEndYear"]                    = GetValue(data, "SolveEndYear"                   );
    input["SolveTarget"]                     = GetValue(data, "SolveTarget"                    );
    input["SolveTargetValue"]                = GetValue(data, "SolveTargetValue"               );
    input["SolveTargetYear"]                 = GetValue(data, "SolveTargetYear"                );
    input["SolveExpenseGeneralAccountBasis"] = GetValue(data, "SolveExpenseGeneralAccountBasis");
    input["SpecifiedAmount"]                 = GetValue(data, "SpecifiedAmount"                );
    input["DeathBenefitOption"]              = GetValue(data, "DeathBenefitOption"             );
    input["Payment"]                         = GetValue(data, "Payment"                        );
    input["PaymentMode"]                     = GetValue(data, "PaymentMode"                    );
    input["Dumpin"]                          = GetValue(data, "Dumpin"                         );
    input["Internal1035ExchangeAmount"]      = GetValue(data, "1035ExchangeAmount"             );
    input["Internal1035ExchangeFromMec"]     = GetValue(data, "1035ExchangeIsMec"              );
    input["Internal1035ExchangeTaxBasis"]    = GetValue(data, "1035ExchangeBasis"              );
    input["GeneralAccountRate"]              = GetValue(data, "GeneralAccountRate"             );
    input["GeneralAccountRateType"]          = GetValue(data, "GeneralAccountRateType"         );
    input["LoanRate"]                        = GetValue(data, "LoanRate"                       );
    input["LoanRateType"]                    = GetValue(data, "LoanRateType"                   );

    input.RealizeAllSequenceInput();

    std::string census_data = GetValue(data, "Census");

    std::cout
        << "      Input:        "
        << Timer::elapsed_msec_str(timer.stop().elapsed_seconds())
        << "<BR>\n"
        ;
    if("" == census_data)
        {
        ShowIllusOutput(input);
        }
    else
        {
        // If the user requested, show illustration for each life in census
        // as well as composite.
        bool show_each_life = false;
        if(data.queryCheckbox("ShowEachLife"))
          show_each_life = true;
        ShowCensusOutput(input, census_data, show_each_life);
        }
}

void ShowIllusOutput(Input const& a_input)
{
    static mcenum_emission const emission = mcenum_emission
        (   mce_emit_text_stream
        |   mce_emit_quietly
        );
    illustrator run_functor(emission);
    std::cout << "<pre><small>";
    run_functor("CGI_illustration", a_input);
    std::cout << "</small></pre>";
    std::cout << "<hr>\n\n";
    std::cout
        << "    Calculations: "
        << Timer::elapsed_msec_str(run_functor.seconds_for_calculations())
        << "<BR>\n"
        << "    Output:       "
        << Timer::elapsed_msec_str(run_functor.seconds_for_output())
        << "<BR>\n"
        ;
}

void ShowCensusOutput
    (Input const& a_input
    ,std::string const& a_census
    ,bool show_each_life
    )
{
    std::vector<std::string> headers;
    std::vector<Input> lives;

    std::istringstream iss_census(a_census);
    std::string line;

    // Get header line; parse into field names.
    if(std::getline(iss_census, line, '\r'))
        {
        iss_census >> std::ws;

        std::istringstream iss_line(line);
        std::string token;

        while(std::getline(iss_line, token, '\t'))
            {
            headers.push_back(token);
            }
        }

    // Read each subsequent line into an input object representing one life.
    int current_line = 0;
    while(std::getline(iss_census, line, '\r'))
        {
        ++current_line;

        iss_census >> std::ws;

        Input input(a_input);
        std::istringstream iss_line(line);
        std::string token;
        std::vector<std::string> values;

        while(std::getline(iss_line, token, '\t'))
            {
            values.push_back(token);
            }
        if(values.size() != headers.size())
            {
            alarum()
                << "Line #" << current_line << ":<BR>"
                << "  (" << line << ")<BR>"
                << "should have one value per column.<BR>"
                << "Number of values: " << values.size() << "; "
                << "number expected: " << headers.size() << ".<BR>"
                << LMI_FLUSH
                ;
            }

        for(int j = 0; j < lmi::ssize(headers); ++j)
            {
            input[headers[j]] = values[j];
            }
        input.RealizeAllSequenceInput();
        lives.push_back(input);
        }

    // Print census as HTML table.
    std::cout << "Census:<BR><BR>";
    std::cout << "<TABLE>";
    std::cout << "<TR>";

    for(auto const& h : headers)
        {
        std::cout << "<TD>" << h << "</TD>";
        }
    std::cout << "</TR>";
    for(auto const& i : lives)
        {
        std::cout << "<TR>";
        for(auto const& h : headers)
            {
            std::cout << "<TD>" << i[h] << "</TD>";
            }
        std::cout << "</TR>";
        }
    std::cout << "</TABLE>";
    std::cout << "<BR>";

    std::cout << "Composite illustration:<BR>";

    // Calculate and print results.
    static mcenum_emission const emission = mcenum_emission
        (   mce_emit_text_stream
        |   mce_emit_composite_only
        |   mce_emit_quietly
        );
    illustrator run_functor(emission);
    std::cout << "<pre><small>";
    run_functor("CGI_census", lives);
    std::cout << "</small></pre>";
    std::cout << "<hr>\n\n";
    std::cout
        << "    Calculations: "
        << Timer::elapsed_msec_str(run_functor.seconds_for_calculations())
        << "<BR>\n"
        << "    Output:       "
        << Timer::elapsed_msec_str(run_functor.seconds_for_output())
        << "<BR>\n"
        ;

    // SOMEDAY !! Inefficient: we already did all the calculations for
    // each life in order to get the composite.
    if(show_each_life)
        {
        for(auto const& i : lives)
            {
            ShowIllusOutput(i);
            }
        }
}
