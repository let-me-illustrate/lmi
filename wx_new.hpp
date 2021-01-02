// Allocation functions to work around a wx, mpatrol, and msw problem.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef wx_new_hpp
#define wx_new_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <cstddef>                      // size_t

#if defined LMI_WX_NEW_USE_SO_ATTRIBUTES && !defined UNIT_TESTING_WX_NEW
#
#   if defined LMI_WX_NEW_BUILD_SO && defined LMI_WX_NEW_USE_SO
#       error Both LMI_WX_NEW_BUILD_SO and LMI_WX_NEW_USE_SO defined.
#   endif // defined LMI_WX_NEW_BUILD_SO && defined LMI_WX_NEW_USE_SO
#
#   if defined LMI_WX_NEW_BUILD_SO
#       define LMI_WX_NEW_SO LMI_EXPORT
#   elif defined LMI_WX_NEW_USE_SO
#       define LMI_WX_NEW_SO LMI_IMPORT
#   else  // !defined LMI_WX_NEW_BUILD_SO && !defined LMI_WX_NEW_USE_SO
#       error Neither LMI_WX_NEW_BUILD_SO nor LMI_WX_NEW_USE_SO defined.
#   endif // !defined LMI_WX_NEW_BUILD_SO && !defined LMI_WX_NEW_USE_SO
#
#else  // !defined LMI_WX_NEW_USE_SO_ATTRIBUTES
#   define LMI_WX_NEW_SO
#endif // !defined LMI_WX_NEW_USE_SO_ATTRIBUTES

/// When wx is used as an msw dll, memory is allocated and freed
/// across dll boundaries, and that caused mpatrol to emit spurious
/// diagnostics. This facility was devised to work around that
/// problem. It remains valuable although lmi no longer uses mpatrol,
/// because it indicates memory allocations that must have no matching
/// deallocations within lmi.
///
/// Build these functions as a separate shared library, and use
/// 'new(wx)' to allocate memory that will be freed by wx--e.g., a
/// frame window that's created in an application but (unavoidably)
/// freed by a wx dll.

enum wx_allocator{wx};

LMI_WX_NEW_SO void* operator new  (std::size_t, wx_allocator);
LMI_WX_NEW_SO void* operator new[](std::size_t, wx_allocator);

LMI_WX_NEW_SO void  operator delete  (void*, wx_allocator);
LMI_WX_NEW_SO void  operator delete[](void*, wx_allocator);

#endif // wx_new_hpp
