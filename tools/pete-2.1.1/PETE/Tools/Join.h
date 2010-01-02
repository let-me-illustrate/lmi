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

#ifndef PETE_SRC_TOOLS_JOIN_H
#define PETE_SRC_TOOLS_JOIN_H

#include <string>

using std::string;

//-----------------------------------------------------------------------------
// joinWithComma(string,string,...)
//
// Joins a bunch of strings together with commas between them.  If any of the
// strings are empty then the redundant commas are eliminated.  This routine
// is used to produce the list of template arguments given lists for each
// function argument (some of which may be empty).
//-----------------------------------------------------------------------------

extern string joinWithComma(const string& a,const string& b);

extern string joinWithComma(const string& a,const string& b,
			  const string& c);

#endif // PETE_SRC_TOOLS_JOIN_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Join.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo


