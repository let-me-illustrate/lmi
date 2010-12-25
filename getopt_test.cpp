// Get command-line options--unit test.
//
// Copyright (C) 1987, 1989 Free Software Foundation, Inc.
//
// (Modified by Douglas C. Schmidt for use with GNU G++.)
// (Modified by Vinicius J. Latorre for use with long options.)
// (Modified by Gregory W. Chicares in the years stated below:
//   substantive changes marked inline with 'GWC'; trivial changes
//   (formatting, style, standardization) not so marked, to reduce
//   clutter.)
//
// Latorre's original is here:
//   http://groups.google.com/groups?selm=9502241613.AA06287%40moon.cpqd.br
//
// This is a derived work. Any defect in it should not reflect on
// the reputations of Douglas C. Schmidt or Vinicius J. Latorre.
//
// GWC modifications are
//   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares
// and released with the same licensing terms as the original, viz.:

// This file is part of the GNU C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms of
// the GNU Library General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.  This library is distributed in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA

// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// GWC added this RCS Id:
// $Id$

// GWC added this to conform to LMI standards.
#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

// GWC changed filename to conform to LMI standard: lowercase, .hpp .
// #include <GetOpt.h>
#include "getopt.hpp"

#include "assert_lmi.hpp"
#include "miscellany.hpp" // lmi_array_size()
#include "test_tools.hpp"

#include <sstream>

struct getopt_test
{
    static std::string test(int, char*[]);
};

std::string getopt_test::test(int argc, char* argv[])
{
    // These preconditions are required by C++98 3.6.1/2 and also by
    // C99 5.1.2.2.1; violating them could cause a crash.
    LMI_ASSERT(0 <= argc);
    LMI_ASSERT(0 == argv[argc]);

    std::ostringstream oss;

    static char const* vfile[] = {"file", "archive", 0};
    static char const* vlist[] = {"one", "two", "three", 0};
    static char const* vopt [] = {"optional", "alternative", 0};
    static Option long_options[] =
        {
            {"add"     ,REQD_ARG ,0 ,  0 ,0     ,""},
            {"append"  ,NO_ARG   ,0 ,  0 ,0     ,""},
            {"delete"  ,REQD_ARG ,0 ,  0 ,0     ,""},
            {"verbose" ,NO_ARG   ,0 ,  0 ,0     ,""},
            {"create"  ,NO_ARG   ,0 ,  0 ,0     ,""},
            {"file"    ,REQD_ARG ,0 ,  0 ,0     ,""},
            {"list"    ,LIST_ARG ,0 ,  0 ,0     ,""},
            {"opt"     ,OPT_ARG  ,0 ,  0 ,0     ,""},
            {"alt"     ,ALT_ARG  ,0 ,  0 ,0     ,""},
            {"vfile"   ,REQD_ARG ,0 ,  0 ,vfile ,""},
            {"vlist"   ,LIST_ARG ,0 ,  0 ,vlist ,""},
            {"vopt"    ,OPT_ARG  ,0 ,  0 ,vopt  ,""},
            {"valt"    ,ALT_ARG  ,0 ,  0 ,vopt  ,""},
            {0         ,NO_ARG   ,0 ,  0 ,0     ,""}
        };
    int option_index = 0;
    GetOpt getopt_long
        (argc
        ,argv
        ,"abc:d:o::0123456789"
        ,long_options
        ,&option_index
        ,1
        );

    int digit_optind = 0;
    int this_option_optind = 1;
    int c;
    while(EOF != (c = getopt_long()))
        {
        switch(c)
            {
            case 0:
                {
                oss << "option " << long_options[option_index].name;
                if(getopt_long.optarg)
                    {
                    oss << " with arg " << getopt_long.optarg;
                    }
                oss << '\n';
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
                    oss << "digits occur in two different argv-elements.\n";
                    }
                digit_optind = this_option_optind;
                oss << "option " << static_cast<char>(c) << "\n";
                }
                break;

            case 'a':
                {
                oss << "option a\n";
                }
                break;

            case 'b':
                {
                oss << "option b\n";
                }
                break;

            case 'c':
                {
                oss << "option c with value '" << getopt_long.optarg << "'\n";
                }
                break;

            case 'd':
                {
                oss << "option d with value '" << getopt_long.optarg << "'\n";
                }
                break;

            case 'o':
                {
                oss << "option o";
                if(getopt_long.optarg)
                    {
                    oss << " with value '" << getopt_long.optarg << "'";
                    }
                oss << '\n';
                }
                break;

            case '?':
                {
                ; // Do nothing.
                }
                break;

            default:
                {
                oss << "? getopt returned character code " << c << " ?\n";
                }
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        oss << "non-option ARGV-elements: ";
        while(c < argc)
            {
            oss << argv[c++];
            }
        oss << '\n';
        }

    return oss.str();
}

// A set of simulated command-line options might be written thus:
//    char* test_argv[] = {"", "--verbose", "xyz", 0};
// but then the strings would be const, yet C99 5.1.2.2.1/2 requires
// that they be modifiable, and getopt assumes it can modify them.

int test_main(int, char*[])
{
    {
    char arg0[] = {""};
    char arg1[] = {"--verbose"};
    char arg2[] = {"xyz"};
    char* test_argv[] = {arg0, arg1, arg2, 0};
    int test_argc = -1 + static_cast<int>(lmi_array_size(test_argv));
    std::string s = getopt_test::test(test_argc, test_argv);
    BOOST_TEST_EQUAL(s, "option verbose\nnon-option ARGV-elements: xyz\n");
    }

    {
    char* test_argv[] = {0};
    int test_argc = -1 + static_cast<int>(lmi_array_size(test_argv));
    std::string s = getopt_test::test(test_argc, test_argv);
    BOOST_TEST_EQUAL(s, "");
    }

    {
    char arg0[] = {""};
    char arg1[] = {"-o"};
    char arg2[] = {"-d1,2,3"};
    char* test_argv[] = {arg0, arg1, arg2, 0};
    int test_argc = -1 + static_cast<int>(lmi_array_size(test_argv));
    std::string s = getopt_test::test(test_argc, test_argv);
    BOOST_TEST_EQUAL(s, "option o\noption d with value '1,2,3'\n");
    }

    return EXIT_SUCCESS;
}

