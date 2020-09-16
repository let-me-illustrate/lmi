// Life insurance illustrations: command-line interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "dbdict.hpp"                   // print_databases()
#include "getopt.hpp"
#include "global_settings.hpp"
#include "gpt_server.hpp"
#include "handle_exceptions.hpp"        // report_exception()
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
#include "path.hpp"
#include "path_utility.hpp"
#include "so_attributes.hpp"
#include "timer.hpp"
#include "value_cast.hpp"
#include "verify_products.hpp"

#include <algorithm>                    // for_each()
#include <cmath>                        // fabs()
#include <cstdio>                       // printf()
#include <functional>                   // bind()
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
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

    Input naic_no_solve;
    naic_no_solve["ProductName"       ] = "sample2naic";
    naic_no_solve["SolveType"         ] = "No solve";
    naic_no_solve["Gender"            ] = "Male";
    naic_no_solve["Smoking"           ] = "Nonsmoker";
    naic_no_solve["UnderwritingClass" ] = "Standard";
    naic_no_solve["GeneralAccountRate"] = "0.06";
    naic_no_solve["Payment"           ] = "20000.0";
    naic_no_solve["SpecifiedAmount"   ] = "1000000.0";
    naic_no_solve["SolveToWhich"      ] = "Maturity";
    naic_no_solve.RealizeAllSequenceInput();

    double expected_value = 0.0;
    double observed_value = 0.0;

    expected_value = 6305652.52;
    z("CLI_selftest", naic_no_solve);
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

    Input naic_solve_specamt {naic_no_solve};
    naic_solve_specamt["SolveType"] = "Specified amount";
    expected_value = 1879139.14;
    z("CLI_selftest", naic_solve_specamt);
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

    Input naic_solve_ee_prem {naic_no_solve};
    naic_solve_ee_prem["SolveType"] = "Employee premium";
    expected_value = 10673.51;
    z("CLI_selftest", naic_solve_ee_prem);
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

    Input finra_no_solve      {naic_no_solve};
    Input finra_solve_specamt {naic_solve_specamt};
    Input finra_solve_ee_prem {naic_solve_ee_prem};
    finra_no_solve     ["ProductName"] = "sample2finra";
    finra_solve_specamt["ProductName"] = "sample2finra";
    finra_solve_ee_prem["ProductName"] = "sample2finra";

#if defined _GLIBCXX_DEBUG
    std::cout << "Timing test skipped: takes too long in debug mode" << std::endl;
#else  // !defined _GLIBCXX_DEBUG
    std::cout
        << "Test speed:"
        << "\n  naic, no solve      : "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", naic_no_solve))
        << "\n  naic, specamt solve : "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", naic_solve_specamt))
        << "\n  naic, ee prem solve : "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", naic_solve_ee_prem))
        << "\n  finra, no solve     : "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", finra_no_solve))
        << "\n  finra, specamt solve: "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", finra_solve_specamt))
        << "\n  finra, ee prem solve: "
        << TimeAnAliquot(std::bind(z, "CLI_selftest", finra_solve_ee_prem))
        << std::endl
        ;
#endif // !defined _GLIBCXX_DEBUG
}

/// Validate products.
///
/// Run an illustration for every product in every state (whether
/// approved there or not), reporting any conflict in parameters
/// that would make that impossible. See:
///   https://lists.nongnu.org/archive/html/lmi/2020-11/msg00020.html

void product_test()
{
    // Allow unapproved states.
    global_settings::instance().set_regression_testing(true);

    // Pay zero and don't solve, to make this test go faster.
    Input input;
    input["Payment"           ] = "0.0";
    input["SolveType"         ] = "No solve";

    illustrator z(mce_emit_nothing);

    ce_product_name c;
    std::vector<std::string> const& p = c.all_strings();
    std::vector<std::string> const& s = all_strings_state();
    for(auto const& i : p)
        {
        std::cout << "Testing product " << i << std::endl;
        input["ProductName"        ] = i;
        for(auto const& j : s)
            {
            input["StateOfJurisdiction"] = j;
            try
                {
                z("eraseme", input);
                }
            catch(...)
                {
                std::cout << i << ", " << j << ":" << std::endl;
                report_exception();
                }
            }
        }
}

void process_command_line(int argc, char* argv[])
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
    static Option long_options[] =
      {
        {"ash_nazg"     ,NO_ARG   ,nullptr ,001 ,nullptr ,"ash nazg durbatulûk"},
        {"ash_naz"      ,NO_ARG   ,nullptr ,077 ,nullptr ,"fraud"},
        {"mellon"       ,NO_ARG   ,nullptr ,002 ,nullptr ,"pedo mellon a minno"},
        {"mello"        ,NO_ARG   ,nullptr ,077 ,nullptr ,"fraud"},
        {"prospicience" ,REQD_ARG ,nullptr ,003 ,nullptr ,"validation date"},
        {"accept"       ,NO_ARG   ,nullptr ,'a' ,nullptr ,"accept license (-l to display)"},
        {"data_path"    ,REQD_ARG ,nullptr ,'d' ,nullptr ,"path to data files"},
        {"emit"         ,REQD_ARG ,nullptr ,'e' ,nullptr ,"choose what output to emit"},
        {"file"         ,REQD_ARG ,nullptr ,'f' ,nullptr ,"input file to run"},
        {"help"         ,NO_ARG   ,nullptr ,'h' ,nullptr ,"display this help and exit"},
        {"license"      ,NO_ARG   ,nullptr ,'l' ,nullptr ,"display license and exit"},
        {"product_test" ,NO_ARG   ,nullptr ,'o' ,nullptr ,"validate products and exit"},
        {"print_db"     ,NO_ARG   ,nullptr ,'p' ,nullptr ,"print products and exit"},
        {"selftest"     ,NO_ARG   ,nullptr ,'s' ,nullptr ,"perform self test and exit"},
        {"test_db"      ,NO_ARG   ,nullptr ,'t' ,nullptr ,"test products and exit"},
        {"pyx"          ,REQD_ARG ,nullptr ,'x' ,nullptr ,"for docimasy"},
        {nullptr        ,NO_ARG   ,nullptr ,000 ,nullptr ,""}
      };

    bool license_accepted    = false;

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

            case 003:
                {
                std::istringstream iss(getopt_long.optarg);
                int ymd_as_int;
                iss >> ymd_as_int;
                if(!iss || !iss.eof())
                    {
                    warning() << "Invalid prospicience option value '"
                              << getopt_long.optarg
                              << "' (must be in YYYYMMDD format)."
                              << std::flush
                              ;
                    }
                else
                    {
                    global_settings::instance().set_prospicience_date
                        (calendar_date(ymd_t(ymd_as_int))
                        );
                    }
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
                std::string const e = fs::path{s}.extension().string();
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
                getopt_long.usage();
                std::cout << "Suboptions for '--emit':\n";
                for(auto const& i : allowed_strings_emission())
                    {
                    std::cout << "  " << i << '\n';
                    }
                return;
                }
                break;

            case 'l':
                {
                std::cerr << license_as_text() << "\n\n";
                return;
                }
                break;

            case 'o':
                {
                product_test();
                return;
                }
                break;

            case 'p':
                {
                print_databases();
                return;
                }
                break;

            case 's':
                {
                self_test();
                return;
                }
                break;

            case 't':
                {
                verify_products();
                return;
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
    process_command_line(argc, argv);
    return EXIT_SUCCESS;
}
