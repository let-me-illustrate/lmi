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

// $Id: wx_new.cpp,v 1.1 2005-02-23 12:37:20 chicares Exp $

#include "pchfile.hpp"

#ifdef __BORLANDC__
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

