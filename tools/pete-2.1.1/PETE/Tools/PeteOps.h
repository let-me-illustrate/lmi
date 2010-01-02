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

#ifndef PETE_TOOLS_PETEOPS_H
#define PETE_TOOLS_PETEOPS_H

#include "Tools/OperatorDescriptor.h"
#include <vector>
#include <map>
#include <string>
using std::map;
using std::vector;
using std::string;

void peteOps(map<string, vector<OperatorDescriptor> > &m);

#endif // PETE_TOOLS_PETEOPS_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PeteOps.h,v $   $Author: chicares $
// $Revision: 1.1 $   $Date: 2008-09-04 13:55:31 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
