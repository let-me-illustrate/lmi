// This may look like C code, but it is really -*- C++ -*-

// Getopt for GNU.
// Copyright (C) 1987, 1989 Free Software Foundation, Inc.

// (Modified by Douglas C. Schmidt for use with GNU G++.)
// (Modified by Vinicius J. Latorre for use with long options.)
// (Modified by Gregory W. Chicares in years stated below:
//   substantive changes marked inline with 'GWC'; trivial changes
//   (formatting, style, standardization) not so marked, to reduce
//   clutter.)
//
// Get the original Latorre sources here:
// http://groups.google.com/groups?selm=9502241613.AA06287%40moon.cpqd.br
// http://www.geocrawler.com/archives/3/364/1995/2/0/2053420/
// [Both URLs should work, but the second seemed to lack the full
// contents when I revisited it later.]
//
// GWC modifications Copyright (C) 2001, 2002, 2003, 2004, 2005
//   Gregory W. Chicares and released with the same licensing terms as
//   the original, viz.:

// This file is part of the GNU C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms of
// the GNU Library General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.  This library is distributed in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// GWC added this RCS Id:
// $Id: getopt.cpp,v 1.3 2005-03-11 13:40:41 chicares Exp $

// GWC conditionalized gcc-specific pragma to avoid a warning when
// compiling with como C++ using a gcc backend.
#ifdef __GNUG__
#   if defined __GNUC__ && !defined __COMO__
#       pragma implementation
#   endif // defined __GNUC__ && !defined __COMO__
#endif //  __GNUG__

// GWC removed a dozen or two lines that served to make alloca() available
// for various compilers.
//
// The proximate reason is that the latest (as this is written on 2002-02-07)
// version of cygwin is incompatible with the macro originally given here for
// 'alloca'. Copying the cygwin macro here wouldn't work: it's incompatible
// with mingw. Those two compilers have no header in common that implements
// alloca(). There's only one call to alloca() here, in GetOpt::exchange(),
// and we don't use alloca() anywhere else, so I replaced it with std::malloc(),
// taking care to std::free() the storage allocated. It would arguably be better
// to use C++'s facilities, but that could also be said of std::fprintf() e.g.
// and I have no wish to rewrite this whole thing at this time.
//
// alloca() is nonstandard. It is said to have some advantages:
//
// http://www.gnu.org/manual/glibc-2.2.3/html_node/libc_55.html
//   fast, efficient
//   safe with std::longjmp(): "This is the most important reason to use alloca."
//   [but this is C++, so std::longjmp safety isn't a big issue--GWC]
// http://groups.google.com/groups?selm=dilae3w6r6a.fsf%40isolde.research.att.com
//   stroustrup doubts claimed speed, considers alloca() a 'hack'
//
// It also has disadvantages:
//
// http://www.gnu.org/manual/glibc-2.2.3/html_node/libc_56.html
//   doesn't give clean error message on failure; not portable
// http://www.eskimo.com/~scs/C-faq/faq.html
//   Steve Summit's C FAQ: see question 7.32
// http://sources.redhat.com/ml/gdb/2000-11/msg00109.html
//   man page quotes
//   BSD: "its use is discouraged"
//   Solaris: "Its use is strongly discouraged"
// borland C++ 5.02 online help:
//   use in try-block causes stack corruption
//   "If the calling function does not contain any references to local
//    variables in the stack, the stack will not be restored correctly
//    when the function exits, resulting in a program crash." This is
//   followed by an example of the 'safe' way to use alloca(), which
//   involves adding three lines of code to create a dummy variable.
//
// A web search will reveal more problems; I judge that there's enough
// evidence here to support banning alloca() from this project.

// GWC added this to conform to LMI standards.
#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// GWC changed filename to conform to LMI standard: lowercase, .hpp .
// #include <GetOpt.h>
#include "getopt.hpp"

