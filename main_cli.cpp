// Life insurance illustrations: command-line interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "dbdict.hpp"                   // print_databases()
#include "getopt.hpp"
#include "global_settings.hpp"
#include "gpt_server.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "license.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "main_common.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "mc_enum_types_aux.hpp"        // allowed_strings_emission(), mc_emission_from_string()
#include "mec_server.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "so_attributes.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>                       // std::printf()
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

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
    z("CLI_selftest", IP);
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

    IP["SolveToWhich"] = "Maturity";

    IP["SolveType"] = "Specified amount";
    expected_value = 1879139.14;
    z("CLI_selftest", IP);
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
    z("CLI_selftest", IP);
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
        << TimeAnAliquot(boost::bind(z, "CLI_selftest", IP), 0.1)
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
        {"file"      ,REQD_ARG ,0 ,'f' ,0 ,"input file to run"},
        {"data_path" ,REQD_ARG ,0 ,'d' ,0 ,"path to data files"},
        {"print_db"  ,NO_ARG   ,0 ,'p' ,0 ,"print product databases and exit"},
        {0           ,NO_ARG   ,0 ,0   ,0 ,""}
      };

    bool license_accepted    = false;
    bool show_license        = false;
    bool show_help           = false;
    bool run_selftest        = false;
    bool run_profile         = false;
    bool print_all_databases = false;

    mcenum_emission emission(mce_emit_nothing);

    std::vector<std::string> illustrator_names;
    std::vector<std::string> mec_server_names;
    std::vector<std::string> gpt_server_names;

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

            case 'd':
                {
                global_settings::instance().set_data_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case 'e':
                {
                LMI_ASSERT(nullptr != getopt_long.optarg);
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
                            emission = mcenum_emission
                                ( emission
                                | mc_emission_from_string(token)
                                );
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
                }
                break;

            case 'f':
                {
                LMI_ASSERT(nullptr != getopt_long.optarg);
                std::string const s(getopt_long.optarg);
                std::string const e = fs::extension(s);
                if(".cns" == e || ".ill" == e || ".ini" == e || ".inix" == e)
                    {
                    illustrator_names.push_back(getopt_long.optarg);
                    }
                else if(".mec" == e)
                    {
                    mec_server_names.push_back(getopt_long.optarg);
                    }
                else if(".gpt" == e)
                    {
                    gpt_server_names.push_back(getopt_long.optarg);
                    }
                else
                    {
                    warning()
                        << "'"
                        << s
                        << "': unrecognized file extension."
                        << LMI_FLUSH
                        ;
                    }
                }
                break;

            case 'h':
                {
                show_help = true;
                }
                break;

            case 'l':
                {
                show_license = true;
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

            case 's':
                {
                run_selftest = true;
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
        std::vector<std::string> const& v = allowed_strings_emission();
        typedef std::vector<std::string>::const_iterator vsi;
        for(vsi i = v.begin(); i != v.end(); ++i)
            {
            std::cout << "  " << *i << '\n';
            }
        return;
        }

    if(run_selftest)
        {
        self_test();
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

    std::for_each
        (illustrator_names.begin()
        ,illustrator_names.end()
        ,illustrator(emission)
        );

    std::for_each
        (mec_server_names.begin()
        ,mec_server_names.end()
        ,mec_server(emission)
        );

    std::for_each
        (gpt_server_names.begin()
        ,gpt_server_names.end()
        ,gpt_server(emission)
        );
}

int try_main(int argc, char* argv[])
{
    initialize_filesystem();
    process_command_line(argc, argv);
    return EXIT_SUCCESS;
}

