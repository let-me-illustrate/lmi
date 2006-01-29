// Allocation functions to work around a wx, mpatrol, and msw problem.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: wx_new.cpp,v 1.4 2006-01-29 13:52:01 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "wx_new.hpp"

void* operator new(std::size_t bytes, wx_allocator)
{
    return operator new(bytes);
}

void* operator new[](std::size_t bytes, wx_allocator)
{
    return operator new(bytes);
}

void operator delete(void* pointer, wx_allocator)
{
    operator delete(pointer);
}

void operator delete[](void* pointer, wx_allocator)
{
    operator delete(pointer);
}