// GWC added this.
#include <algorithm>     // std::max()
// GWC added this for prototype of std::isalpha().
#include <cctype>
// GWC added this.
#include <cstdio>
// GWC added this.
#include <cstdlib> // std::malloc
// GWC added this.
#include <cstring>
// GWC added this.
#include <iostream>
// GWC added this.
#include <ostream>
// GWC added this.
#include <stdexcept>
// GWC added this.
#include <vector>

char* GetOpt::nextchar = 0;
int GetOpt::first_nonopt = 0;
int GetOpt::last_nonopt = 0;

GetOpt::GetOpt (int argc, char** argv, char const* optstring)
 :list_option (0)
 ,list_option_first (0)
 ,optindvalue (EOF)
 ,opterr (true)
 ,nargc (argc)
 ,nargv (argv)
 ,noptstring (optstring)
 ,nlongopts (0)
 ,nlongind (0)
 ,nlong_only (0)
{
    Initialize (noptstring);
}


GetOpt::GetOpt (int argc, char** argv, char const* optstring,
                Option const* longopts, int* longind, int long_only)
 :list_option (0)
 ,list_option_first (0)
 ,optindvalue (EOF)
 ,opterr (true)
 ,nargc (argc)
 ,nargv (argv)
 ,noptstring (optstring)
 ,nlongopts (longopts)
 ,nlongind (longind)
 ,nlong_only (long_only)
{
    // Automatically register any short-option alii for long options.
    for(Option const* i = nlongopts; 0 != i->name; ++i)
        {
        if(!i->val)
            {
            return;
            }
        // THIRD_PARTY !! It appears that 'val' really should be a char.
        noptstring += static_cast<char>(i->val);
        if(NO_ARG != i->has_arg)
            {
            noptstring += ":";
            }
        if(OPT_ARG == i->has_arg)
            {
            noptstring += ":";
            }
        }
    Initialize (noptstring);
}


void
GetOpt::Initialize  (std::string const& a_optstring)
{
  // Initialize the internal data when the first call is made.
  // Start processing options with ARGV-element 1 (since ARGV-element 0
  // is the program name); the sequence of previously skipped
  // non-option ARGV-elements is empty.

  first_nonopt = last_nonopt = optind = 1;
  optarg = nextchar = 0;

  // Determine how to handle the ordering of options and nonoptions.

  if (0 == a_optstring.c_str())
    {
      noptstring = "";
      if (std::getenv ("_POSIX_OPTION_ORDER"))
        ordering = REQUIRE_ORDER;
      else
        ordering = PERMUTE;
    }
  else if ('-' == a_optstring[0])
    ordering = RETURN_IN_ORDER;
  else if (('+' == a_optstring[0]) || (std::getenv ("_POSIX_OPTION_ORDER")))
    ordering = REQUIRE_ORDER;
  else
    ordering = PERMUTE;
}


void
GetOpt::exchange (char** argv)
{
  int nonopts_size = (last_nonopt - first_nonopt) * sizeof (char*);
// GWC substituted std::malloc() for alloca() and added call to std::free() below.
//  char** temp = static_cast<char**>(alloca (nonopts_size));
  char** temp = static_cast<char**>(std::malloc (nonopts_size));
  if(0 == temp)
    {
    throw std::runtime_error("Out of memory.");
    }

  // Interchange the two blocks of data in argv.

  std::memcpy (temp, &argv[first_nonopt], nonopts_size);
  std::memcpy (&argv[first_nonopt], &argv[last_nonopt],
         (optind - last_nonopt) * sizeof (char*));
  std::memcpy (&argv[first_nonopt + optind - last_nonopt], temp,
         nonopts_size);

  std::free(temp);

  // Update records for the slots the non-options now occupy.

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}


int
GetOpt::List_Value  (int i)
{
  Option const* x = list_option;

  list_option_first = 0;
  optarg = nargv[i];
  nextchar = 0;
  if (list_option->has_arg != LIST_ARG)
    list_option = 0;
  if (x->flag)
    {
      *(x->flag) = x->val;
      return  0;
    }
  return  x->val;
}


