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

#include "Tools/Join.h"

//-----------------------------------------------------------------------------
// joinWithComma(string,string,...)
//
// Joins a bunch of strings together with commas between them.  If any of the
// strings are empty then the redundant commas are eliminated.  This routine
// is used to produce the list of template arguments given lists for each
// function argument (some of which may be empty).
//-----------------------------------------------------------------------------

string joinWithComma(const string& a,const string& b)
{
  string str;
  if ((a.size() > 0) && (b.size() > 0))
  {
    str = a + "," + b;
  }
  else
  {
    str = a + b;
  }
  return str;
}

string joinWithComma(const string& a,const string& b,
			  const string& c)
{
  return joinWithComma(joinWithComma(a,b),c);
}

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Join.cpp,v $   $Author: chicares $
// $Revision: 1.1 $   $Date: 2008-09-04 13:55:21 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
