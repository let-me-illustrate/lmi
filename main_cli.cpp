// Life insurance illustrations: command-line interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "dbdict.hpp"       // print_databases()
#include "getopt.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "license.hpp"
#include "lmi.hpp"          // is_antediluvian_fork()
#include "main_common.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "mec_server.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "so_attributes.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef> // NULL, std::size_t
#include <cstdio>  // std::printf(), std::remove()
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

/// Run a suite of test cases.
///
/// Run every file with extension
///   '.cns', '.ini', or '.mec'
/// in a given system-testing directory, emitting data appropriate for
/// automated comparison with previously-saved results.

void system_test()
{
    Timer timer;
    global_settings::instance().set_regression_testing(true);
    fs::path test_dir(global_settings::instance().regression_test_directory());
    fs::directory_iterator i(test_dir);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        try
            {
            if(is_directory(*i))
                {
                continue;
                }
            else if(".cns" == fs::extension(*i))
                {
                std::cout << "Regression testing: " << i->string() << std::endl;
                (illustrator(mce_emit_test_data))(*i);
                }
            else if(".ini" == fs::extension(*i))
                {
                std::cout << "Regression testing: " << i->string() << std::endl;
                (illustrator(mce_emit_custom_0 ))(*i);
                }
            else if(".mec" == fs::extension(*i))
                {
                std::cout << "Regression testing: " << i->string() << std::endl;
                (mec_server (mce_emit_test_data))(*i);
                }
            else
                {
                // Do nothing. The test directory typically contains
                // many files of other types that are deliberately
                // ignored.
                }
            }
        catch(...)
            {
            report_exception();
            }
        }
    std::cout << "system_test(): " << timer.stop().elapsed_msec_str() << std::endl;
}

/// Spot check and time some insurance calculations.
///
/// The antediluvian fork's calculated results don't match the
/// production system's, so no assertions are made about them; but the
/// speed difference is interesting.

void self_test()
{
    bool const antediluvian = is_antediluvian_fork();

    illustrator z(mce_emit_nothing);

    Input IP;
    IP["Gender"            ] = "Male";
    IP["Smoking"           ] = "Nonsmoker";
    IP["UnderwritingClass" ] = "Standard";
    IP["GeneralAccountRate"] = "0.06";
    IP["Payment"           ] = "20000.0";
    IP["SpecifiedAmount"   ] = "1000000.0";
    IP.RealizeAllSequenceInput();

    double expected_value = 0.0;
    double observed_value = 0.0;

    IP["SolveType"] = "No solve";
    expected_value = 6305652.52;
    z("", IP);
    observed_value = z.principal_ledger()->GetCurrFull().AcctVal.back();
    if(!antediluvian && .005 < std::fabs(expected_value - observed_value))
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

    IP["DeprecatedSolveToWhich"] = "Maturity";

    IP["SolveType"] = "Specified amount";
    expected_value = 1879139.14;
    z("", IP);
    observed_value = z.principal_ledger()->GetCurrFull().AcctVal.back();
    if(!antediluvian && .005 < std::fabs(expected_value - observed_value))
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

    IP["SolveType"] = "Employee premium";
    expected_value = 10673.51;
    z("", IP);
    observed_value = z.principal_ledger()->GetLedgerInvariant().EeGrossPmt.front();
    if(!antediluvian && .005 < std::fabs(expected_value - observed_value))
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

    std::cout
        << "Test solve speed: "
        << TimeAnAliquot(boost::bind(z, "", IP), 0.1)
        << '\n'
        ;
}

/// Run self-test repeatedly (intended for use with 'gprof').

void profile()
{
    for(int j = 0; j < 10; ++j)
        {
        self_test();
        }
}

