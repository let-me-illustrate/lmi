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

// $Id: main.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"
#include "argv0.hpp"
#include "calculate.hpp"
#include "fenv_lmi.hpp"
#include "getopt.hpp"
#include "global_settings.hpp"
#include "license.hpp"
#include "miscellany.hpp"
#include "sigfpe.hpp"
#include "value_cast_ihs.hpp"

#include <algorithm>
#include <csignal>
#include <cstdlib> // std::free()
#include <exception>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

//============================================================================
void SelfTest()
{
    IllusVal IV;
    InputParms IP;
    IP.Status[0].Gender  = e_male;
    IP.Status[0].Smoking = e_nonsmoker;
    IP.Status[0].Class = e_preferred;
    IP.SpecAmt.assign(IP.SpecAmt.size(), r_spec_amt(1000000.0));

    Timer timer;
    int const n = 100;
    double z = 0.0;
    for(int j = 0; j < n; j++)
        {
        z = IV.Run(IP);
        }
    timer.Stop();
    double const expected_value = 5498.99;
    if(.005 < std::fabs(expected_value - z))
        {
        std::ostringstream error;
        error
            << "Value should be "
            << expected_value
            << ", but is "
            << value_cast_ihs<std::string>(z)
            << " .\n"
            ;
        throw std::runtime_error(error.str());
        }
    std::cout << "    Time for " << n << " solves: " << timer.Report() << '\n';

    // These run times are measured at the command line. Timings in file
    // 'statistics.txt' are run from make and are generally higher,
    // probably due to make's overhead; bc++5.02 timings are for IDE build.
    //
    // msec for 100 iterations, median of three runs:
    // bc++5.02 gcc-2.95.2-1         date
    //    16065        -----   2001-02-27
    //    16338        27028   2001-03-02
    //    14187        22549   2001-03-02  skip checksums in SOA table manager
    //    14304        19268   2001-03-02  gcc didn't have maximum optimization
    //    14097        13947   2001-03-03  use sgi power() template (gcc only)
    //    13874        13643   2001-03-06
    //    14294        13674   2001-03-10
    //    14470        13914   2001-03-10  BIDS -> vector in SOA tablfile.hpp
    //    14313        13689   2001-03-14
    //    13898        13291   2001-03-16
    //    13750        13348   2001-03-20  bc++5.02 cstring.h -> std::string
    //    13954        13048   2001-03-24  console program
    //    14621        13129   2001-03-26  use std::string in inputs class
    //    14652        13062   2001-03-27
    //    14426        12791   2001-04-10
    //    14294        12881   2001-04-12
    //    14255        12882   2001-04-19
    //    11022         8605   2001-06-18 faster rounding

    //
    // build times: max optimization
    //   bc++5.02  gcc-2.95.2-1      date
    //  GUI console GUI console
    //  77          893        2001-03-16
    //  72          902        2001-03-20
    //  76      7   873   143  2001-03-24
    //  80      8   891   166  2001-03-28
    //  76      8              2001-04-12  added three headers to pch.hpp
    //  85      8              2001-04-19
    //  75      8              2001-04-20
    //  76      8         170  2001-05-15
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
int main(int argc, char* argv[])
{
    // Set boost filesystem default name check function to native. Its
    // facilities are used with names the user controls, and users
    // may specify names that are not portable. The default name check
    // function should be set before using this boost library, to
    // ensure that it's used uniformly.
    fs::path::default_name_check(fs::native);

    fs::path argv0(argv[0]);

    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(0);

    // TODO ?? Instead, consider a singleton that checks the control
    // word upon destruction?
    initialize_fpu();

    if(SIG_ERR == std::signal(SIGFPE, (void(*)(int))floating_point_error_handler))
        {
        warning() << "Cannot install floating point error signal handler.\n";
        return EXIT_FAILURE;
        }

// TODO ?? Factor out the getopt stuff, into a new function like this:
//    process_command_line(argc, argv);

    int c;
    int digit_optind = 0;
    int this_option_optind = 1;
    int option_index = 0;
//    static char const* vfile[] = {"file", "archive", 0};
//    static char const* vlist[] = {"one", "two", "three", 0};
//    static char const* vopt[] = {"optional", "alternative", 0};
    static struct Option long_options[] =
      {
//        {"do"      ,REQD_ARG, 0,   0, 0    , "do something"},
        {"help"    ,NO_ARG,   0, 'h', 0    , "display this help and exit"},
//        {"delete"  ,REQD_ARG, 0,   0, 0    , "delete something"},
        {"license" ,NO_ARG,   0, 'l', 0    , "display license and exit"},
        {"accept"  ,NO_ARG,   0, 'a', 0    , "accept license (-l to display)"},
        {"selftest",NO_ARG,   0, 's', 0    , "perform self test and exit"},
        {"profile" ,NO_ARG,   0, 'p', 0    , "set up for profiling and exit"},
        {"illfile" ,REQD_ARG, 0, 'i', 0    , "run illustration"},
        {"cnsfile" ,REQD_ARG, 0, 'c', 0    , "run census"},
//        {"list"    ,LIST_ARG, 0,   0, 0    , "list"},
//        {"opt"     ,OPT_ARG,  0,   0, 0    , "optional"},
//        {"alt"     ,ALT_ARG,  0,   0, 0    , "alternative"},
//        {"vfile"   ,REQD_ARG, 0,   0, vfile, "file type"},
//        {"vlist"   ,LIST_ARG, 0,   0, vlist, "list type"},
//        {"vopt"    ,OPT_ARG,  0,   0, vopt , "optional"},
//        {"valt"    ,ALT_ARG,  0,   0, vopt , "alternative"},
        {0         ,NO_ARG,   0,   0, 0    , ""}
      };
    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;
    bool run_selftest     = false;
    bool run_profile      = false;
    bool run_illustration = false;
    bool run_census       = false;

    std::vector<std::string> ill_names;
    std::vector<std::string> cns_names;

    try
        {
        std::signal(SIGFPE, (void(*)(int))floating_point_error_handler);

//        GetOpt  getopt_long (argc, argv, "abc:d:o::0123456789",
        GetOpt  getopt_long (argc, argv, "ac:hi:ls",
                             long_options, &option_index, 1);

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
                std::printf ("option d with value '%s'\n", getopt_long.optarg);
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

              case 's':
                {
                run_selftest = true;
                }
                break;

              case 'p':
                {
                run_profile = true;
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

              case '?':
                break;

              default:
                std::printf ("? getopt returned character code 0%o ?\n", c);
              }
          }

        if((c = getopt_long.optind) < argc)
          {
            std::printf ("non-option ARGV-elements: ");
            while (c < argc)
              std::printf ("%s ", argv[c++]);
            std::printf ("\n");
          }

        if(!license_accepted)
            {
            std::cerr << license_notices() << "\n\n";
            }

        if(show_license)
            {
            std::cerr << license_as_text() << "\n\n";
            return EXIT_SUCCESS;
            }

        if(show_help)
            {
            getopt_long.usage();
            return EXIT_SUCCESS;
            }

        if(run_selftest)
            {
            SelfTest();
            return EXIT_SUCCESS;
            }

        if(run_profile)
            {
            Profile();
            return EXIT_SUCCESS;
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
                (ill_names.begin()
                ,ill_names.end()
// TODO ?? expunge                ,RunCensusDeprecated<std::string>()
                ,RunCensusDeprecated()
                );
            }
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

