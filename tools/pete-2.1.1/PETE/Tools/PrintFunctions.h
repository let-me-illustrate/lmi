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
// Class:
// UnaryFunction
// UnaryCastFunction
// UnaryStdFunction
// BinaryFunction
// BinaryStdFunction
// TrinaryFunction
// AssignFunction
//-----------------------------------------------------------------------------

#ifndef PETE_TOOLS_PRINTFUNCTIONS_H
#define PETE_TOOLS_PRINTFUNCTIONS_H

//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Overview:
//
// The classes defined here are print functors that will print definitions
// of C++ functions for given operators and classes.  They have static member
// functions called print() that take an output stream and a number of
// operator and class definitions.  Values from the operator and class
// definition objects are used to fill in the definition that is printed.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Typedefs:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------------

#include <iostream>

using std::endl;

#include <string>

using std::string;

#include "Tools/ClassDescriptor.h"
#include "Tools/OperatorDescriptor.h"
#include "Tools/Join.h"

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Full Description:
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// UnaryFunction
//
// Prints the definition of a unary function that creates a PETE object for a
// given operator from a given class.  First it checks if the class has any
// template arguments.  If the class has template args then the function must
// be templated on those arguments and we must use the keyword "typename"
// to define some types.
//-----------------------------------------------------------------------------

class UnaryFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1) const
  {
    string args = joinWithComma(opdef.argDef(), class1.argDef(1));
    bool temp = (args.size() > 0);
    string typenameString = temp ? "typename " : "";

    ostr << endl;
    if (temp)
    {
      ostr << "template<" << args << ">" << endl;
    }
    ostr << "inline " << typenameString << "MakeReturn<UnaryNode<"
	 << opdef.tag() << "," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1) << " >::Leaf_t> >::Expression_t" << endl
	 << "" << opdef.function() << "(const " << class1.inputClass(1)
	 << " & l)"
	 << endl
	 << "{" << endl
	 << "  typedef UnaryNode<" << opdef.tag() << "," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1) << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<"
	 << class1.inputClass(1) << " >::make(l)));" << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class UnaryCastFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1) const
  {
    ostr << endl
	 << "template<" << joinWithComma("class T1",class1.argDef(2))
	 << ">" << endl
	 << "inline typename MakeReturn<UnaryNode<" << opdef.tag() << "<T1>,"
	 << endl
	 << "  typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t> >::Expression_t" << endl
	 << "" << opdef.function() << "(const T1&, const "
	 << class1.inputClass(2) << " & l)" << endl
	 << "{" << endl
	 << "  typedef UnaryNode<" << opdef.tag() << "<T1>," << endl
	 << "    typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<"
	 << class1.inputClass(2) << " >::make(l)));" << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
// This happens to be the same as UnaryCastFunction, for now at least.
//-----------------------------------------------------------------------------

class UnaryStdFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1) const
  {
    ostr << endl
	 << "template<" << joinWithComma("class T1",class1.argDef(2))
	 << ">" << endl
	 << "inline typename MakeReturn<UnaryNode<" << opdef.tag() << "<T1>,"
	 << endl
	 << "  typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t> >::Expression_t" << endl
	 << "" << opdef.function() << "(const T1&, const "
	 << class1.inputClass(2) << " & l)" << endl
	 << "{" << endl
	 << "  typedef UnaryNode<" << opdef.tag() << "<T1>," << endl
	 << "    typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<"
	 << class1.inputClass(2) << " >::make(l)));" << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class BinaryFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1,
	     const ClassDescriptor& class2) const
  {
    string args = joinWithComma(class1.argDef(1),class2.argDef(2));
    bool temp = (args.size() > 0);
    string typenameString = temp ? "typename " : "";

    ostr << endl;
    if (temp)
    {
      ostr << "template<" << args << ">" << endl;
    }
    ostr << "inline " << typenameString << "MakeReturn<BinaryNode<"
	 << opdef.tag() << "," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1)
	 << " >::Leaf_t," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class2.inputClass(2)
	 << " >::Leaf_t> >::Expression_t" << endl
	 << "" << opdef.function() << "(const " << class1.inputClass(1)
	 << " & l,const " << class2.inputClass(2) << " & r)" << endl
	 << "{" << endl
	 << "  typedef BinaryNode<" << opdef.tag() << "," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1)
	 << " >::Leaf_t," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class2.inputClass(2)
	 << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<" << class1.inputClass(1) << " >::make(l),"
	 << endl
	 << "    CreateLeaf<" << class2.inputClass(2) << " >::make(r)));"
	 << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class BinaryStdFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1,
	     const ClassDescriptor& class2) const
  {
    ostr << endl
	 << "template<"
	 << joinWithComma("class T1",class1.argDef(2),class2.argDef(3))
	 << ">" << endl
	 << "inline typename MakeReturn<BinaryNode<"
	 << opdef.tag() << "<T1>," << endl
	 << "  typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t," << endl
	 << "  typename CreateLeaf<" << class2.inputClass(3)
	 << " >::Leaf_t> >::Expression_t" << endl
	 << "" << opdef.function()
	 << "(const T1&,"
	 << " const " << class1.inputClass(2) << " & l,"
	 << " const " << class2.inputClass(3) << " & r)" << endl
	 << "{" << endl
	 << "  typedef BinaryNode<" << opdef.tag() << "<T1>," << endl
	 << "    typename CreateLeaf<" << class1.inputClass(2)
	 << " >::Leaf_t," << endl
	 << "    typename CreateLeaf<" << class2.inputClass(3)
	 << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<" << class1.inputClass(2) << " >::make(l),"
	 << endl
	 << "    CreateLeaf<" << class2.inputClass(3) << " >::make(r)));"
	 << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class TrinaryFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,
	     const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1,
	     const ClassDescriptor& class2,
	     const ClassDescriptor& class3) const
  {
    string args = joinWithComma(class1.argDef(1),
				class2.argDef(2),class3.argDef(3));
    bool temp = (args.size() > 0);
    string typenameString = temp ? "typename " : "";

    ostr << endl;
    if (temp)
    {
      ostr << "template<" << args << ">" << endl;
    }
    ostr << "inline " << typenameString << "MakeReturn<TrinaryNode<"
	 << opdef.tag() << "," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1)
	 << " >::Leaf_t," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class2.inputClass(2)
	 << " >::Leaf_t," << endl
	 << "  " << typenameString << "CreateLeaf<"
	 << class3.inputClass(3)
	 << " >::Leaf_t> >::Expression_t" << endl
	 << opdef.function() << "(const " << class1.inputClass(1)
	 << " & c,const " << class2.inputClass(2) << " & t,const "
	 << class3.inputClass(3) << " & f)"
	 << endl
	 << "{" << endl
	 << "  typedef TrinaryNode<" << opdef.tag() << "," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class1.inputClass(1)
	 << " >::Leaf_t," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class2.inputClass(2)
	 << " >::Leaf_t," << endl
	 << "    " << typenameString << "CreateLeaf<"
	 << class3.inputClass(3)
	 << " >::Leaf_t> Tree_t;" << endl
	 << "  return MakeReturn<Tree_t>::make(Tree_t(" << endl
	 << "    CreateLeaf<" << class1.inputClass(1) << " >::make(c),"
	 << endl
	 << "    CreateLeaf<" << class2.inputClass(2) << " >::make(t),"
	 << endl
	 << "    CreateLeaf<" << class3.inputClass(3) << " >::make(f)));"
	 << endl
	 << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
