// -*- C++ -*-
// ACL:license
// ----------------------------------------------------------------------
// This software and ancillary information (herein called "SOFTWARE")
// called PETE (Portable Expression Template Engine) is
// made available under the terms described here.  The SOFTWARE has been
// approved for release with associated LA-CC Number LA-CC-99-5.
// 
// Unless otherwise indicated, this SOFTWARE has been authored by an
// employee or employees of the University of California, operator of the
// Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
// the U.S. Department of Energy.  The U.S. Government has rights to use,
// reproduce, and distribute this SOFTWARE. The public may copy, distribute,
// prepare derivative works and publicly display this SOFTWARE without 
// charge, provided that this Notice and any statement of authorship are 
// reproduced on all copies.  Neither the Government nor the University 
// makes any warranty, express or implied, or assumes any liability or 
// responsibility for the use of this SOFTWARE.
// 
// If SOFTWARE is modified to produce derivative works, such modified
// SOFTWARE should be clearly marked, so as not to confuse it with the
// version available from LANL.
// 
// For more information about PETE, send e-mail to pete@acl.lanl.gov,
// or visit the PETE web page at http://www.acl.lanl.gov/pete/.
// ----------------------------------------------------------------------
// ACL:license

#include "Vec3.h"

int main()
{
  Vec3 a, b, c;

  c = 4;

  b[0] = -1;
  b[1] = -2;
  b[2] = -3;

  a = b + c;

  cout << a << endl;

  Vec3 d;
  const Expression<BinaryNode<OpAdd,
    Reference<Vec3>, Reference<Vec3> > > &expr1 = b + c;
  d = expr1;
  cout << d << endl;
  
  int num = forEach(expr1, CountLeaf(), SumCombine());
  cout << num << endl;

  const Expression<BinaryNode<OpAdd, Reference<Vec3>, 
    BinaryNode<OpMultiply, Scalar<int>,
    Reference<Vec3> > > > &expr2 = b + 3 * c;
  num = forEach(expr2, CountLeaf(), SumCombine());
  cout << num << endl;
  
  const Expression<BinaryNode<OpAdd, Reference<Vec3>, 
    BinaryNode<OpMultiply, Reference<Vec3>,
    Reference<Vec3> > > > &expr3 = b + c * d;
  num = forEach(expr3, CountLeaf(), SumCombine());
  cout << num << endl;
}

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Vec3.cpp,v $   $Author: swhaney $
// $Revision: 1.8 $   $Date: 2000/01/04 01:40:16 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