int
GetOpt::List_No_Value  ()
{
  Option const* x = list_option;

  list_option_first = 0;
  list_option = 0;
  optindvalue = EOF;
  optarg = 0;
  nextchar = 0;
  if (x->flag)
    {
      *(x->flag) = x->val;
      return  0;
    }
  return  x->val;
}


void
GetOpt::print_expanding  (char* v)
{
  int x;

  for (;  (x = *v) != 0;  v++)
    if (x < 040)
      std::fprintf (stderr, "^%c", x + '@');
    else if (x > 0177)
      std::fprintf (stderr, "\\%o", x);
    else
      std::fprintf (stderr, "%c", x);
}


void
GetOpt::print_invalid  ()
{
  std::fprintf (stderr, "%s: invalid argument '", nargv[0]);
  print_expanding (optarg);
  std::fprintf (stderr, "'\n");
}


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

int
GetOpt::operator () (void)
{
  optopt = 0;

  // We are processing a LIST_ARG or ALT_ARG,
  // now we try to see if the next argument is a current option value
  // or is another option.

TRY_TO_GET_A_VALUE:
  if (list_option)
    {
      // If we have done all the ARGV-elements, stop the scan.

      if (optind == nargc)
          // Check if first LIST_ARG with no argument.
          if (list_option_first)
            return  List_No_Value ();
          else
            return EOF;

      if (list_option->valid == 0)
        {
          // If there isn't a valid list of values,
          // try to see if current argument isn't an option.

          if (nargv[optind][0] != '-')
            return  List_Value (optind++);

          // An argument starting with '-' may be an option or not,
          // this check is made below.
        }
      else
        {
          // If there is a valid list of values,
          // try to see if current argument is a valid value.

          char const** v;

          for (v = list_option->valid, optindvalue = 0;  *v != 0; v++, optindvalue++)
            if (std::strcmp (*v, nargv[optind]) == 0)
              return  List_Value (optind++);
          optindvalue = EOF;

          // Check if first LIST_ARG with no argument;
          // Otherwise, here we know that the LIST_ARG processing terminates.

          if (list_option_first)
            return  List_No_Value ();
          list_option = 0;
        }
    }

  if (nextchar == 0 || *nextchar == 0)
    {
      if (ordering == PERMUTE)
        {
          // If we have just processed some options following some non-options,
          // exchange them so that the options come first.

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange (nargv);
          else if (last_nonopt != optind)
            first_nonopt = optind;

          // Now skip any additional non-options
          // and extend the range of non-options previously skipped.

          while (optind < nargc
                 && (nargv[optind][0] != '-' || nargv[optind][1] == 0))
            optind++;
          last_nonopt = optind;
        }

      // Special ARGV-element '--' means premature end of options.
      // Skip it like a null option,
      // then exchange with previous non-options as if it were an option,
      // then skip everything else like a non-option.

      if (optind != nargc && !std::strcmp (nargv[optind], "--"))
        {
          optind++;

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange (nargv);
          else if (first_nonopt == last_nonopt)
            first_nonopt = optind;
          last_nonopt = nargc;

          optind = nargc;
        }

      // If we have done all the ARGV-elements, stop the scan
      // and back over any non-options that we skipped and permuted.

      if (optind == nargc)
        {
          // Check if first LIST_ARG with no argument;
          // Otherwise, terminates LIST_ARG processing.

          if (list_option_first)
            return  List_No_Value ();
          list_option = 0;

          // Set the next-arg-index to point at the non-options
          // that we previously skipped, so the caller will digest them.

          if (first_nonopt != last_nonopt)
            optind = first_nonopt;
          return EOF;
        }

      // If we have come to a non-option and did not permute it,
      // either stop the scan or describe it to the caller and pass it by.

      if (nargv[optind][0] != '-' || nargv[optind][1] == 0)
        {
          // Check if first LIST_ARG with no argument;
          // Otherwise, terminates LIST_ARG processing.

          if (list_option_first)
            return  List_No_Value ();
          list_option = 0;

          if (ordering == REQUIRE_ORDER)
            return EOF;
          optarg = nargv[optind++];
          optopt = EOF;
          return 0;
        }

      // We have found another option-ARGV-element.
      // Start decoding its characters.

      nextchar = nargv[optind] + 1;
    }

  // Decode the current option-ARGV-element.

  // Check whether the ARGV-element is a long option.

  // If long_only and the ARGV-element has the form "-f", where f is
  // a valid short option, don't consider it an abbreviated form of
  // a long option that starts with f.  Otherwise there would be no
  // way to give the -f short option.

  // On the other hand, if there's a long option "fubar" and
  // the ARGV-element is "-fu", do consider that an abbreviation of
  // the long option, just like "--fu", and not "-f" with arg "u".

  // This distinction seems to be the most useful approach.
  if
    (   nlongopts
    &&  (
            ('-' == nargv[optind][1]) ? (nextchar++, 1) : (0)
        ||  (
                nlong_only
            &&  (
                    nargv[optind][2]
                ||  !std::strchr (noptstring.c_str(), nargv[optind][1])
                )
            )
        )
    )
    {
      char* nameend;
      Option const* p;
      Option const* pfound = 0;
      int exact = 0;
      int ambig = 0;
      int indfound = 0;
      int option_index;

      for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
        // Do nothing.
        ;

      // Test all long options for either exact match or abbreviated matches.
      for (p = nlongopts, option_index = 0; p->name; p++, option_index++)
        if (!std::strncmp (p->name, nextchar, nameend - nextchar))
          {
            if (static_cast<int>(std::strlen (p->name)) == nameend - nextchar)
              {
                // Exact match found.
                pfound = p;
                indfound = option_index;
                exact = 1;
                break;
              }
            else if (pfound == 0)
              {
                // First nonexact match found.
                pfound = p;
                indfound = option_index;
              }
            else
              // Second or later nonexact match found.
              ambig = 1;
          }

      if (ambig && !exact)
        {
          if (list_option)
            // It is not a long option, but it is a value for LIST_ARG
            return  List_Value (optind++);
          if (opterr)
            std::fprintf (stderr, "%s: option '%s' is ambiguous\n",
                     nargv[0], nargv[optind]);
          nextchar += std::strlen (nextchar);
          optind++;
          return '?';
        }

      if (pfound)
        {
          // Check if first LIST_ARG with no argument;
          // Otherwise, terminates LIST_ARG processing.

          if (list_option_first)
            return  List_No_Value ();
          list_option = 0;

          option_index = indfound;
          optind++;
          if ((*nameend) != 0)
            // (-option=value)
            // Don't test has_arg with >, because some C compilers don't
            // allow it to be used on enums.
            switch (pfound->has_arg)
             {

              case NO_ARG:
                if (opterr)
                  {
                    if (nargv[optind - 1][1] == '-')
                      // --option
                      std::fprintf (stderr,
                               "%s: option '--%s' doesn't allow an argument\n",
                               nargv[0], pfound->name);
                    else
                      // +option or -option
                      std::fprintf (stderr,
                           "%s: option '%c%s' doesn't allow an argument\n",
                           nargv[0], nargv[optind - 1][0], pfound->name);
                  }
                nextchar += std::strlen (nextchar);
                return '?';

              case LIST_ARG:
                list_option = pfound;

              default:
                optarg = nameend + 1;
                break;

             }
          else
            // (-option value) or (-option)
            switch (pfound->has_arg)
             {

              // See methods List_Value and List_No_Value
              case ALT_ARG:
              case LIST_ARG:
                nextchar += std::strlen (nextchar);
                if (nlongind)
                  *nlongind = option_index;
                list_option_first = 1;
                list_option = pfound;
                goto  TRY_TO_GET_A_VALUE;

              case REQD_ARG:
                if (optind < nargc)
                  optarg = nargv[optind++];
                else
                  {
                    if (opterr)
                      std::fprintf (stderr, "%s: option '%s' requires an argument\n",
                               nargv[0], nargv[optind - 1]);
                    nextchar += std::strlen (nextchar);
                    return  (':' == noptstring[0]) ? (':') : ('?');
                  }
                break;

              default:
                optarg = 0;
                break;

             }

          // Check if 'optarg' is a valid value.

          {
            int  result;

            nextchar += std::strlen (nextchar);
            if (nlongind)
              *nlongind = option_index;
            if (pfound->flag)
              {
                *(pfound->flag) = pfound->val;
                result = 0;
              }
            else
              result = pfound->val;

            // If there is a valid list of values,
            // check if 'optarg' is a valid value.

            if ((pfound->valid) && (optarg))
              {
                char const** v;

                for (v = pfound->valid, optindvalue = 0;  *v != 0; v++, optindvalue++)
                  if (std::strcmp (*v, optarg) == 0)
                    return  result;
                optindvalue = EOF;

                // Here we know it is an invalid value.

                if (opterr)
                  {
                    std::fprintf (stderr, "%s: value '", nargv[0]);
                    print_expanding (optarg);
                    if (nargv[optind - 2][1] == '-')
                      // --option
                      std::fprintf (stderr, "' is invalid for option '--%s'\n",
                               pfound->name);
                    else
                      // +option or -option
                      std::fprintf (stderr, "' is invalid for option '%c%s'\n",
                               nargv[optind - 2][0], pfound->name);
                  }
                return  '?';
              }

            // There is a NULL valid list of values.

            return  result;
          }
        }

      // Can't find it as a long option.  If this is not long_only,
      // or the option starts with '--' or is not a valid short
      // option, then it's an error.
      // Otherwise interpret it as a short option.

      if (!nlong_only || nargv[optind][1] == '-' || std::strchr (noptstring.c_str(), *nextchar) == 0)
        {
          if (list_option)
            // It is not a long option, but it is a value for LIST_ARG
            return  List_Value (optind++);
          if (opterr)
            {
              // GWC suppressed the declarations of these two unused variables.
//              char  * v;
//              int  x;

              if (nargv[optind][1] == '-')
                // --option
                std::fprintf (stderr, "%s: unrecognized option '--",
                         nargv[0]);
              else
                // +option or -option
                std::fprintf (stderr, "%s: unrecognized option '%c",
                         nargv[0], nargv[optind][0]);
              print_expanding (nextchar);
              std::fprintf (stderr, "'\n");
            }
          // THIRD_PARTY !! Does the rhs mean '0'?
          nextchar = const_cast<char*>("");
          optind++;
          return '?';
        }
    }

  // Look at and handle the next option-character.

  {
    int c = *nextchar++;
    char* temp = const_cast<char*>(std::strchr (noptstring.c_str(), c));

    // Increment 'optind' when we start to process its last character.
    if (*nextchar == 0)
      optind++;

    if (temp == 0 || c == ':')
      {
        if (opterr)
          {
            if (c < 040 || c >= 0177)
              {
                // Check if first LIST_ARG with no argument;
                // Otherwise, terminates LIST_ARG processing.

                if (list_option_first)
                  return  List_No_Value ();
                list_option = 0;

                std::fprintf (stderr, "%s: unrecognized option, character code 0%o\n",
                         nargv[0], c);
              }
            else
              {
                if (list_option)
                  // It is not a short option, but it is a value for LIST_ARG
                  return  List_Value ((*nextchar == 0) ? (optind - 1) : (optind++));
                std::fprintf (stderr, "%s: unrecognized option '-%c'\n",
                         nargv[0], c);
              }
          }
        optopt = c;
        return '?';
      }

    // Check if first LIST_ARG with no argument;
    // Otherwise, terminates LIST_ARG processing.

    if (list_option_first)
      return  List_No_Value ();
    list_option = 0;

    if (temp[1] == ':')
      {
        if (temp[2] == ':')
          {
            // This is an option that accepts an argument optionally.
            if (*nextchar != 0)
              {
                optarg = nextchar;
                optind++;
              }
            else
              optarg = 0;
            nextchar = 0;
          }
        else
          {
            // This is an option that requires an argument.
            if (*nextchar != 0)
              {
                optarg = nextchar;
                // If we end this ARGV-element by taking the rest as an arg,
                // we must advance to the next element now.
                optind++;
              }
            else if (optind == nargc)
              {
                if (opterr)
                  std::fprintf (stderr, "%s: option '-%c' requires an argument\n",
                           nargv[0], c);
                optopt = c;
                c = (':' == noptstring[0]) ? (':') : ('?');
              }
            else
              // We already incremented 'optind' once;
              // increment it again when taking next ARGV-elt as argument.
              optarg = nargv[optind++];
            nextchar = 0;
          }
      }
    return c;
  }
}


