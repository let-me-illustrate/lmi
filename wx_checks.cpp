// Validate assumptions about how the wxWindows library was built.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "wx_checks.hpp"

#include <wx/defs.h>

#if !wxCHECK_VERSION(3,1,0) // wx prior to version 3.1.0 .
#   error Outdated library: wx-3.1.0 or greater is required.
#endif // wx prior to version 3.1.0 .

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

#if !wxUSE_LIBLZMA
#   error Enable wxUSE_LIBLZMA in wx setup.
#endif // !wxUSE_LIBLZMA

// Ensure that certain inappropriate options aren't used.

// License not obviously compatible with GPL.
#if wxUSE_APPLE_IEEE
#   error Disable wxUSE_APPLE_IEEE in wx setup.
#endif // wxUSE_APPLE_IEEE

// The patents have expired, but the stench lingers:
//   https://www.gnu.org/philosophy/gif.html
#if wxUSE_GIF
#   error Disable wxUSE_GIF in wx setup.
#endif // wxUSE_GIF