void process_command_line(int argc, char* argv[])
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
//    static char const* vfile[] = {"file", "archive", 0};
//    static char const* vlist[] = {"one", "two", "three", 0};
//    static char const* vopt[] = {"optional", "alternative", 0};
    static Option long_options[] =
      {
        {"ash_nazg"  ,NO_ARG   ,0 ,001 ,0 ,"ash nazg durbatulûk"},
        {"ash_naz"   ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"mellon"    ,NO_ARG   ,0 ,002 ,0 ,"pedo mellon a minno"},
        {"mello"     ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"pyx"       ,REQD_ARG ,0 ,'x' ,0 ,"for docimasy"},
        {"help"      ,NO_ARG   ,0 ,'h' ,0 ,"display this help and exit"},
        {"license"   ,NO_ARG   ,0 ,'l' ,0 ,"display license and exit"},
        {"accept"    ,NO_ARG   ,0 ,'a' ,0 ,"accept license (-l to display)"},
        {"selftest"  ,NO_ARG   ,0 ,'s' ,0 ,"perform self test and exit"},
        {"profile"   ,NO_ARG   ,0 ,'o' ,0 ,"set up for profiling and exit"},
        {"emit"      ,REQD_ARG ,0 ,'e' ,0 ,"choose what output to emit"},
        {"illfile"   ,REQD_ARG ,0 ,'i' ,0 ,"run illustration"},
        {"cnsfile"   ,REQD_ARG ,0 ,'c' ,0 ,"run census"},
//      {"gptfile"   ,REQD_ARG ,0 ,'g' ,0 ,"test GPT"}, // Reserved for future use.
        {"mecfile"   ,REQD_ARG ,0 ,'m' ,0 ,"test MEC testing"},
        {"data_path" ,REQD_ARG ,0 ,'d' ,0 ,"path to data files"},
        {"print_db"  ,NO_ARG   ,0 ,'p', 0, "print product databases"},
        {"regress"   ,NO_ARG   ,0 ,'r' ,0 ,"run regression test"},
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
    bool print_all_databases = false;
    bool run_illustration    = false;
    bool run_census          = false;
    bool run_mec_test        = false;

    e_emission emission(mce_emit_nothing);
    // Suppress enumerators for options not fully implemented.
    emission.allow(emission.ordinal("emit_pdf_to_printer"), false);
    emission.allow(emission.ordinal("emit_pdf_to_viewer" ), false);
    emission.allow(emission.ordinal("emit_custom_0"      ), false);

    std::vector<std::string> ill_names;
    std::vector<std::string> cns_names;
    std::vector<std::string> mec_names;

    int digit_optind = 0;
    int this_option_optind = 1;
    int option_index = 0;
    GetOpt getopt_long
        (argc
        ,argv
        ,""
        ,long_options
        ,&option_index
        ,true
        );

    int c;
    while(EOF != (c = getopt_long()))
        {
        switch(c)
            {
            case 0:
                {
                char const* current_option = long_options[option_index].name;
                std::printf("option %s", current_option);
                if(getopt_long.optarg)
                    {
                    std::printf(" with arg %s", getopt_long.optarg);
                    }
                std::printf("\n");
                }
                break;

            case 001:
                {
                global_settings::instance().set_ash_nazg(true);
                }
                break;

            case 002:
                {
                global_settings::instance().set_mellon(true);
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
                    std::printf("digits occur in two different argv-elements.\n");
                    }
                digit_optind = this_option_optind;
                std::printf("option %c\n", c);
                }
                break;

            case 'a':
                {
                license_accepted = true;
                }
                break;

            case 'b':
                {
                std::printf("option b\n");
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
                global_settings::instance().set_data_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case 'e':
                {
                int emission_suboptions = mce_emit_nothing;

                LMI_ASSERT(NULL != getopt_long.optarg);
                std::string const s(getopt_long.optarg);
                std::istringstream iss(s);
                for(;EOF != iss.peek();)
                    {
                    std::string token;
                    std::getline(iss, token, ',');
                    if(!token.empty())
                        {
                        try
                            {
                            e_emission z(token);
                            if(!emission.is_allowed(emission.ordinal(token)))
                                {
                                throw std::runtime_error(token);
                                }
                            emission_suboptions |= z.value();
                            }
                        catch(std::runtime_error const&)
                            {
                            std::cerr
                                << argv[0]
                                << ": unrecognized '--emit' suboption "
                                << "'" << token << "'"
                                << std::endl
                                ;
                            }
                        }
                    }
                emission = mcenum_emission(emission_suboptions);
                }
                break;

//          case 'g': // Reserved for future use.

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

            case 'm':
                {
                run_mec_test = true;
                mec_names.push_back(getopt_long.optarg);
                }
                break;

            case 'o':
                {
                run_profile = true;
                }
                break;

            case 'p':
                {
                print_all_databases = true;
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
                global_settings::instance().set_regression_test_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case 'x':
                {
                global_settings::instance().set_pyx(getopt_long.optarg);
                if(contains(global_settings::instance().pyx(), "system_testing"))
                    {
                    global_settings::instance().set_regression_testing(true);
                    }
                }
                break;

            case '?':
                {
                break;
                }

            default:
                {
                std::cerr << "Unrecognized option character '" << c << "'.\n";
                }
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        std::cerr << "Unrecognized parameters:\n";
        while(c < argc)
            {
            std::cerr << "  '" << argv[c++] << "'\n";
            }
        std::cerr << std::endl;
        }

    if(!license_accepted)
        {
        std::cerr << license_notices_as_text() << "\n\n";
        }

    if(show_license)
        {
        std::cerr << license_as_text() << "\n\n";
        return;
        }

    if(show_help)
        {
        getopt_long.usage();

        std::cout << "Suboptions for '--emit':\n";
        for(std::size_t j = 0; j < emission.cardinality(); ++j)
            {
            if(emission.is_allowed(j))
                {
                std::cout << "  " << emission.str(j) << '\n';
                }
            }

        return;
        }

    if(run_selftest)
        {
        self_test();
        return;
        }

    if(run_regression_test)
        {
        system_test();
        return;
        }

    if(run_profile)
        {
        profile();
        return;
        }

    if(print_all_databases)
        {
        print_databases();
        return;
        }

    if(run_illustration)
        {
        std::for_each
            (ill_names.begin()
            ,ill_names.end()
            ,illustrator(emission.value())
            );
        }

    if(run_census)
        {
        std::for_each
            (cns_names.begin()
            ,cns_names.end()
            ,illustrator(emission.value())
            );
        }

    if(run_mec_test)
        {
        std::for_each
            (mec_names.begin()
            ,mec_names.end()
            ,mec_server(emission.value())
            );
        }
}

int try_main(int argc, char* argv[])
{
    initialize_filesystem();
    process_command_line(argc, argv);
    return EXIT_SUCCESS;
}

