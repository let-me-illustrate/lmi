// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// This is a derived work based on Beman Dawes's boost test library
// that bears the following copyright and license statement:
// [Beman Dawes's copyright and license statement begins]
// (C) Copyright Beman Dawes 1995-2001. Permission to copy, use, modify, sell
// and distribute this software is granted provided this copyright notice
// appears in all copies. This software is provided "as is" without express or
// implied warranty, and with no claim as to its suitability for any purpose.
//
// See http://www.boost.org for updates, documentation, and revision history.
// [end Beman Dawes's copyright and license statement]
//
// Gregory W. Chicares modified it in 2004 to make it independent of
// the boost directory structure, and in any other ways indicated
// below, and in any later years shown above. Any defect in it should
// not reflect on Beman Dawes's reputation.

// The original boost.org test library upon which this derived work is
// based was later replaced by a very different library. That new test
// library has more features but is less transparent; it is not
// strictly compatible with tests written for the original library;
// and, for boost-1.31.0 at least, it didn't work out of the box with
// the latest como compiler. The extra features don't seem to be worth
// the cost.
//
// It seems unwise to mix boost versions, and better to extract this
// library from boost-1.23.0 and remove its dependencies on other
// parts of boost, which is easy to do because, as Beman Dawes says:
// Header dependencies are deliberately restricted to reduce coupling.

// boost/cpp_main.cpp (header or not, as you like it)  ----------------------//
//
// This file may be included as a header file, or may be compiled and placed
// in a library for traditional linking. It is unusual for non-template
// non-inline implementation code to be used as a header file, but the user
// may elect to do so because header-only implementation requires no library
// build support. (Suggested by Ed Brey)

#include "pchfile.hpp"

#include "catch_exceptions.hpp"
#include "getopt.hpp"
#include "license.hpp"
#include "main_common.hpp"

#include <cstdlib>                      // free()
#include <iostream>

#if defined LMI_MSW
#   include <fcntl.h>                   // _O_BINARY
#   include <io.h>                      // _setmode()
#   include <stdio.h>                   // _fileno()
#endif // defined LMI_MSW

#if defined __MINGW32__
/// Conform to C99 [7.19.6.1/8]:
///   "The exponent always contains at least two digits, and only as
///   many more digits as necessary to represent the exponent."
/// See:
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/28747
/// For the (corrected) return type, see:
///   https://lists.nongnu.org/archive/html/lmi/2011-06/msg00040.html

#   if defined __GNUC__ && 40600 <= LMI_GCC_VERSION
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wattributes"
#   endif // defined __GNUC__ && 40600 <= LMI_GCC_VERSION
extern "C" unsigned int _get_output_format(void) {return 1;}
#   if defined __GNUC__ && 40600 <= LMI_GCC_VERSION
#       pragma GCC diagnostic pop
#   endif // defined __GNUC__ && 40600 <= LMI_GCC_VERSION
#endif // defined __MINGW32__

// main()  ------------------------------------------------------------------//

// Uniformly detects and reports the occurrence of several types of errors,
// reducing the various errors to a uniform return value which is returned
// to the host environment.
//
// There are two intended uses:
//
//   * In production programs, which require no further action beyond
//     naming the top-level function cpp_main() instead of main().
//
//   * In test frameworks, which supply cpp_main() to detect (or catch)
//     test specific errors, report them,  and then return a presumably
//     non-zero value.
//
// Requires: A user-supplied cpp_main() function with same interface as main().
//
// Effects:
//
//   Call cpp_main( argc, argv ) in a try block.
//
//   Treat as errors:
//
//     *  exceptions from cpp_main().
//     *  non-zero return from cpp_main().
//
//   Report errors to both cout (with details) and cerr (summary).
//   Rationale: Detail error reporting goes to cout so that it is properly
//   interlaced with other output, thus aiding error analysis. Summary goes
//   to cerr in case cout is redirected.
//
// Returns: non-zero if any error was detected.
//
// Note: this header is designed to work well if boost test tools happen to be
// used, but there is no requirement that they be used. There are no
// dependencies on test_tools headers.

int cpp_main(int argc, char* argv[]);  // prototype for user's cpp_main()

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.

namespace lmi_test
{
  namespace test
  {
    class cpp_main_caller
    {
        int argc;
        char** argv;
      public:
        cpp_main_caller(int c, char** v) : argc(c), argv(v) {}
        int operator()() {return cpp_main(argc, argv);}
    };
  } // namespace test
} // namespace lmi_test

int main(int argc, char* argv[])
{
#if defined LMI_MSW
    // Force standard output streams to binary mode.
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif // defined LMI_MSW

    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(nullptr);

    // TRICKY !! Some long options are aliased to unlikely octal values.
    static Option long_options[] =
      {
        {"help",    NO_ARG, nullptr, 001, nullptr, "display this help and exit"},
        {"license", NO_ARG, nullptr, 002, nullptr, "display license and exit"},
        {"accept",  NO_ARG, nullptr, 003, nullptr, "accept license (-l to display)"},
        {nullptr,   NO_ARG, nullptr,   0, nullptr, ""}
      };

    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;

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
            case 001:
                {
                show_help = true;
                }
                break;

            case 002:
                {
                show_license = true;
                }
                break;

            case 003:
                {
                license_accepted = true;
                }
                break;

            case '?':
                {
                std::cerr << "Unrecognized option '";
                int offset = getopt_long.optind - 1;
                if(0 < offset)
                    {
                    std::cerr << getopt_long.nargv[offset];
                    }
                std::cerr << "'.\n";
                }
                break;

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
        std::cout << license_notices_as_text() << "\n\n";
        }

    if(show_license)
        {
        std::cout << license_as_text() << "\n\n";
        return EXIT_SUCCESS;
        }

    if(show_help)
        {
        getopt_long.usage();
        return EXIT_SUCCESS;
        }

    return lmi_test::catch_exceptions
        (lmi_test::test::cpp_main_caller(argc, argv)
        ,std::cout
        ,std::cerr
        );
}
