// Declaration of specific tn_range types.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: tn_range_types.hpp,v 1.3 2005-03-10 04:44:06 chicares Exp $

#ifndef tn_range_types_hpp
#define tn_range_types_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"           // Template class forward declaration.
#include "tn_range_type_trammels.hpp" // Trammels for specific tn_range types.

// TODO ?? Move these two to a 'test' file.

typedef tn_range<int   , percentage_trammel<int   > > r_int_percentage;
typedef tn_range<double, percentage_trammel<double> > r_double_percentage;

typedef tn_range<double, proportion_trammel     <double> > tnr_proportion;
typedef tn_range<double, nonnegative_trammel    <double> > tnr_nonnegative_double;
typedef tn_range<int   , nonnegative_trammel    <int   > > tnr_nonnegative_integer;
typedef tn_range<int   , issue_age_trammel      <int   > > tnr_issue_age;
typedef tn_range<int   , attained_age_trammel   <int   > > tnr_attained_age;
typedef tn_range<int   , duration_trammel       <int   > > tnr_duration;
typedef tn_range<int   , month_trammel          <int   > > tnr_month;
typedef tn_range<double, corridor_factor_trammel<double> > tnr_corridor_factor;
typedef tn_range<int   , date_trammel           <int   > > tnr_date;

#endif // tn_range_types_hpp

