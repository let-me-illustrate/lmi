// Validate assumptions about how the wxWindows library was built.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "wx_checks.hpp"

#include <wx/defs.h>

#if !wxCHECK_VERSION(2,5,4) // wx prior to version 2.5.4 .
#   error Outdated library: wx-2.5.4 or greater is required.
#endif // wx prior to version 2.5.4 .

#if wxCHECK_VERSION(3,0,0)
#   error Remove the "1252" html exception in 'test_coding_rules.cpp'.
#endif // wxCHECK_VERSION(3,0,0)

// Require certain optional wx components.

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#   error Enable wxUSE_DOC_VIEW_ARCHITECTURE in wx setup.
#endif // !wxUSE_DOC_VIEW_ARCHITECTURE

#if !wxUSE_MDI_ARCHITECTURE
#   error Enable wxUSE_MDI_ARCHITECTURE in wx setup.
#endif // !wxUSE_MDI_ARCHITECTURE

#if !wxUSE_PRINTING_ARCHITECTURE
#   error Enable wxUSE_PRINTING_ARCHITECTURE in wx setup.
#endif // !wxUSE_PRINTING_ARCHITECTURE

// Disable all the checks below which, unlike the ones above, do not prevent
// LMI from being compiled or from working but simply check that wxWidgets
// build used by LMI is "optimized" for it. Disabling them allows us to use
// standard, not modified, version of wxWidgets which would be impossible
// otherwise.
#if 0

// Ensure that certain inappropriate options aren't used.

// License not obviously compatible with GPL.
#if wxUSE_APPLE_IEEE
#   error Disable wxUSE_APPLE_IEEE in wx setup.
#endif // wxUSE_APPLE_IEEE

// Graphics format with potential patent issues.
#if wxUSE_LIBJPEG
#   error Disable wxUSE_LIBJPEG in wx setup.
#endif // wxUSE_LIBJPEG

// Graphics format with potential patent issues.
#if wxUSE_LIBTIFF
#   error Disable wxUSE_LIBTIFF in wx setup.
#endif // wxUSE_LIBTIFF

// Patented: incompatible with software freedom.
#if wxUSE_GIF
#   error Disable wxUSE_GIF in wx setup.
#endif // wxUSE_GIF

#if defined __GNUC__ && defined LMI_MSW
// Not yet implemented for gcc on the msw platform.
#   if wxUSE_ON_FATAL_EXCEPTION
#       error Disable wxUSE_ON_FATAL_EXCEPTION in wx setup.
#   endif // wxUSE_ON_FATAL_EXCEPTION

// Not yet implemented for gcc on the msw platform.
#   if wxUSE_STACKWALKER
#       error Disable wxUSE_STACKWALKER in wx setup.
#   endif // wxUSE_STACKWALKER
#endif // defined __GNUC__ && defined LMI_MSW

// This application is single threaded, and wasn't designed to be
// thread safe. It might work perfectly well if compiled with thread
// support enabled (but not used), but that has never been tested, and
// presumably would require changes to the lmi makefiles.
//
#if wxUSE_THREADS
#   error Disable wxUSE_THREADS in wx setup.
#endif // wxUSE_THREADS

#endif // 0
