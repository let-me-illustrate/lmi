// Overloaded operator new to work around a wx and mpatrol problem.
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

// $Id: wx_new.hpp,v 1.2 2005-03-23 17:36:44 chicares Exp $

// When wx is used as a dll, memory is allocated and freed across dll
// boundaries, and that causes mpatrol to emit spurious diagnostics.
// To work around that problem, use 'new(wx)' (implemented in another
// dll) to allocate memory that will be freed by wx--for instance, a
// frame window that's created in an application but (unavoidably)
// freed by a wx dll. The purpose of this workaround is solely to
// avoid spurious diagnostics; it is not suggested that this is a good
// way to manage memory.

#ifndef wx_new_hpp
#define wx_new_hpp

#include "config.hpp"

#if defined LMI_MSW && defined LMI_WX_NEW_BUILDING_DLL
#   define LMI_WX_NEW_EXPIMP __declspec(dllexport)
#elif defined LMI_MSW && defined LMI_WX_NEW_USING_DLL
#   define LMI_WX_NEW_EXPIMP __declspec(dllimport)
#else // Neither building nor using as dll.
#   define LMI_WX_NEW_EXPIMP
#endif // Neither building nor using as dll.

#include <cstddef> // std::size_t

enum wx_allocator{wx};

LMI_WX_NEW_EXPIMP void* operator new(std::size_t, wx_allocator);
LMI_WX_NEW_EXPIMP void* operator new[](std::size_t, wx_allocator);
LMI_WX_NEW_EXPIMP void operator delete(void*, wx_allocator);
LMI_WX_NEW_EXPIMP void operator delete[](void*, wx_allocator);

#endif // wx_new_hpp

