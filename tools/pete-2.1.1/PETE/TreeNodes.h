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

#ifndef PETE_PETE_TREENODES_H
#define PETE_PETE_TREENODES_H

///////////////////////////////////////////////////////////////////////////////
//
// WARNING: THIS FILE IS FOR INTERNAL PETE USE. DON'T INCLUDE IT YOURSELF
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
// CLASS NAME
//   Reference<T>
//
// DESCRIPTION
//   Reference is a special kind of node that contains a reference to an object
//   of type T.  It can be converted to a (const T &), and other tree objects
//   will perform this conversion before returning their elements.
//
//-----------------------------------------------------------------------------

template<class T>
struct Reference
{
  //---------------------------------------------------------------------------
  // Export the type of thing we're referencing.

  typedef T Type_t;

  //---------------------------------------------------------------------------
  // Reference can be created from a const ref.

  inline
  Reference(const T &reference)
    : reference_m(reference)
  { }

  //---------------------------------------------------------------------------
  // Copy constructor

  inline
  Reference(const Reference<T> &model)
    : reference_m(model.reference())
  { }

  //---------------------------------------------------------------------------
  // Reference can be converted to a const ref

  inline
  const T &reference() const
  {
    return reference_m;
  }

  //---------------------------------------------------------------------------
  // Conversion operators.

  operator const T& () const { return reference_m; }
  operator T& () const { return const_cast<T&>(reference_m); }

  const T &reference_m;
};

//-----------------------------------------------------------------------------
//
// CLASS NAME
//   DeReference<T>
//
// DESCRIPTION
//   DeReference is a simple traits class that unwraps the Reference struct.
//   If T is not a reference object then DeReference gives (const T &).
//   If T is a Reference object, then DeReference gives a const ref to the
//   wrapped object.
//
//-----------------------------------------------------------------------------

template<class T>
struct DeReference
{
  typedef const T &Return_t;
  typedef T Type_t;
  static inline Return_t apply(const T &a) { return a; }
};

template<class T>
struct DeReference<Reference<T> >
{
  typedef const T &Return_t;
  typedef T Type_t;
  static inline Return_t apply(const Reference<T> &a) { return a.reference(); }
};

//-----------------------------------------------------------------------------
//
// CLASS NAME
//   UnaryNode<Op, Child>
//
// DESCRIPTION
//   A tree node for representing unary expressions. The node holds a
//   child (of type Child), which is the type of the expression sub tree and a
//   an operation (of type Op), which is typically the operation applied to
//   the sub tree.
//
//-----------------------------------------------------------------------------

template<class Op, class Child>
class UnaryNode
{
public:

  inline
  typename DeReference<Child>::Return_t
  child() const { return DeReference<Child>::apply(child_m); }

  //---------------------------------------------------------------------------
  // Constructor using just the child.

  inline
  UnaryNode(const Child &c)
    : child_m(c) { }

  //---------------------------------------------------------------------------
  // Copy constructor.

  inline
  UnaryNode(const UnaryNode<Op, Child> &t)
    : child_m(t.child()) { }

  //---------------------------------------------------------------------------
  // Constructor using a UnaryNode with a different child and/or a different
  // storage tag. Note: for this to work, a Child must be constructable
  // from an OtherChild.

  template<class OtherChild>
  inline
  UnaryNode(const UnaryNode<Op, OtherChild> &t)
    : child_m(t.child()) { }

  //---------------------------------------------------------------------------
  // Constructor using a UnaryNode with a different child,
  // some arbitrary argument, and a different storage tag.
  // Note: for this to work, a Child must be constructable
  // from an OtherChild and an Arg.

  template<class OtherChild, class Arg>
  inline
  UnaryNode(const UnaryNode<Op, OtherChild> &t, const Arg &a)
    : child_m(t.child(), a) { }

  //---------------------------------------------------------------------------
  // Constructor using a BinaryNode with a different Child and
  // two arbitrary arguments.
  // Note: for this to work, a Child  must be constructable
  // from an OtherChild and an Arg1 & Arg2.

  template<class OtherChild, class Arg1, class Arg2>
  inline
  UnaryNode(const UnaryNode<Op, OtherChild> &t,
	    const Arg1 &a1, const Arg2 &a2)
    : child_m(t.child(), a1, a2)
    { }

private:

  Child child_m;

};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   BinaryNode<Op, Left, Right>
//
// DESCRIPTION
//   A tree node for representing binary expressions. The node holds a
//   left child (of type Left), which is the type of the LHS expression
//   sub tree, a right child (of type Right), which is the type of the RHS
//   expression sub tree, and an operation (of type OP), which is applied
//   to the two sub trees.
//
//-----------------------------------------------------------------------------

template<class Op, class Left, class Right>
class BinaryNode
{
public:

  inline
  typename DeReference<Left>::Return_t
  left() const { return DeReference<Left>::apply(left_m); }

  inline
  typename DeReference<Right>::Return_t
  right() const { return DeReference<Right>::apply(right_m); }

  //---------------------------------------------------------------------------
  // Constructor using just the two children.

  inline
  BinaryNode(const Left &l, const Right &r)
    : left_m(l), right_m(r)
  { }

  //---------------------------------------------------------------------------
  // Copy constructor.

  inline
  BinaryNode(const BinaryNode<Op, Left, Right> &t)
    : left_m(t.left()), right_m(t.right())
  { }

