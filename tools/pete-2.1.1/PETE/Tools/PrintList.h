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
// **CLASSNAME**
//-----------------------------------------------------------------------------

#ifndef PETE_TOOLS_PRINTLIST_H
#define PETE_TOOLS_PRINTLIST_H

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

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Full Description:
//
//-----------------------------------------------------------------------------

template<class OSTR,class PROTOTYPE,class LIST1>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1)
{
  typename LIST1::const_iterator i1;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    prototype.print(ostr,*i1);
  }
}

template<class OSTR,class PROTOTYPE,class LIST1,class LIST2>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      prototype.print(ostr,*i1,*i2);
    }
  }
}

template<class OSTR,class PROTOTYPE,class LIST1,class LIST2,class LIST3>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2,const LIST3& list3)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  typename LIST3::const_iterator i3;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      for (i3 = list3.begin();i3 != list3.end(); ++i3)
      {
	prototype.print(ostr,*i1,*i2,*i3);
      }
    }
  }
}

template<class OSTR,class PROTOTYPE,
  class LIST1,class LIST2,class LIST3,class LIST4>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2,const LIST3& list3,const LIST4& list4)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  typename LIST3::const_iterator i3;
  typename LIST4::const_iterator i4;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      for (i3 = list3.begin();i3 != list3.end(); ++i3)
      {
	for (i4 = list4.begin();i4 != list4.end(); ++i4)
	{
	  prototype.print(ostr,*i1,*i2,*i3,*i4);
	}
      }
    }
  }
}

#endif     // PETE_TOOLS_PRINTLIST_H

//////////////////////////////////////////////////////////////////////

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PrintList.h,v $   $Author: chicares $
// $Revision: 1.2 $   $Date: 2008-09-07 17:38:20 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
