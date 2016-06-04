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

#ifndef PETE_PETE_COMBINERS_H
#define PETE_PETE_COMBINERS_H

///////////////////////////////////////////////////////////////////////////////
//
// WARNING: THIS FILE IS FOR INTERNAL PETE USE. DON'T INCLUDE IT YOURSELF
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
// CLASS NAMES
//   Combine1<A,Op,Tag>
//   Combine2<A,B,Op,Tag>
//   Combine3<A,B,C,Op,Tag>
//
// DESCRIPTION
//   A,B, and C are the types of the combined arguments
//   Op is the type of the operator from the expression tree
//   Tag is the tag type which selects the type of combiner operation
//
//   Users create their own combiner operations by specializing these
//   three structs for a particular user defined tag.  A typical example
//   might be an operation that compares the domains of all the leaves in
//   an expression.
//
//   Combine1 requires the user to define:
//      typedef ... Type_t;
//         - the return type of the combination
//      static Type_t combine(const A &a, const Tag &t) {}
//         - a function that combines a
//
//   Combine2 requires the user to define:
//      typedef ... Type_t;
//         - the return type of the combination
//      static Type_t combine(const A &a, const B &b,
//         const Tag &t) {}
//         - a function that combines a and b
//
//   Combine3 requires the user to define:
//      typedef ... Type_t;
//         - the return type of the combination
//      static Type_t combine(const A &a, const B &b, const C &c,
//         const Tag &t) {}
//         - a function that combines a and b and c
//
//   The default actions for combiners are:
//
//      Combine1 - returns the arguments
//      Combine2 - no default action... the user must define this at least
//      Combine3 - uses Combine2 to combine a and b and combine that with c
//
//-----------------------------------------------------------------------------

template<class A, class Op, class Tag>
struct Combine1
{
  typedef A Type_t;
  inline static
  Type_t combine(const A &a, const Tag &) { return a; }
};

template<class A, class B, class Op, class Tag>
struct Combine2
{
  // no default action.  It's an error to not specialize this struct.
};

template<class A,class B,class C,class Op,class Tag>
struct Combine3
{
  typedef typename Combine2<A, B, Op, Tag>::Type_t Type1_t;
  typedef typename Combine2<Type1_t, C, Op, Tag>::Type_t Type_t;
  inline static
  Type_t combine(const A& a, const B& b, const C& c, const Tag& t)
  {
    return
      Combine2<Type1_t, C,
      Op, Tag>::combine(Combine2<A, B, Op, Tag>::combine(a,b,t),c,t);
  }
};

//-----------------------------------------------------------------------------
//
// FUNCTION NAME
//   peteCombine()
//
// DESCRIPTION
//   These functions provide a simple user interface to PETE's CombineN
//   functors.  For example:
//
//   peteCombine(a, b, op, tag);
//
//   is a synonym for:
//
//   Combine2<A, B, Op, Tag>::combine(a, b, tag);
//
//-----------------------------------------------------------------------------

template<class A, class Op, class Tag>
inline typename Combine1<A, Op, Tag>::Type_t
peteCombine(const A &a, const Op &, const Tag &t)
{
  return Combine1<A, Op, Tag>::combine(a, t);
}

template<class A, class B, class Op, class Tag>
inline typename Combine2<A, B, Op, Tag>::Type_t
peteCombine(const A &a, const B &b, const Op &, const Tag &t)
{
  return Combine2<A, B, Op, Tag>::combine(a, b, t);
}

template<class A, class B, class C, class Op, class Tag>
inline typename Combine3<A, B, C, Op, Tag>::Type_t
peteCombine(const A &a, const B &b, const C &c, const Op &, const Tag &t)
{
  return Combine3<A, B, C, Op, Tag>::combine(a, b, c, t);
}

//-----------------------------------------------------------------------------
//
// CLASS NAME
//   TreeCombine
//
// DESCRIPTION
//   This combiner is provided so that the user can write a functor
//   that modifies the leaves of an expression and use
//   ForEach<Expr,UserFunctor,TreeCombine> to get the entire expression
//   back with modified leaves.
//
//   This type of operation is useful when expressions are being
//   passed around.  For example, you might subset an expression and pass
//   that result to another function.  The type of expression tree that you
//   are passing has subsetted nodes, and could be a different type from the
//   original expression.
//
//-----------------------------------------------------------------------------

