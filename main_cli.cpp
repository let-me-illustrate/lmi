// Life insurance illustrations: command-line interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: main_cli.cpp,v 1.6 2005-06-11 15:31:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"
#include "argv0.hpp"
#include "calculate.hpp"
#include "custom_io_0.hpp"
#include "getopt.hpp"
#include "global_settings.hpp"
#include "group_values.hpp"
#include "ledger_text_formats.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "miscellany.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>  // std::printf()
#include <exception>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

//============================================================================
void RegressionTestOneCensusFile(fs::directory_iterator i)
{
    std::cout << "Regression testing: " << i->string() << std::endl;
    multiple_cell_document doc(i->string());

// The run order depends on the first cell's parameters and ignores any
// conflicting input for any individual cell. Perhaps we should detect
// conflicting input and signal an error? It would probably be cleaner
// to offer this input item (and a few similar ones) only at the case
// level. TODO ?? Fix this, and remove duplicate code in the census
// manager.
    if(doc.case_parms()[0].RunOrder != doc.cell_parms()[0].RunOrder)
        {
        fatal_error()
            << "Case-default run order '"
            << doc.case_parms()[0].RunOrder
            << "' differs from first cell's run order '"
            << doc.cell_parms()[0].RunOrder
            << "'. Make them consistent then run again."
            << LMI_FLUSH
            ;
        }

    run_census()(*i, emit_to_spew_file, doc.cell_parms());
}

//============================================================================
void RegressionTestOneIniFile(fs::directory_iterator i)
{
    std::cout << "Regression testing: " << i->string() << std::endl;
    IllusVal IV;
    IllusInputParms IP;
    SetSpecialInput(IP, i->string().c_str());
    IV.Run(&IP);
    fs::path out_file = fs::change_extension(*i, ".test0");
    PrintFormSpecial(IV.ledger(), out_file.string().c_str());
}

//============================================================================
void RegressionTest()
{
    fs::path test_dir(global_settings::instance().regression_test_directory);
    fs::directory_iterator i(test_dir);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        if(is_directory(*i))
            {
            continue;
            }
        else if(".cns" == fs::extension(*i))
            {
            RegressionTestOneCensusFile(i);
            }
        else if(".ini" == fs::extension(*i))
            {
            RegressionTestOneIniFile(i);
            }
        else
            {
            ; // Do nothing.
            }
        }
}

//============================================================================
void SelfTest()
{
    IllusVal IV;
    InputParms IP;
    IP.Status[0].Gender  = e_male;
    IP.Status[0].Smoking = e_nonsmoker;
    IP.Status[0].Class = e_preferred;
    IP.EePremium.assign(IP.SpecAmt.size(), r_pmt(10000.0));
    IP.SpecAmt.assign(IP.SpecAmt.size(), r_spec_amt(1000000.0));
    double expected_value = 0.0;
    double observed_value = 0.0;

    IP.SolveType = e_solve_none;
    expected_value = 29206514.78;
    IV.Run(&IP);
    observed_value = IV.ledger().GetCurrFull().AcctVal.back();
    if(.005 < std::fabs(expected_value - observed_value))
        {
        warning()
            << "Value should be "
            << value_cast<std::string>(expected_value)
            << ", but is "
            << value_cast<std::string>(observed_value)
            << " ."
            << LMI_FLUSH
            ;
        }

    IP.SolveType = e_solve_specamt;
    expected_value = 1827289;
    observed_value = IV.Run(&IP);
    if(.005 < std::fabs(expected_value - observed_value))
        {
        warning()
            << "Value should be "
            << value_cast<std::string>(expected_value)
            << ", but is "
            << value_cast<std::string>(observed_value)
            << " .\n"
            << LMI_FLUSH
            ;
        }

    IP.SolveType = e_solve_ee_prem;
    expected_value = 5498.99;
    observed_value = IV.Run(&IP);
    if(.005 < std::fabs(expected_value - observed_value))
        {
        warning()
            << "Value should be "
            << value_cast<std::string>(expected_value)
            << ", but is "
            << value_cast<std::string>(observed_value)
            << " .\n"
            << LMI_FLUSH
            ;
        }

// TODO ?? Temporarily suppress this test pending investigation of
// a recently-exposed problem.
#if 0
    multiple_cell_document census;
    std::vector<IllusInputParms> input_vector = census.cell_parms();
    input_vector.push_back(input_vector.front());
    std::ostream dev_null_os(0);
    RunCensus runner(dev_null_os);
    runner(input_vector);

std::cout << "? " << runner.XXXComposite.GetCurrFull().AcctVal.front() << std::endl;
std::cout << "? " << runner.XXXComposite.GetCurrFull().AcctVal.back() << std::endl;

std::cout << "? " << runner.XXXComposite.GetCurrFull().AcctVal[54] << std::endl;
std::cout << "? " << runner.XXXComposite.GetLedgerInvariant().GetInforceLives().front() << std::endl;
std::cout << "? " << runner.XXXComposite.GetLedgerInvariant().GetInforceLives().size() << std::endl;

    observed_value = runner.XXXComposite.GetLedgerInvariant().GrossPmt[0];
    expected_value = 12819.32;
    if(.005 < std::fabs(expected_value - observed_value))
        {
        warning()
            << "Value should be "
            << value_cast<std::string>(expected_value)
            << ", but is "
            << value_cast<std::string>(observed_value)
            << " .\n"
            << LMI_FLUSH
            ;
        }
#endif // 0

    std::cout
        << "Test solve speed: "
        << aliquot_timer(boost::bind(&IllusVal::Run, &IV, &IP), 5)
        << '\n'
        ;
}

