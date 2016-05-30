// Allocation functions to work around a wx, mpatrol, and msw problem.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef wx_new_hpp
#define wx_new_hpp

#include "config.hpp"

#include <cstddef>                      // std::size_t

// TODO ?? CALCULATION_SUMMARY Revise in light of this message:
//   http://lists.nongnu.org/archive/html/lmi/2006-10/msg00024.html

#if defined HAVE_CONFIG_H
// For msw, rely on the 'auto-import' kludge favored by autotools.
#   define LMI_WX_NEW_SO
#elif defined LMI_MSW
#   if defined LMI_WX_NEW_BUILD_SO
#       define LMI_WX_NEW_SO __declspec(dllexport)
#   elif defined LMI_WX_NEW_USE_SO
#       define LMI_WX_NEW_SO __declspec(dllimport)
#   else  // !defined LMI_WX_NEW_BUILD_SO && !defined LMI_WX_NEW_USE_SO
#       error Either LMI_WX_NEW_BUILD_SO or LMI_WX_NEW_USE_SO must be defined.
#   endif // !defined LMI_WX_NEW_BUILD_SO && !defined LMI_WX_NEW_USE_SO
#else  // !defined HAVE_CONFIG_H && !defined LMI_MSW
#   error Unknown platform and build system.
#endif // !defined HAVE_CONFIG_H && !defined LMI_MSW

/// When wx is used as an msw dll, memory is allocated and freed
/// across dll boundaries, and that causes mpatrol to emit spurious
/// diagnostics.
///
/// To work around this problem, build these functions as a separate
/// dll, and use 'new(wx)' to allocate memory that will be freed by
/// wx--for instance, a frame window that's created in an application
/// but (unavoidably) freed by a wx dll. The sole purpose of this
/// workaround is to avoid spurious diagnostics; it is not suggested
/// that this is a good way to manage memory.
///
/// It is assumed (but not known) that no such problem occurs on other
/// platforms. Therefore, the macros above never use ELF visibility
/// attributes.

enum wx_allocator{wx};

LMI_WX_NEW_SO void* operator new  (std::size_t, wx_allocator);
LMI_WX_NEW_SO void* operator new[](std::size_t, wx_allocator);

LMI_WX_NEW_SO void  operator delete  (void*, wx_allocator);
LMI_WX_NEW_SO void  operator delete[](void*, wx_allocator);

#endif // wx_new_hpp

