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

#ifndef PETE_PETE_SCALAR_H
#define PETE_PETE_SCALAR_H

///////////////////////////////////////////////////////////////////////////////
//
// WARNING: THIS FILE IS FOR INTERNAL PETE USE. DON'T INCLUDE IT YOURSELF
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
// CLASS NAME
//    Scalar<T>
//
// DESCRIPTION
//    A wrapper around a scalar to be used in PETE expressions.
//
//-----------------------------------------------------------------------------

template<class T>
class Scalar
{
public:

  //---------------------------------------------------------------------------
  // Default constructor takes no action.

  inline
  Scalar() { }

  //---------------------------------------------------------------------------
  // Constructor from a single value.

  inline
  Scalar(const T &t) : scalar_m(t) { }

  template<class T1>
  inline
  explicit Scalar(const T1 &t) : scalar_m(t) { }

  //---------------------------------------------------------------------------
  // Constructor with arbitary second/third arguments, which is/are ignored.
  // Needed for compatibility with tree node constructors taking an
  // arbitrary argument.

  template<class Arg>
  inline
  Scalar(const Scalar<T> &s, const Arg &)
    : scalar_m(s.scalar_m) { }

  template<class Arg1, class Arg2>
  inline
  Scalar(const Scalar<T> &s, const Arg1 &, const Arg2 &)
    : scalar_m(s.scalar_m) { }

  //---------------------------------------------------------------------------
  // Copy constructor

  inline
  Scalar(const Scalar<T> &s) : scalar_m(s.scalar_m) { }

  //---------------------------------------------------------------------------
  // Return value.

  inline
  const T &value() const { return scalar_m; }

  //---------------------------------------------------------------------------
  // Assignment operators.

  inline
  Scalar<T> &operator=(const Scalar<T> &rhs)
  {
    scalar_m = rhs.scalar_m;

    return *this;
  }

  inline
  Scalar<T> &operator=(const T &rhs)
  {
    scalar_m = rhs;

    return *this;
  }

private:

  //---------------------------------------------------------------------------
  // The scalar value is stored here.

  T scalar_m;
};


#endif // PETE_PETE_SCALAR_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Scalar.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
