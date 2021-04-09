// Get command-line options.
//
// Copyright (C) 1987, 1989, 1992 Free Software Foundation, Inc.
//
// (Modified by Douglas C. Schmidt for use with GNU G++ (Getopt).)
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
//   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares
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

// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// This version of 'getopt' appears to the caller like standard Unix 'getopt'
// but it behaves differently for the user, since it allows the user
// to intersperse the options with the other arguments.

// As 'getopt' works, it permutes the elements of 'argv' so that,
// when it is done, all the options precede everything else.  Thus
// all application programs are extended to handle flexible argument order.

// Setting the environment variable _POSIX_OPTION_ORDER disables permutation.
// Then the behavior is completely standard.

// GNU application programs can use a third alternative mode in which
// they can distinguish the relative order of options and other arguments.

#ifndef getopt_hpp
#define getopt_hpp

// GWC removed obsolete '#pragma interface'.

// GWC added this to conform to lmi standards.
#include "config.hpp"

// GWC added this.
#include "so_attributes.hpp"

// GWC suppressed nonstandard <std.h> .
//#include <std.h>
#include <cstdio>
// GWC added header for EXIT_SUCCESS.
#include <cstdlib>
// GWC added <iosfwd>
#include <iosfwd>
// GWC changed nonstandard to standard string class.
//#include <String.h>
#include <string>

// Describe the long-named options requested by the application.
// The LONG_OPTIONS argument below is a vector of 'struct option'
// terminated by an element containing a name which is zero.

// The field 'has_arg' is:
// NO_ARG          if the option does not take an argument,
// REQD_ARG        if the option requires an argument
//                 (accepts the following syntax:
//                 -option=value
//                 -option value),
// OPT_ARG         if the option takes an optional argument
//                 (accepts the following syntax:
//                 -option
//                 -option=value),
// LIST_ARG        if the option takes a list argument
//                 (zero, one or more arguments; accepts the
//                 following syntax:
//                 -option
//                 -option=value
//                 -option=value value ...
//                 -option value
//                 -option value value ...),
// ALT_ARG         if the option takes an optional argument,
//                 (an OPT_ARG extension, that is,
//                 accepts the following syntax:
//                 -option
//                 -option=value
//                 -option value).

// If the field 'flag' is not NULL, it points to a variable that is set
// to the value given in the field 'val' when the option is found, but
// left unchanged if the option is not found.

// To have a long-named option do something other than set an 'int' to
// a compiled-in constant, such as set a value from 'optarg', set the
// option's 'flag' field to zero and its 'val' field to a nonzero
// value (the equivalent single-letter option character, if there is
// one).  For long options that have a zero 'flag' field, 'getopt'
// returns the contents of the 'val' field.

// If field 'valid' is not NULL, it points to a vector of string
// terminated by an element which is zero, this vector corresponds to
// valid values for this option.
// If field 'valid' is NULL and the field 'has_arg' is LIST_ARG,
// all value will be considered a valid value for this option until
// reaches a valid option or an option terminator.

enum Option_Argument
    {NO_ARG, REQD_ARG, OPT_ARG, LIST_ARG, ALT_ARG};

struct Option
{
    char const* name;
    Option_Argument has_arg;
    int* flag;
    int val;
    char const** valid;
    char const* descr;
};

// GWC added 'LMI_SO'.
class LMI_SO GetOpt
{
  private:
    // The next char to be scanned in the option-element
    // in which the last option character we returned was found.
    // This allows us to pick up the scan where we left off.

    // If this is zero, or a null string, it means resume the scan
    // by advancing to the next ARGV-element.

    static char* nextchar;

    // Describe how to deal with options that follow non-option ARGV-elements.

    // UNSPECIFIED means the caller did not specify anything;
    // the default is then REQUIRE_ORDER if the environment variable
    // _OPTIONS_FIRST is defined, PERMUTE otherwise.

    // REQUIRE_ORDER means don't recognize them as options.
    // Stop option processing when the first non-option is seen.
    // This is what Unix does.

    // PERMUTE is the default.  We permute the contents of 'argv' as we scan,
    // so that eventually all the options are at the end.  This allows options
    // to be given in any order, even with programs that were not written to
    // expect this.

    // RETURN_IN_ORDER is an option available to programs that were written
    // to expect options and other ARGV-elements in any order and that care about
    // the ordering of the two.  We describe each non-option ARGV-element
    // as if it were the argument of an option with character code zero.
    // Using '-' as the first character of the list of option characters
    // requests this mode of operation.

    // The special argument '--' forces an end of option-scanning regardless
    // of the value of 'ordering'.  In the case of RETURN_IN_ORDER, only
    // '--' can cause 'getopt' to return EOF with 'optind' != ARGC.

    enum OrderingEnum {REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER};
    OrderingEnum ordering;

    // Handle permutation of arguments.

    // Describe the part of ARGV that contains non-options that have
    // been skipped.  'first_nonopt' is the index in ARGV of the first of them;
    // 'last_nonopt' is the index after the last of them.

    static int first_nonopt;
    static int last_nonopt;

    // The last long option that has field 'has_arg' set to LIST_ARG.
    // In each iteration, the current argument is checked to see if it is
    // an valid value for the last list long option or it is an option.

    Option const* list_option;

    // Flag used to determine the very first LIST_ARG processing.

    int list_option_first;

    void exchange(char** argv);

    // Internal functions

    void Initialize(std::string const& a_optstring);
    int List_Value(int i);
    int List_No_Value();
    void print_expanding(char* v);