int
GetOpt::next_arg (int &i)
{
  int tmp;

  // Terminates LIST_ARG processing.

  list_option = 0;

  if (0 < std::sscanf (nargv[optind], "%d", &tmp))
    {
      i = tmp;
      optind++;
      return 1;
    }
  else
    return 0;
}


int
GetOpt::next_arg (double &d)
{
  double tmp;

  // Terminates LIST_ARG processing.

  list_option = 0;

  if (0 < std::sscanf (nargv[optind], "%lf", &tmp))
    {
      d = tmp;
      optind++;
      return 1;
    }
  else
    return 0;
}


int
// GWC changed nonstandard to standard string class.
//GetOpt::next_arg (String &s)
GetOpt::next_arg (std::string &s)
{
  // Terminates LIST_ARG processing.

  list_option = 0;

  if ('-' != nargv[optind][0])
    {
      s = nargv[optind];
      optind++;
      return 1;
    }
  else
    return 0;
}

// Added by GWC.
void
GetOpt::usage(int status)
{
    std::ostream& os(EXIT_SUCCESS == status ? std::cout : std::cerr);
    os
        << "Usage: "
        << nargv[0]
        << '\n';
    std::string::size_type max_name_length = 0;
    typedef std::vector<std::string> str_vec;
    typedef str_vec::iterator str_vec_i;
    str_vec option_names;
    str_vec option_descriptions;
    for(Option const* i = nlongopts; 0 != i->name; ++i)
        {
        std::string s("  ");
        // GWC added test to ensure std::isalpha()'s behavior is defined.
        // INELEGANT !! This useful idiom should be abstracted out.
        if
            (   0 == i->flag
            &&  (EOF == i->val) || (0 <= i->val && i->val <= UCHAR_MAX)
            &&  std::isalpha(i->val)
            )
            {
            s += "-";
            s += static_cast<char>(i->val);
            s += ", ";
            }
        else
            {
            s += "    ";
            }
        s += "--";
        s += i->name;
        s += "   ";
        max_name_length = std::max(max_name_length, s.size());
        option_names.push_back(s);
        option_descriptions.push_back(i->descr);
        }
    str_vec_i n;
    str_vec_i d;
    for
        (n  = option_names.begin() ,d  = option_descriptions.begin()
        ;n != option_names.end()   ,d != option_descriptions.end()
        ;++n, ++d
        )
        {
        n->resize(max_name_length, ' ');
        os << *n << *d << '\n';
        }
    std::exit(status);
}




#ifdef TEST_GetOpt

#include <cstdio>

int
main (int argc, char** argv)
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
      {"add",     REQD_ARG,        0, 0, 0},
      {"append",  NO_ARG,          0, 0, 0},
      {"delete",  REQD_ARG,        0, 0, 0},
      {"verbose", NO_ARG,          0, 0, 0},
      {"create",  NO_ARG,          0, 0, 0},
      {"file",    REQD_ARG,        0, 0, 0},
      {"list",    LIST_ARG,        0, 0, 0},
      {"opt",     OPT_ARG,         0, 0, 0},
      {"alt",     ALT_ARG,         0, 0, 0},
      {"vfile",   REQD_ARG,        0, 0, vfile},
      {"vlist",   LIST_ARG,        0, 0, vlist},
      {"vopt",    OPT_ARG,         0, 0, vopt},
      {"valt",    ALT_ARG,         0, 0, vopt},
      {0,         NO_ARG,          0, 0, 0}
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

#endif // TEST_GetOpt

