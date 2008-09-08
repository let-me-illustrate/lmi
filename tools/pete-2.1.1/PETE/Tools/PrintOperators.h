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
// UnaryOp
// UnarySpecialOp
// UnaryBoolOp
// UnaryCastOp
// UnaryStdOp
// BinaryOp
// BinarySpecialOp
// BinaryBoolOp
// BinaryLeftOp
// BinaryStdOp
// BinaryAssignOp
// BinaryAssignBoolOp
// TrinaryOp
// InsertOp
//-----------------------------------------------------------------------------

#ifndef PETE_TOOLS_PRINTOPERATORS_H
#define PETE_TOOLS_PRINTOPERATORS_H

//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Overview:
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

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Full Description:
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// UnaryOp
//
// This class prints the basic unary operator tag for operations that pass
// the type of the argument through.
//-----------------------------------------------------------------------------

class UnaryOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    ostr << endl;
    if (opdef.templateArgs())
    {
      ostr << "template<" << opdef.argDef() << ">" << endl;
    }
    ostr << "struct " << opdef.tag(false) << endl
	 << "{" << endl;
    if (opdef.templateArgs())
    {
      ostr << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	   << opdef.tag(false) << "," << opdef.argDef() << ")"
	   << endl;
    }
    else
    {
      ostr << "  PETE_EMPTY_CONSTRUCTORS(" << opdef.tag(false) << ")"
	   << endl;
    }
    ostr << "  template<class T>" << endl
	 << "  inline typename UnaryReturn<T, " << opdef.tag()
	 << " >::Type_t" << endl
	 << "  operator()(const T &a) const" << endl
	 << "  {" << endl
	 << "    " << opdef.expression() << endl
	 << "  }" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// UnarySpecialOp
//
// This class prints the operator tag for unary operations that compute the
// return type by specializing the UnaryReturn struct. Tag is the same
// as a unary-op since a UnaryReturn<> struct must be provided in either
// case.
//-----------------------------------------------------------------------------

class UnarySpecialOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    UnaryOp().print(ostr, opdef);
  }
};

//-----------------------------------------------------------------------------
// UnaryBoolOp
//
// This class prints the operator tag for unary operations that return
// bools.
//-----------------------------------------------------------------------------

class UnaryBoolOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print Tag:

    UnaryOp().print(ostr, opdef);

    // Print UnaryReturn specialization:

    string args = joinWithComma("class T", opdef.argDef());

    ostr << endl << "template<" << args << " >" << endl
	 << "struct UnaryReturn<T, " << opdef.tag() << " > {"
	 << endl
	 << "  typedef bool Type_t;" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// UnaryCastOp
//
// This class prints the operator tag for unary operations that perform
// cast expressions.
//-----------------------------------------------------------------------------

class UnaryCastOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print Tag:

    ostr << endl
	 << "template <class T1>" << endl
	 << "struct " << opdef.tag() << "" << endl
	 << "{" << endl
	 << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	 << opdef.tag(false) << ", T1)" << endl
	 << "  template<class T2>" << endl
	 << "  inline typename UnaryReturn<T2, "
	 << opdef.tag() << "<T1> >::Type_t" << endl
	 << "  operator()(const T2 &a) const" << endl
	 << "  {" << endl
	 << "    " << opdef.expression() << endl
	 << "  }" << endl
	 << "};" << endl;

    // Print UnaryReturn specialization:

    ostr << endl << "template<class T1, class T2>" << endl
	 << "struct UnaryReturn<T2, " << opdef.tag() << "<T1> > {"
	 << endl
	 << "  typedef T1 Type_t;" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// UnaryStdOp
//
// This class prints the operator tag for std::unary_function.
//-----------------------------------------------------------------------------

class UnaryStdOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print Tag:

    ostr << endl
	 << "template <class T1>" << endl
	 << "struct " << opdef.tag() << "" << endl
	 << "{" << endl
	 << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	 << opdef.tag(false) << ", T1)" << endl
	 << "  template<class T2>" << endl
	 << "  inline typename UnaryReturn<T2, "
	 << opdef.tag() << "<T1> >::Type_t" << endl
	 << "  operator()(const T2 &a) const" << endl
	 << "  {" << endl
	 << "    " << opdef.expression() << endl
	 << "  }" << endl
	 << "};" << endl;

    // Print UnaryReturn specialization:

    ostr << endl << "template<class T1, class T2>" << endl
	 << "struct UnaryReturn<T2, " << opdef.tag() << "<T1> > {"
	 << endl
	 << "  typedef typename T1::result_type Type_t;" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// BinaryOp