struct TreeCombine
{
  PETE_EMPTY_CONSTRUCTORS(TreeCombine)
};

template<class A, class Op>
struct Combine1<A, Op, TreeCombine >
{
  typedef UnaryNode<Op, A> Type_t;
  inline static
  Type_t combine(const A &a, const TreeCombine &)
  {
    return Type_t(a);
  }
};

template<class A, class B, class Op>
struct Combine2<A, B, Op, TreeCombine >
{
  typedef BinaryNode<Op, A, B> Type_t;
  inline static
  Type_t combine(const A &a, const B &b, const TreeCombine &)
  {
    return Type_t(a, b);
  }
};

template<class A, class B, class C, class Op>
struct Combine3<A, B, C, Op, TreeCombine >
{
  typedef TrinaryNode<Op, A, B, C> Type_t;
  inline static
  Type_t combine(const A &a, const B &b, const C &c, const TreeCombine &)
  {
    return Type_t(a, b, c);
  }
};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   OpCombine
//
// DESCRIPTION
//   A combiner that uses the operations in the expression tree.
//
//-----------------------------------------------------------------------------

struct OpCombine
{
  PETE_EMPTY_CONSTRUCTORS(OpCombine)
};

template<class A,class Op>
struct Combine1<A, Op, OpCombine>
{
  typedef typename UnaryReturn<A, Op>::Type_t Type_t;
  inline static
  Type_t combine(A a, OpCombine) { return Op()(a); }
};

template<class A,class B,class Op>
struct Combine2<A, B, Op, OpCombine>
{
  typedef typename BinaryReturn<A, B, Op>::Type_t Type_t;
  inline static
  Type_t combine(A a, B b, OpCombine)
  {
    return Op()(a, b);
  }
};

template<class A,class B,class C,class Op>
struct Combine3<A, B, C, Op, OpCombine>
{
  typedef typename TrinaryReturn<A, B, C, Op>::Type_t Type_t;
  inline static
  Type_t combine(A a, B b, C c, OpCombine)
  {
    return Op()(a, b, c);
  }
};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   AndCombine
//
// DESCRIPTION
//   A handy combiner for extracting bool queries from expressions.
//
//-----------------------------------------------------------------------------

struct AndCombine
{
  PETE_EMPTY_CONSTRUCTORS(AndCombine)
};

template<class Op>
struct Combine2<bool, bool, Op, AndCombine>
{
  typedef bool Type_t;
  inline static
  Type_t combine(bool a, bool b, AndCombine)
  {
    return (a && b);
  }
};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   OrCombine
//
// DESCRIPTION
//   A handy combiner for extracting bool queries from expressions.
//
//-----------------------------------------------------------------------------

struct OrCombine
{
  PETE_EMPTY_CONSTRUCTORS(OrCombine)
};

template<class Op>
struct Combine2<bool, bool, Op, OrCombine>
{
  typedef bool Type_t;
  inline static
  Type_t combine(bool a, bool b, OrCombine)
  {
    return (a || b);
  }
};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   NullCombine
//
// DESCRIPTION
//   This combiner doesn't do anything. Used when the combination isn't meant
//   to return anything since combiners cannot return void.
//
//-----------------------------------------------------------------------------

struct NullCombine
{
  PETE_EMPTY_CONSTRUCTORS(NullCombine)
};

template<class Op>
struct Combine2<int, int, Op, NullCombine>
{
  typedef int Type_t;
  inline static
  Type_t combine(int, int, NullCombine)
  {
    return 0;
  }
};


//-----------------------------------------------------------------------------
//
// CLASS NAME
//   SumCombine
//
// DESCRIPTION
//   This combiner can be used to count things in expressions.
//
//-----------------------------------------------------------------------------

struct SumCombine
{
  PETE_EMPTY_CONSTRUCTORS(SumCombine)
};

template<class Op>
struct Combine2<int, int, Op, SumCombine>
{
  typedef int Type_t;
  inline static
  Type_t combine(int a, int b, SumCombine)
  {
    return a + b;
  }
};


#endif // PETE_PETE_COMBINERS_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Combiners.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:19 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
