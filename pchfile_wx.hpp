// Precompiled header file.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// Always include this header first in every '.cpp' file that uses wxWidgets,
// before anything else except comments and whitespace. Never include it in
// any header file.

#ifndef pchfile_wx_hpp
#define pchfile_wx_hpp

#include "pchfile.hpp"

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH
#   include <wx/wx.h>

#   include "wx_new.hpp"
#   include "wx_utility.hpp"
#   include "wx_workarounds.hpp"

#   include "product_editor.hpp"
#   include "view_ex.hpp"
#endif

#endif // pchfile_wx_hpp