  public:
    // Set to an option character which was unrecognized.

    int optopt;

    // For communication from 'getopt' to the caller.
    // When 'getopt' finds an option that takes an argument,
    // the argument value is returned here.
    // Also, when 'ordering' is RETURN_IN_ORDER,
    // each non-option ARGV-element is returned here.

    char* optarg;

    // Index in ARGV of the next element to be scanned.
    // This is used for communication to and from the caller
    // and for communication between successive calls to 'getopt'.
    // On entry to 'getopt', zero means this is the first call; initialize.

    // When 'getopt' returns EOF, this is the index of the first of the
    // non-option elements that the caller should itself scan.

    // Otherwise, 'optind' communicates from one call to the next
    // how much of ARGV has been scanned so far.

    int optind;

    // Used only for long options.
    // If current option has field 'valid' not NULL and current argument
    // is a valid value, 'optindvalue' indexes the valid value in field 'valid'.
    // If there is no valid value in field 'valid' or field 'valid' is NULL,
    // 'optindvalue' is set to EOF.

    int optindvalue;

    // Callers store false here to inhibit the error message
    // for unrecognized options.

    bool opterr;

    int    nargc;
    char** nargv;
    std::string noptstring;
    Option const* nlongopts;
    int* nlongind;
    int nlong_only;

    // Scan elements of ARGV (whose length is ARGC) for option characters
    // given in OPTSTRING.

    // If an element of ARGV starts with '-', and is not exactly "-" or "--",
    // then it is an option element.  The characters of this element
    // (aside from the initial '-') are option characters.  If 'getopt'
    // is called repeatedly, it returns successively each of the option characters
    // from each of the option elements.

    // If 'getopt' finds another option character, it returns that character,
    // updating 'optind' and 'nextchar' so that the next call to 'getopt' can
    // resume the scan with the following option character or ARGV-element.

    // If there are no more option characters, 'getopt' returns 'EOF'.
    // Then 'optind' is the index in ARGV of the first ARGV-element
    // that is not an option.  (The ARGV-elements have been permuted
    // so that those that are not options now come last.)

    // OPTSTRING is a string containing the legitimate option characters.
    // A colon in OPTSTRING means that the previous character is an option
    // that wants an argument.  The argument is taken from the rest of the
    // current ARGV-element, or from the following ARGV-element,
    // and returned in 'optarg'.

    // If an option character is seen that is not listed in OPTSTRING,
    // return '?' after printing an error message.  If you set 'opterr' to
    // false, the error message is suppressed but we still return '?'.

    // If a char in OPTSTRING is followed by a colon, that means it wants an arg,
    // so the following text in the same ARGV-element, or the text of the following
    // ARGV-element, is returned in 'optarg'.  Two colons mean an option that
    // wants an optional arg; if there is text in the current ARGV-element,
    // it is returned in 'optarg'.

    // If OPTSTRING starts with '-' or '+', it requests different methods of
    // handling the non-option ARGV-elements.
    // See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

    // Long-named options begin with '--' instead of '-'.
    // Their names may be abbreviated as long as the abbreviation is unique
    // or is an exact match for some defined option.  If they have an
    // argument, it follows the option name in the same ARGV-element, separated
    // from the option name by a '=', or else the in next ARGV-element.
    // When 'getopt' finds a long-named option, it returns 0 if that option's
    // 'flag' field is nonzero, the value of the option's 'val' field
    // if the 'flag' field is zero.

    // The elements of ARGV aren't really const, because we permute them.
    // But we pretend they're const in the prototype to be compatible
    // with other systems.

    // LONGOPTS is a vector of 'struct option' terminated by an
    // element containing a name which is zero.

    // LONGIND returns the index in LONGOPT of the long-named option found.
    // It is only valid when a long-named option has been found by the most
    // recent call.

    // If LONG_ONLY is nonzero, '-' as well as '--' can introduce
    // long-named options. If an option that starts with '-' (not '--')
    // doesn't match a long option, but does match a short option,
    // it is parsed as a short option instead.
    // GWC note: i.e., use long only iff (bool)long_only is ***false***.

    GetOpt(int argc, char** argv, char const* optstring);
    GetOpt(int argc, char** argv, char const* optstring,
            Option const* longopts, int* longind, int long_only);
    GetOpt(GetOpt const&) = delete;
    GetOpt& operator=(GetOpt const&) = delete;
    ~GetOpt() = default; // Added by GWC.

    int operator()();

    // first_char returns the first character of the argument.

    int first_char();

    // next_arg looks at next argument for an integer, double or string
    // depending on the type of argument given to it. If the correct type is
    // found, the value is set and next_arg returns 1.  If the type is not
    // correct, next_arg returns 0.

    // double arguments start with a digit, plus, minus or period.
    // integer arguments start with a digit.
    // String arguments have no restriction.

    // If the next argument is an integer, set the reference variable to it
    // and increment the index to the options.  Return 1 if an integer is
    // found, else return 0.

    int next_arg(int& i);
    int next_arg(double& d);
    // GWC changed nonstandard to standard string class.
    int next_arg(std::string& s);

    // Prints a message for an invalid argument.

    void print_invalid();

    // Added by GWC.
    void usage(int status = EXIT_SUCCESS);
    void usage(std::ostream&);
};

// GWC removed an extraneous ';' that followed this function definition.
inline int GetOpt::first_char()
{
    return nargv[optind][0];
}

#endif // getopt_hpp
