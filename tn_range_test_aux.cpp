// Trammeled Numeric range type: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: tn_range_test_aux.cpp,v 1.6 2008-12-27 02:56:58 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "tn_range.tpp"          // Template class implementation.
#include "tn_range_test_aux.hpp" // Trammel class.

// Normally, one would prefer to instantiate all tn_range templates
// in a single file. Here, however, the point is to instantiate one
// such template in a different translation unit than the unit-test
// driver, in order to prove that explicit instantion works.

template class tn_range<int, percentage_trammel<int> >;

