// Provide expm1() [C99 7.12.6.3] for toolsets that lack it.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

// The include guard has an 'lmi_' prefix to avoid conflict, because
// 'expm1_h' is an obvious name that libraries might use.

#ifndef lmi_expm1_h
#define lmi_expm1_h

#include "config.hpp"

// This header provides no prototype for exmpm1() because that
// should be the job of <cmath>, once C++ adopts C99 extensions.

#endif // lmi_expm1_h

