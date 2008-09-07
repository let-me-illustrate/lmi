// -*- C++ -*-
//
// Copyright (C) 1998, 1999, 2000, 2002  Los Alamos National Laboratory,
// Copyright (C) 1998, 1999, 2000, 2002  CodeSourcery, LLC
//
// This file is part of FreePOOMA.
//
// FreePOOMA is free software; you can redistribute it and/or modify it
// under the terms of the Expat license.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Expat
// license for more details.
//
// You should have received a copy of the Expat license along with
// FreePOOMA; see the file LICENSE.
//
//-----------------------------------------------------------------------------
// PAssert non-template definitions.
//-----------------------------------------------------------------------------

// Include files

#include "Tools/PAssert.h"

//-----------------------------------------------------------------------------
// Use stdio.h instead of cstdio. Justification:
//
//   o Intel doesn't put these in the std:: namespace,
//     and writing varargs wrapper functions doesn't
//     sound like fun.
//   o Ultimately we should switch to ostringstreams,
//     but currently EGCS doesn't have these.
//   o Finally, this is a .cmpl.cpp files, so using stdio.h
//     doesn't pollute the global namespace of users of PAssert.
//
// Note that stdio.h (and other C header files) is required, but
// deprecated, by the '98 version of the standard.
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if !PETE_EXCEPTIONS
# include <stdlib.h>
#endif


namespace Pete {

//-----------------------------------------------------------------------------
// Definitions of methods in Assertion class.  The constructor stores
// the assertion message in a char buffer.
//-----------------------------------------------------------------------------

Assertion::Assertion(const char *msg, const char *file, int line)
{
  msg_m = new char[strlen(msg) + 1];
  strcpy(msg_m, msg);
  file_m = new char[strlen(file) + 1];
  strcpy(file_m, file);
  line_m = line;
}

Assertion::Assertion(const Assertion &a)
{
  msg_m = new char[strlen(a.what())+1];
  strcpy(msg_m, a.what());
  file_m = new char[strlen(a.file())+1];
  strcpy(file_m, a.file());
  line_m = a.line();
}

Assertion &Assertion::operator=(const Assertion &a)
{
  msg_m = new char[strlen(a.what())+1];
  strcpy(msg_m, a.what());
  file_m = new char[strlen(a.file())+1];
  strcpy(file_m, a.file());
  line_m = a.line();

  return *this;
}

//-----------------------------------------------------------------------------
// Function to perform the task of actually throwing an assertion, from a
// PAssert situation.
//-----------------------------------------------------------------------------

void toss_cookies(const char *msg, const char *file, int line ...)
{
  va_list ap;
  va_start(ap, line);
  char buf[256];
  vsprintf(buf, msg, ap);
  va_end(ap);

  Assertion a(buf, file, line);

#if PETE_EXCEPTIONS
  throw a;
#else
  fprintf(stderr, "### PETE Assertion Failure ###\n");
  fprintf(stderr, "### %s\n", a.what());
  fprintf(stderr, "### File %s; Line %d.\n", a.file(), a.line());
  abort();
#endif
}

} // namespace Pete

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PAssert.cpp,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
