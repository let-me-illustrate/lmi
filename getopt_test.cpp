// Get command-line options--unit test.
//
// Copyright (C) 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: getopt_test.cpp,v 1.1 2007-05-03 16:33:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "getopt.hpp"

#include "test_tools.hpp"

struct getopt_test
{
    static int test(int, char*[]);
};

int getopt_test::test(int argc, char* argv[])
{
  int c;
  int digit_optind = 0;
  int this_option_optind = 1;
  int option_index = 0;
  static char const* vfile[] = {"file", "archive", 0};
  static char const* vlist[] = {"one", "two", "three", 0};
  static char const* vopt[] = {"optional", "alternative", 0};
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
  GetOpt  getopt_long (argc, argv, "abc:d:o::0123456789",
                       long_options, &option_index, 1);

  while ((c = getopt_long ()) != EOF)
    {
      switch (c)
        {
        case 0:
          std::printf ("option %s", long_options[option_index].name);
          if (getopt_long.optarg)
            std::printf (" with arg %s", getopt_long.optarg);
          std::printf ("\n");
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
          if (digit_optind != 0 && digit_optind != this_option_optind)
            std::printf ("digits occur in two different argv-elements.\n");
          digit_optind = this_option_optind;
          std::printf ("option %c\n", c);
          break;

        case 'a':
          std::printf ("option a\n");
          break;

        case 'b':
          std::printf ("option b\n");
          break;

        case 'c':
          std::printf ("option c with value '%s'\n", getopt_long.optarg);
          break;

        case 'd':
          std::printf ("option d with value '%s'\n", getopt_long.optarg);
          break;

        case 'o':
          std::printf ("option o");
          if (getopt_long.optarg)
            std::printf (" with value '%s'", getopt_long.optarg);
          std::printf ("\n");
          break;

        case '?':
          break;

        default:
          std::printf ("? getopt returned character code 0%o ?\n", c);
        }
    }

  if ((c = getopt_long.optind) < argc)
    {
      std::printf ("non-option ARGV-elements: ");
      while (c < argc)
        std::printf ("%s ", argv[c++]);
      std::printf ("\n");
    }

  std::exit (0);
}

int test_main(int, char*[])
{
    int test_argc = 0;
    char* test_argv[] = {NULL};
    getopt_test::test(test_argc, test_argv);
    return EXIT_SUCCESS;
}

