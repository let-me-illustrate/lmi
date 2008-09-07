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

#ifndef PETE_SRC_TOOLS_OPERATORDESCRIPTOR_H
#define PETE_SRC_TOOLS_OPERATORDESCRIPTOR_H

#include <iostream>

using std::ostream;
using std::endl;

#include <string>

using std::string;

#include "Tools/DescriptorBase.h"

class OperatorDescriptor: public DescriptorBase<4> {
public:

  //---------------------------------------------------------------------------
  // Constructors.

  OperatorDescriptor()
  { }

  OperatorDescriptor(const string &tag, const string &func,
		     const string &expr, const string &arg = "")
  {
    addData(0, tag);
    addData(1, func);
    addData(2, expr);
    addData(3, arg);
  }

  OperatorDescriptor(const OperatorDescriptor &model)
  : DescriptorBase<4>(model)
  { }

  //---------------------------------------------------------------------------
  // Trivial destructor.

  ~OperatorDescriptor() { }

  //---------------------------------------------------------------------------
  // Copy-assignment operator: just copy members.

  OperatorDescriptor &operator=(const OperatorDescriptor &rhs)
  {
    DescriptorBase<4>::operator=(rhs);

    return *this;
  }

  //---------------------------------------------------------------------------
  // Return strings/info.

  const string tag(bool full = true) const
  {
    if (full)
      return str(0);
    else
      return str(0).substr(0, str(0).find('<'));
  }

  const string &function() const
  {
    return str(1);
  }

  const string &expression() const
  {
    return str(2);
  }

  const string &argDef() const
  {
    return str(3);
  }

  bool templateArgs() const
  {
    return argDef().size() != 0;
  }

};

inline ostream &operator<<(ostream &os, const OperatorDescriptor &o)
{
  os << "TAG  = " << o.tag() << endl;
  os << "FUNC = " << o.function() << endl;
  os << "EXPR = " << o.expression() << endl;
  os << "ARG  = " << o.argDef() << endl;

  return os;
}

#endif // PETE_SRC_TOOLS_OPERATORDESCRIPTOR_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: OperatorDescriptor.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
