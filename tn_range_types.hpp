// Declaration of specific tn_range types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef tn_range_types_hpp
#define tn_range_types_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"             // Template class forward declaration.
#include "tn_range_type_trammels.hpp"   // Trammels for specific tn_range types.

// SOMEDAY !! Move these two to a 'test' file.

typedef tn_range<int          , percentage_trammel     <int          >> r_int_percentage;
typedef tn_range<double       , percentage_trammel     <double       >> r_double_percentage;

typedef tn_range<double       , unrestricted_trammel   <double       >> tnr_unrestricted_double;
typedef tn_range<double       , proportion_trammel     <double       >> tnr_proportion;
typedef tn_range<double       , nonnegative_trammel    <double       >> tnr_nonnegative_double;
typedef tn_range<int          , nonnegative_trammel    <int          >> tnr_nonnegative_integer;
typedef tn_range<int          , age_trammel            <int          >> tnr_age;
typedef tn_range<int          , duration_trammel       <int          >> tnr_duration;
typedef tn_range<int          , month_trammel          <int          >> tnr_month;
typedef tn_range<double       , corridor_factor_trammel<double       >> tnr_corridor_factor;
typedef tn_range<calendar_date, date_trammel           <calendar_date>> tnr_date;

#endif // tn_range_types_hpp
