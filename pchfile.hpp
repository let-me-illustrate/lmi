// Precompiled header support.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: pchfile.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

// Precompiled header file.
//
// Include at the beginning of every .cpp file (and nowhere else).
// Make sure your code compiles with '-DIGNORE_PCH' ! See style.txt .
// Try adding 'CPPFLAGS=-DLMI_IGNORE_PCH' to the make command line.

#ifndef pchfile_hpp
#define pchfile_hpp

#include "config.hpp"

#   ifdef LMI_COMPILER_USES_PCH
#       ifndef LMI_IGNORE_PCH

#           ifdef LMI_WX
#               include <wx/wxprec.h>
#               if !defined wxUSE_DOC_VIEW_ARCHITECTURE || !wxUSE_DOC_VIEW_ARCHITECTURE
#                   error Enable wxUSE_DOC_VIEW_ARCHITECTURE in <wx/msw/setup.h>.
#               endif // !defined wxUSE_DOC_VIEW_ARCHITECTURE || !wxUSE_DOC_VIEW_ARCHITECTURE
#               if !defined wxUSE_MDI_ARCHITECTURE || !wxUSE_MDI_ARCHITECTURE
#                   error Enable wxUSE_MDI_ARCHITECTURE in <wx/msw/setup.h>.
#               endif // !definedwxUSE_MDI_ARCHITECTURE || !wxUSE_MDI_ARCHITECTURE
#           endif // LMI_WX

#           include <cmath>
#           include <cstdio>
#           include <cstdlib>
#           include <cstring>
#           include <string>

#       endif // LMI_IGNORE_PCH
#   endif // LMI_COMPILER_USES_PCH

#endif // pchfile_hpp

