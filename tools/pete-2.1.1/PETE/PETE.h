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

#ifndef PETE_PETE_PETE_H
#define PETE_PETE_PETE_H

///////////////////////////////////////////////////////////////////////////////
//
// This is the header file you should include if you want to use PETE, the
// Portable Expression Template Engine. You don't need add any member
// functions to make your container class PETE-aware, but you will need to
// create some traits classes and use the MakeOperator tool to create
// operator and math functions.
//
// See html/index.html for detailed instructions on using PETE.
//
///////////////////////////////////////////////////////////////////////////////

#if defined PETE_MAKE_EMPTY_CONSTRUCTORS && PETE_MAKE_EMPTY_CONSTRUCTORS

#define PETE_EMPTY_CONSTRUCTORS(CLASS)  \
  CLASS() { }   \
  CLASS(const CLASS &) { } \
  CLASS &operator=(const CLASS &) { return *this; }

#define PETE_EMPTY_CONSTRUCTORS_TEMPLATE(CLASS, ARG)  \
  CLASS() { }   \
  CLASS(const CLASS<ARG> &) { } \
  CLASS &operator=(const CLASS<ARG> &) { return *this; }

#else

#define PETE_EMPTY_CONSTRUCTORS(CLASS)
#define PETE_EMPTY_CONSTRUCTORS_TEMPLATE(CLASS, ARG)

#endif

#include "PETE/Scalar.h"
#include "PETE/TypeComputations.h"
#include "PETE/TreeNodes.h"
#include "PETE/OperatorTags.h"
#include "PETE/Functors.h"
#include "PETE/Combiners.h"
#include "PETE/ForEach.h"
#include "PETE/CreateLeaf.h"

// Some useful PETE definitions.

#define PETE_MAJOR_VERSION                 2
#define PETE_MINOR_VERSION                 1
#define PETE_PATCH_LEVEL                   1
#define PETE_VERSION_STRING                "PETE 2.1.1"
#define PETE_VERSION_NUM_STRING            "2.1.1"

// Pooma Extensions:

#include "PETE/PoomaPETE.h"

#endif // PETE_PETE_PETE_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PETE.h,v $   $Author: chicares $
// $Revision: 1.3 $   $Date: 2008-09-09 02:47:15 $
// ----------------------------------------------------------------------
// ACL:rcsinfo

