// Instantiation of specific tn_range types.
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

#include "pchfile.hpp"

#include "tn_range.tpp"                 // Template class implementation.
#include "tn_range_type_trammels.hpp"   // Trammels for specific tn_range types.

// This header need not be included:
//   #include "tn_range_types.hpp"
// because it merely declares typedefs that aren't needed here.

// SOMEDAY !! Move these two to a 'test' file.

template class tn_range<int          , percentage_trammel     <int          >>;
template class tn_range<double       , percentage_trammel     <double       >>;

template class tn_range<double       , unrestricted_trammel   <double       >>;
template class tn_range<double       , proportion_trammel     <double       >>;
template class tn_range<double       , nonnegative_trammel    <double       >>;
template class tn_range<int          , nonnegative_trammel    <int          >>;
template class tn_range<int          , age_trammel            <int          >>;
template class tn_range<int          , duration_trammel       <int          >>;
template class tn_range<int          , month_trammel          <int          >>;
template class tn_range<double       , corridor_factor_trammel<double       >>;
template class tn_range<calendar_date, date_trammel           <calendar_date>>;

// Explicitly instantiate trammel_base for every type actually used.
// Otherwise, its specialized members (minimum_minimorum(), e.g.)
// cause linker problems with gcc-5. See:
//   https://lists.nongnu.org/archive/html/lmi/2015-09/msg00000.html

template class trammel_base<calendar_date>;
template class trammel_base<double       >;
template class trammel_base<int          >;