// AssignFunctionForClass
//
// Print an operator function that takes LHS and RHS and calls the user-defined
// function evaluate(LHS,OperatorTag,RHS).
// This function allows us to define all the assignment operations (except for
// operator= which is replaced by assign) for classes that don't define them
// as member functions.
//-----------------------------------------------------------------------------

class AssignFunctionForClass
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef,
	     const ClassDescriptor& class1) const
  {
    ostr
      << endl
      << "template<" << joinWithComma(class1.argDef(1),"class RHS")
      <<  ">" << endl
      << "inline" << endl
      << class1.inputClass(1) << "& " << opdef.function()
      << "(" << class1.inputClass(1) << "& lhs,const RHS& rhs)" << endl
      << "{" << endl
      << "  typedef typename CreateLeaf<RHS>::Leaf_t Leaf_t;" << endl
      << "  evaluate(lhs," << opdef.tag()
      << "(),MakeReturn<Leaf_t>::make(CreateLeaf<RHS>::make(rhs)));"
      << endl
      << "  return lhs;" << endl
      << "}" << endl;
  }
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class AssignFunction
{
public:
  template<class OSTR>
  void print(OSTR& ostr,const OperatorDescriptor& opdef) const
  {
    ostr
      << endl
      << "template<class LHS,class RHS>" << endl
      << "inline LHS&" << endl
      << opdef.function()
      << "(LHS& lhs,const RHS& rhs)" << endl
      << "{" << endl
      << "  typedef typename CreateLeaf<RHS>::Leaf_t Leaf_t;" << endl
      << "  evaluate(lhs," << opdef.tag()
      << "(),MakeReturn<Leaf_t>::make(CreateLeaf<RHS>::make(rhs)));"
      << endl
      << "  return lhs;" << endl
      << "}" << endl;
  }
};

//////////////////////////////////////////////////////////////////////

#endif     // PETE_TOOLS_PRINTFUNCTIONS_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PrintFunctions.h,v $   $Author: chicares $
// $Revision: 1.3 $   $Date: 2008-09-08 12:59:25 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
