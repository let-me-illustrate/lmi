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

// $Id: wx_checks.cpp,v 1.1 2005-03-11 03:09:22 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include <wx/defs.h>

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#   error Enable wxUSE_DOC_VIEW_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif

#if !wxUSE_MDI_ARCHITECTURE
#   error Enable wxUSE_MDI_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif

#if !wxUSE_PRINTING_ARCHITECTURE
#   error Enable wxUSE_PRINTING_ARCHITECTURE in wx/include/msw/wx/setup.h .
#endif

