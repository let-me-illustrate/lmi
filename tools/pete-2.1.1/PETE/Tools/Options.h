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

#ifndef PETE_TOOLS_OPTIONS_H
#define PETE_TOOLS_OPTIONS_H

//-----------------------------------------------------------------------------
// Functions:
// flagOption
// stringOption
//-----------------------------------------------------------------------------

#include <string>

using std::string;

bool flagOption(int,char **,const string &);

string stringOption(int,char **,const string &,const string &);

#endif // PETE_TOOLS_OPTIONS_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Options.h,v $   $Author: chicares $
// $Revision: 1.1 $   $Date: 2008-09-04 13:55:21 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