//============================================================================
void Profile()
{
    for(int j = 0; j < 10; ++j)
        {
        SelfTest();
        }
}

//============================================================================
void process_command_line(int argc, char* argv[])
{
    int c;
    int digit_optind = 0;
    int this_option_optind = 1;
    int option_index = 0;
//    static char const* vfile[] = {"file", "archive", 0};
//    static char const* vlist[] = {"one", "two", "three", 0};
//    static char const* vopt[] = {"optional", "alternative", 0};
    static struct Option long_options[] =
      {
        {"ash_nazg"  ,NO_ARG   ,0 ,001 ,0 ,"ash nazg durbatulûk"},
        {"ash_naz"   ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"mellon"    ,NO_ARG   ,0 ,002 ,0 ,"pedo mellon a minno"},
        {"mello"     ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"help"      ,NO_ARG   ,0 ,'h' ,0 ,"display this help and exit"},
        {"license"   ,NO_ARG   ,0 ,'l' ,0 ,"display license and exit"},
        {"accept"    ,NO_ARG   ,0 ,'a' ,0 ,"accept license (-l to display)"},
        {"selftest"  ,NO_ARG   ,0 ,'s' ,0 ,"perform self test and exit"},
        {"profile"   ,NO_ARG   ,0 ,'p' ,0 ,"set up for profiling and exit"},
        {"illfile"   ,REQD_ARG ,0 ,'i' ,0 ,"run illustration"},
        {"cnsfile"   ,REQD_ARG ,0 ,'c' ,0 ,"run census"},
        {"data_path" ,REQD_ARG ,0 ,'d' ,0 ,"path to data files"},
        {"regress  " ,NO_ARG   ,0 ,'r' ,0 ,"run regression test"},
        {"test_path" ,REQD_ARG ,0 ,'t' ,0 ,"path to test files"},
//        {"list"    ,LIST_ARG, 0,   0, 0    , "list"},
//        {"opt"     ,OPT_ARG,  0,   0, 0    , "optional"},
//        {"alt"     ,ALT_ARG,  0,   0, 0    , "alternative"},
//        {"vfile"   ,REQD_ARG, 0,   0, vfile, "file type"},
//        {"vlist"   ,LIST_ARG, 0,   0, vlist, "list type"},
//        {"vopt"    ,OPT_ARG,  0,   0, vopt , "optional"},
//        {"valt"    ,ALT_ARG,  0,   0, vopt , "alternative"},
        {0         ,NO_ARG,   0,   0, 0    , ""}
      };
    bool license_accepted    = false;
    bool show_license        = false;
    bool show_help           = false;
    bool run_regression_test = false;
    bool run_selftest        = false;
    bool run_profile         = false;
    bool run_illustration    = false;
    bool run_census          = false;

    std::vector<std::string> ill_names;
    std::vector<std::string> cns_names;

    GetOpt getopt_long
        (argc
        ,argv
        ,"ac:hi:ls"
        ,long_options
        ,&option_index
        ,1
        );

    while(EOF != (c = getopt_long ()))
        {
        switch (c)
            {
            case 0:
                {
                char const* current_option = long_options[option_index].name;
                std::printf("option %s", current_option);
                if(getopt_long.optarg)
                    {
                    std::printf (" with arg %s", getopt_long.optarg);
                    }
                std::printf ("\n");
                }
                break;

            case 001:
                {
                global_settings::instance().ash_nazg = true;
                global_settings::instance().mellon = true;
                }
                break;

            case 002:
                {
                global_settings::instance().mellon = true;
                }
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                {
                if(digit_optind != 0 && digit_optind != this_option_optind)
                    {
                    std::printf ("digits occur in two different argv-elements.\n");
                    }
                digit_optind = this_option_optind;
                std::printf ("option %c\n", c);
                }
                break;

            case 'a':
                {
                license_accepted = true;
                }
                break;

            case 'b':
                {
                std::printf ("option b\n");
                }
                break;

            case 'c':
                {
                run_census = true;
                cns_names.push_back(getopt_long.optarg);
                }
                break;

            case 'd':
                {
                global_settings::instance().data_directory = getopt_long.optarg;
                }
                break;

            case 'h':
                {
                show_help = true;
                }
                break;

            case 'i':
                {
                run_illustration = true;
                ill_names.push_back(getopt_long.optarg);
                }
                break;

            case 'l':
                {
                show_license = true;
                }
                break;

            case 'o':
                {
                std::printf ("option o");
                if(getopt_long.optarg)
                    {
                    std::printf (" with value '%s'", getopt_long.optarg);
                    }
                std::printf ("\n");
                }
                break;

            case 'p':
                {
                run_profile = true;
                }
                break;

            case 'r':
                {
                run_regression_test = true;
                }
                break;

            case 's':
                {
                run_selftest = true;
                }
                break;

            case 't':
                {
                global_settings::instance().regression_test_directory = getopt_long.optarg;
                }
                break;

            case '?':
                {
                break;
                }

            default:
                {
                std::printf ("? getopt returned character code 0%o ?\n", c);
                }
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        std::printf ("non-option ARGV-elements: ");
        while (c < argc)
            {
            std::printf ("%s ", argv[c++]);
            }
        std::printf ("\n");
        }

    if(!license_accepted)
        {
        std::cerr << license_notices() << "\n\n";
        }

    if(show_license)
        {
        std::cerr << license_as_text() << "\n\n";
        return;
        }

    if(show_help)
        {
        getopt_long.usage();
        return;
        }

    if(run_selftest)
        {
        SelfTest();
        return;
        }

    if(run_regression_test)
        {
        RegressionTest();
        return;
        }

    if(run_profile)
        {
        Profile();
        return;
        }

    if(run_illustration)
        {
        RunIllustrationFromFile run_functor = std::for_each
            (ill_names.begin()
            ,ill_names.end()
            ,RunIllustrationFromFile(std::cout)
            );
        std::cerr
            << "File"
            << ((1U < ill_names.size()) ? "s" : "")
            << ":\n"
            ;
        std::copy
            (ill_names.begin()
            ,ill_names.end()
            ,std::ostream_iterator<std::string>(std::cerr, "\n")
            );
        std::cerr
            << "    Input:        "
            << 1000.0 * run_functor.time_for_input
            << " milliseconds\n"
            ;
        std::cerr
            << "    Calculations: "
            << 1000.0 * run_functor.time_for_calculations
            << " milliseconds\n"
            ;
        std::cerr
            << "    Output:       "
            << 1000.0 * run_functor.time_for_output
            << " milliseconds\n"
            ;
        }

    if(run_census)
        {
        std::for_each
            (cns_names.begin()
            ,cns_names.end()
// TODO ?? expunge            ,RunCensusDeprecated<std::string>()
            ,RunCensusDeprecated()
            );
        }
}

//============================================================================
int main(int argc, char* argv[])
{
    try
        {
        initialize_application(argc, argv);
        process_command_line(argc, argv);
        }
    catch(std::exception& e)
        {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
        }
    catch(...)
        {
        std::cerr << "Fatal exception: [no detail available]" << std::endl;
        return EXIT_FAILURE;
        }
}