//
// This class prints the operator tag for binary operations that compute
// their return type in the default manner (by promotion).
//-----------------------------------------------------------------------------

class BinaryOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    ostr << endl;
    if (opdef.templateArgs())
      {
        ostr << "template<" << opdef.argDef() << ">" << endl;
      }
    ostr << "struct " << opdef.tag(false) << "" << endl
         << "{" << endl;
    if (opdef.templateArgs())
      {
	ostr << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	     << opdef.tag(false) << "," << opdef.argDef() << ")"
	     << endl;
      }
    else
      {
	ostr << "  PETE_EMPTY_CONSTRUCTORS(" << opdef.tag(false) << ")"
	     << endl;
      }
    ostr << "  template<class T1, class T2>" << endl
         << "  inline typename BinaryReturn<T1, T2, "
         << opdef.tag() << " >::Type_t" << endl
         << "  operator()(const T1 &a, const T2 &b) const"
         << endl
         << "  {" << endl
         << "    " << opdef.expression() << endl
         << "  }" << endl
         << "};" << endl;
  }
private:
};

//-----------------------------------------------------------------------------
// BinarySpecialOp
//
// This class prints the operator tag for binary operations that compute
// their return type using a specialization of BinaryReturn.
//-----------------------------------------------------------------------------

class BinarySpecialOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    BinaryOp().print(ostr, opdef);
   }
private:
};

//-----------------------------------------------------------------------------
// BinaryBoolOp
//
// This class prints the operator tag for binary operations that return
// bools. Also generates the appropriate BinaryReturn specialization.
//-----------------------------------------------------------------------------

class BinaryBoolOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print tag:

    BinaryOp().print(ostr, opdef);

    // Print BinaryReturn specialization:

    string args = joinWithComma("class T1, class T2",
				     opdef.argDef());

    ostr << endl << "template<" << args << " >" << endl
	 << "struct BinaryReturn<T1, T2, "
	 << opdef.tag() << " > {"
	 << endl
	 << "  typedef bool Type_t;" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// BinaryLeftOp
//
// This class prints the operator tag for binary operations that return
// the left operand. Also generates the appropriate BinaryReturn
// specialization.
//-----------------------------------------------------------------------------

class BinaryLeftOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print tag:

    BinaryOp().print(ostr, opdef);

    // Print BinaryReturn specialization:

    string args = joinWithComma("class T1, class T2",
                                     opdef.argDef());

    ostr << endl << "template<" << args << " >" << endl
         << "struct BinaryReturn<T1, T2, "
         << opdef.tag() << " > {"
         << endl
         << "  typedef T1 Type_t;" << endl
         << "};" << endl;
   }
};

//-----------------------------------------------------------------------------
// BinaryStdOp
//
// This class prints the operator tag for std::binary_function.
//-----------------------------------------------------------------------------

class BinaryStdOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print Tag:

    ostr << endl
	 << "template <class T1>" << endl
	 << "struct " << opdef.tag() << "" << endl
	 << "{" << endl
	 << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	 << opdef.tag(false) << ", T1)" << endl
	 << "  template<class T2, class T3>" << endl
	 << "  inline typename BinaryReturn<T2, T3, "
	 << opdef.tag() << "<T1> >::Type_t" << endl
	 << "  operator()(const T2 &a, const T3 &b) const" << endl
	 << "  {" << endl
	 << "    " << opdef.expression() << endl
	 << "  }" << endl
	 << "};" << endl;

    // Print BinaryReturn specialization:

    ostr << endl << "template<class T1, class T2, class T3>" << endl
	 << "struct BinaryReturn<T2, T3, " << opdef.tag() << "<T1> > {"
	 << endl
	 << "  typedef typename T1::result_type Type_t;" << endl
	 << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// BinaryAssignOp
//
// This class prints the operator tag for assignment operations.
//-----------------------------------------------------------------------------

class BinaryAssignOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print tag

    ostr << endl;
    if (opdef.templateArgs())
      {
        ostr << "template<" << opdef.argDef() << ">" << endl;
      }
    ostr << "struct " << opdef.tag(false) << "" << endl
         << "{" << endl;
    if (opdef.templateArgs())
      {
	ostr << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	     << opdef.tag(false) << "," << opdef.argDef() << ")"
	     << endl;
      }
    else
      {
	ostr << "  PETE_EMPTY_CONSTRUCTORS(" << opdef.tag(false) << ")"
	     << endl;
      }
    ostr << "  template<class T1, class T2>" << endl
         << "  inline typename BinaryReturn<T1, T2, "
         << opdef.tag() << " >::Type_t" << endl
         << "  operator()(const T1 &a, const T2 &b) const"
         << endl
         << "  {" << endl
         << "    " << opdef.expression() << endl
         << "  }" << endl
         << "};" << endl;

    // Print BinaryReturn specialization:

    string args = joinWithComma("class T1, class T2",
                                     opdef.argDef());

    ostr << endl << "template<" << args << " >" << endl
         << "struct BinaryReturn<T1, T2, "
         << opdef.tag() << " > {"
         << endl
         << "  typedef T1 &Type_t;" << endl
         << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// BinaryAssignBoolOp
//
// This class prints the operator tag for assignment operations, which (for
// reasons that escape me -- SWH) might return a bool.
//-----------------------------------------------------------------------------

class BinaryAssignBoolOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print tag:

    ostr << endl;
    if (opdef.templateArgs())
      {
        ostr << "template<" << opdef.argDef() << ">" << endl;
      }
    ostr << "struct " << opdef.tag(false) << "" << endl
         << "{" << endl;
    if (opdef.templateArgs())
      {
        ostr << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
             << opdef.tag(false) << "," << opdef.argDef() << ")"
             << endl;
      }
    else
      {
        ostr << "  PETE_EMPTY_CONSTRUCTORS(" << opdef.tag(false) << ")"
             << endl;
      }
    ostr << "  template<class T1, class T2>" << endl
         << "  inline typename BinaryReturn<T1, T2, "
         << opdef.tag() << " >::Type_t" << endl
         << "  operator()(const T1 &a, const T2 &b) const"
         << endl
         << "  {" << endl
         << "    " << opdef.expression() << endl
         << "  }" << endl
         << "};" << endl;

    // Print BinaryReturn specialization:

    string args = joinWithComma("class T1, class T2",
                                     opdef.argDef());

    ostr << endl << "template<" << args << " >" << endl
         << "struct BinaryReturn<T1, T2, "
         << opdef.tag() << " > {"
         << endl
         << "  typedef bool Type_t;" << endl
         << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// TrinaryOp
//
// This class prints the operator tag for trinary operations.
//-----------------------------------------------------------------------------

class TrinaryOp
{
public:
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    // Print tag:

    ostr << endl;
    if (opdef.templateArgs())
      {
        ostr << "template<" << opdef.argDef() << ">" << endl;
      }
    ostr << "struct " << opdef.tag(false) << "" << endl
         << "{" << endl;
    if (opdef.templateArgs())
    {
      ostr << "  PETE_EMPTY_CONSTRUCTORS_TEMPLATE("
	   << opdef.tag(false) << "," << opdef.argDef() << ")"
	   << endl;
    }
    else
    {
      ostr << "  PETE_EMPTY_CONSTRUCTORS(" << opdef.tag(false) << ")"
	   << endl;
    }
    ostr << "  template<class T1, class T2, "
	 << "class T3>" << endl
         << "  inline typename "
	 << "TrinaryReturn<T1, T2, T3, "
	 << opdef.tag() << " >" << endl
	 << "  ::Type_t" << endl
         << "  operator()(T1 &a, const T2 &b, const T3 &c) const" << endl
         << "  {" << endl
         << "    " << opdef.expression() << endl
         << "  }" << endl
         << "};" << endl;
  }
};

//-----------------------------------------------------------------------------
// InsertOp
//
// This functor converts an operator descriptor into C++ code that can insert
// the operator into a map of vectors of operator descriptors.
// InsertOp should be constructed with the operator type string that says where
// in the map this type of operator is found.
//-----------------------------------------------------------------------------

class InsertOp
{
public:
  InsertOp(const string& optype) : optype_m(optype) { }
  template<class OSTR,class OPDEF>
  void print(OSTR& ostr,const OPDEF& opdef) const
  {
    ostr
      << "  m[\"" << optype_m << "\"].push_back(" << endl
      << "              OperatorDescriptor(\""
      << opdef.tag() << "\"," << endl
      << "                                 \""
      << opdef.function() << "\"," << endl
      << "                                 \""
      << opdef.expression() << "\"," << endl
      << "                                 \""
      << opdef.argDef() << "\"));"
      << endl;
  }
private:
  string optype_m;
};

#endif     // PETE_TOOLS_PRINTOPERATORS_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PrintOperators.h,v $   $Author: chicares $
// $Revision: 1.4 $   $Date: 2008-09-08 12:59:25 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