  //---------------------------------------------------------------------------
  // Constructor using a BinaryNode with a different Left/Right.
  // Note: for this to work, the Left/Right must be constructable
  // from an OtherLeft/OtherRight.

  template<class OtherLeft, class OtherRight>
  inline
  BinaryNode(const BinaryNode<Op, OtherLeft, OtherRight> &t)
    : left_m(t.left()), right_m(t.right())
  { }

  //---------------------------------------------------------------------------
  // Constructor using a BinaryNode with a different Left/Right and
  // some arbitrary argument.
  // Note: for this to work, a Left/Right must be constructable
  // from an OtherLeft/OtherRight and an Arg.

  template<class OtherLeft, class OtherRight, class Arg>
  inline
  BinaryNode(const BinaryNode<Op, OtherLeft, OtherRight> &t,
	     const Arg &a)
    : left_m(t.left(), a), right_m(t.right(), a)
  { }

  //---------------------------------------------------------------------------
  // Constructor using a BinaryNode with a different Left/Right and
  // two arbitrary arguments.
  // Note: for this to work, a Left/Right must be constructable
  // from an OtherLeft/OtherRight and an Arg1 & Arg2.

  template<class OtherLeft, class OtherRight, class Arg1, class Arg2>
  inline
  BinaryNode(const BinaryNode<Op, OtherLeft, OtherRight> &t,
	     const Arg1 &a1, const Arg2 &a2)
    : left_m(t.left(), a1, a2), right_m(t.right(), a1, a2)
  { }

private:

  //---------------------------------------------------------------------------
  // The operation and left/right sub expressions stored in this node of the
  // tree.

  Left  left_m;
  Right right_m;

};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   TrinaryNode<Op, Left, Middle, Right>
//
// DESCRIPTION
//   A tree node for representing trinary expressions. The node holds a
//   Left child (of type Left), which is the type of the LHS expression
//   sub tree (typically a comparison operation); a Middle child (of type
//   Middle), which is the type of the middle (true branch) expression
//   sub tree; a Right child (of type Right), which is the type of
//   the expression (false branch) sub tree; and an operation (of type Op),
//   which is applied to the three sub trees.
//
//-----------------------------------------------------------------------------

template< class Op, class Left, class Middle, class Right>
class TrinaryNode
{
public:

  inline
  typename DeReference<Left>::Return_t
  left() const { return DeReference<Left>::apply(left_m); }

  inline
  typename DeReference<Right>::Return_t
  right() const { return DeReference<Right>::apply(right_m); }

  inline
  typename DeReference<Middle>::Return_t
  middle() const { return DeReference<Middle>::apply(middle_m); }

  //---------------------------------------------------------------------------
  // Constructor with just the three children.

  inline
  TrinaryNode(const Left &l, const Middle &m, const Right &r)
    : left_m(l), middle_m(m), right_m(r)
  { }

  //---------------------------------------------------------------------------
  // Copy constructor.

  inline
  TrinaryNode(const TrinaryNode<Op, Left, Middle, Right> &t)
    : left_m(t.left()), middle_m(t.middle()), right_m(t.right())
  { }

  //---------------------------------------------------------------------------
  // Constructor using a TrinaryNode with a different Left/Middle/Right.
  // Note: for this to work, the Left/Middle/Right must be constructable
  // from an OtherLeft/OtherMiddle/OtherRight.

  template<class OtherLeft, class OtherMiddle, class OtherRight>
  inline
  TrinaryNode(const TrinaryNode<Op, OtherLeft, OtherMiddle, OtherRight> & t)
    : left_m(t.left()), middle_m(t.middle()), right_m(t.right())
  { }

  //---------------------------------------------------------------------------
  // Constructor using a TrinaryNode with a different Left/Middle/Right and
  // some arbitrary argument.
  // Note: for this to work, a Left/Middle/Right must be constructable
  // from an OtherLeft/OtherMiddle/OtherRight and an Arg.

  template<class OtherLeft, class OtherMiddle, class OtherRight, class Arg>
  inline
  TrinaryNode(const TrinaryNode<Op, OtherLeft, OtherMiddle, OtherRight> &t,
	      const Arg &a)
    : left_m(t.left(), a), middle_m(t.middle(), a), right_m(t.right(), a)
  { }

  //---------------------------------------------------------------------------
  // Constructor using a TrinaryNode with a different Left/Middle/Right and
  // two arbitrary arguments.
  // Note: for this to work, a Left/Middle/Right must be constructable
  // from an OtherLeft/OtherMiddle/OtherRight and an Arg1 & Arg2.

  template<class OtherLeft, class OtherMiddle, class OtherRight,
    class Arg1, class Arg2>
  inline
  TrinaryNode(const TrinaryNode<Op, OtherLeft, OtherMiddle, OtherRight> &t,
	      const Arg1 &a1, const Arg2 &a2)
    : left_m(t.left(), a1, a2),
      middle_m(t.middle(), a1, a2) , right_m(t.right(), a1, a2)
  { }

private:

  //---------------------------------------------------------------------------
  // The operation and left, right, and middle sub trees stored at this node.

  Left   left_m;
  Middle middle_m;
  Right  right_m;

};

#endif // PETE_PETE_TREENODES_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: TreeNodes.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
