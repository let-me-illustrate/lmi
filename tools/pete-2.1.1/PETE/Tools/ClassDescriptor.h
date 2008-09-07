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

#ifndef PETE_SRC_TOOLS_CLASSDESCRIPTOR_H
#define PETE_SRC_TOOLS_CLASSDESCRIPTOR_H

#include <iostream>
#include <stdio.h>
#include <string>

using std::string;
using std::ostream;
using std::endl;

#include "Tools/DescriptorBase.h"

class ClassDescriptor: public DescriptorBase<2> {
public:

  //---------------------------------------------------------------------------
  // Constructors.

  ClassDescriptor() { }

  ClassDescriptor(const string &ad, const string &ic)
  {
    addData(0, ad);
    addData(1, ic);
  }

  ClassDescriptor(const ClassDescriptor &model)
  : DescriptorBase<2>(model)
  { }

  //---------------------------------------------------------------------------
  // Trivial destructor.

  ~ClassDescriptor() { }

  //---------------------------------------------------------------------------
  // Copy-assignment operator: just copy members.

  ClassDescriptor &operator=(const ClassDescriptor &rhs)
  {
    DescriptorBase<2>::operator=(rhs);

    return *this;
  }

  //---------------------------------------------------------------------------
  // Return strings with numbers/args substituted.

  string argDef(int i) const
  {
    return substituteNum(i, str(0));
  }

  string inputClass(int i) const
  {
    return substituteNum(i, str(1));
  }

private:

  string substituteNum(int i, const string &s) const
  {
    char n[2];
    sprintf(n, "%d", i);
    string str(s), rep("[n]"), num(n);
    int pos;

    while ((pos = str.find(rep, 0)) < str.size())
      str.replace(pos, 3, num);

    return str;
  }

  // Currently substituteArg is unused.  Pooma used to convert
  // arguments without CreateLeaf, so this function was useful
  // for conversions like l -> Scalar<T1>(l)

  string substituteArg(const string &arg,
    const string &s) const
  {
    string str(s), rep("[arg]");
    int pos;

    while ((pos = str.find(rep, 0)) < str.size())
      str.replace(pos, 5, arg);

    return str;
  }
};

inline ostream &operator<<(ostream &os, const ClassDescriptor &o)
{
  os << "ARG   = " << o.argDef(1) << endl;
  os << "CLASS = " << o.inputClass(1) << endl;

  return os;
}

#endif // PETE_SRC_TOOLS_CLASSDESCRIPTOR_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: ClassDescriptor.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
