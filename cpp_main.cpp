// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: cpp_main.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "catch_exceptions.hpp"
#include "getopt.hpp"
#include "license.hpp"

#include <cstdlib>  // std::free()
#include <iostream>

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
  }
} // Namespace lmi_test.

int main(int argc, char* argv[])
{
    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(0);

    // Long options are meta-options not directly supported by the target
    // compiler. TRICKY !! They are aliased to unlikely octal values.
    static struct Option long_options[] =
      {
        {"help",         NO_ARG,   0, 001, 0, "display this help and exit"},
        {"license",      NO_ARG,   0, 002, 0, "display license and exit"},
        {"accept",       NO_ARG,   0, 003, 0, "accept license (-l to display)"},
        {0,              NO_ARG,   0,   0, 0, ""}
      };

    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;

    int c;
    int option_index = 0;
    GetOpt getopt_long
        (argc
        ,argv
        ,""
        ,long_options
        ,&option_index
        ,true
        );
    getopt_long.opterr = false;
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
                std::cerr << "Unrecognized option ";
                int offset = getopt_long.optind - 1;
                if(0 < offset)
                    {
                    std::cerr << getopt_long.nargv[offset];
                    }
                std::cerr << '\n';
                }
                break;

            default:
                std::cerr << "getopt returned character code 0" << c << '\n';
            }
        }

    if(!license_accepted)
        {
        std::cout << license_notices() << "\n\n";
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

// Revision History
//  2004-10-20 GWC Rename namespace.
//  2004-08-03 GWC Add getopt support to display GPL and the notices
//    it requires.
//  2004-05-06 GWC Add code to force mpatrol to link.
//  2004-05-06 GWC Remove carriage returns.
//  2004-05-05 GWC Extract original library from boost-1.23.0, remove
//    dependencies on other parts of boost, and adapt to lmi conventions.
//  26 Feb 01 Numerous changes suggested during formal review. (Beman)
//  25 Jan 01 catch_exceptions.hpp code factored out.
//  22 Jan 01 Remove test_tools dependencies to reduce coupling.
//   5 Nov 00 Initial boost version (Beman Dawes)

