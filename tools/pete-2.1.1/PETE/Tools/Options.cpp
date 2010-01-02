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
// Functions:
// flagOption
// stringOption
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------

#include <string>
#include <stdlib.h>
using std::string;

#include "Options.h"

bool flagOption(int argc,char ** argv,const string &option)
{
  // Scan through the arguments.

  int i;
  for (i=1;i<argc;++i)
  {
    if (string(argv[i]) == string(option))
      return true;
  }

  return false;
}

string stringOption(int argc,char ** argv,
		    const string &option,const string &def)
{
  // Scan through the arguments.

  int i;
  for (i=1;i<argc;++i)
  {
    if (string(argv[i]) == string(option))
    {
      if (i+1<argc)
	return string(argv[i+1]);
    }
  }

  return def;
}

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Options.cpp,v $   $Author: chicares $
// $Revision: 1.1 $   $Date: 2008-09-04 13:55:21 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
