// Validate assumptions about how the wxWindows library was built.
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

// $Id: wx_checks.cpp,v 1.2 2005-03-18 00:17:21 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

// Require certain optional wx components.

#include <wx/defs.h>

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#   error Enable wxUSE_DOC_VIEW_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif // !wxUSE_DOC_VIEW_ARCHITECTURE

#if !wxUSE_MDI_ARCHITECTURE
#   error Enable wxUSE_MDI_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif // !wxUSE_MDI_ARCHITECTURE

#if !wxUSE_PRINTING_ARCHITECTURE
#   error Enable wxUSE_PRINTING_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif // !wxUSE_PRINTING_ARCHITECTURE

// Ensure that certain inappropriate options aren't used.

#if defined __GNUC__
// Incompatible with gcc.
#   if wxUSE_ON_FATAL_EXCEPTION
#       error Disable wxUSE_ON_FATAL_EXCEPTION in wx/include/msw/wx/setup.h .
#   endif // wxUSE_ON_FATAL_EXCEPTION

// Incompatible with gcc.
#   if wxUSE_STACKWALKER
#       error Disable wxUSE_STACKWALKER in wx/include/msw/wx/setup.h .
#   endif // wxUSE_STACKWALKER
#endif // __GNUC__

// This application is single threaded, and wasn't designed to be
// thread safe. It might work perfectly well if compiled with thread
// support enabled (but not used), but that has never been tested, and
// presumably would require changes to the lmi makefiles.
//
#if wxUSE_THREADS
#   error Disable wxUSE_THREADS in wx/include/msw/wx/setup.h .
#endif // wxUSE_THREADS

