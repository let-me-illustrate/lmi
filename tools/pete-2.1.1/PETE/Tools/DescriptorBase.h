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

#ifndef PETE_SRC_TOOLS_DESCRIPTORBASE_H
#define PETE_SRC_TOOLS_DESCRIPTORBASE_H

#include <algorithm>
#include <string>

using std::string;
using std::copy;

#include "Tools/PAssert.h"

template<int N>
class DescriptorBase {
public:

  //---------------------------------------------------------------------------
  // Constructors: default and copy constructor.

  DescriptorBase()
  { }

  DescriptorBase(const DescriptorBase<N> &model)
  {
    copy(model.strs_m, model.strs_m + N, strs_m);
  }

  //---------------------------------------------------------------------------
  // Trivial destructor.

  ~DescriptorBase() { }

  //---------------------------------------------------------------------------
  // Adds some data to this descriptor.

  void addData(int snum, const string &data)
  {
    PAssert(snum >= 0 && snum < N);

    strs_m[snum] = data;
  }

  //---------------------------------------------------------------------------
  // Copy-assignment operator: just copy members.

  DescriptorBase &operator=(const DescriptorBase &rhs)
  {
    copy(rhs.strs_m, rhs.strs_m + N, strs_m);

    return *this;
  }

  //---------------------------------------------------------------------------
  // Return strings.

  const string &str(int n) const
  {
    PAssert(n >= 0 && n < N);

    return strs_m[n];
  }

private:

  string strs_m[N];

};

#endif // PETE_SRC_TOOLS_DESCRIPTORBASE_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: DescriptorBase.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
