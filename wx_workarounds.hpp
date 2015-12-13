// Miscellaneous wx workarounds.
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

// $Id$

#ifndef wx_workarounds_hpp
#define wx_workarounds_hpp

#include "config.hpp"

#include <wx/defs.h>

// These string-comparison operators became unnecessary in wx-2.5.4,
// and harmful in wx-2.6.0:
//   http://lists.gnu.org/archive/html/lmi/2005-08/msg00002.html

#if !wxCHECK_VERSION(2,5,4) // wx prior to version 2.5.4 .

#   include <wx/string.h>

#   include <string>

inline bool operator==(std::string const& s, wxString const& w)
{
    return s == w.c_str();
}

inline bool operator==(wxString const& w, std::string const& s)
{
    return s == w.c_str();
}

inline bool operator!=(std::string const& s, wxString const& w)
{
    return s != w.c_str();
}

inline bool operator!=(wxString const& w, std::string const& s)
{
    return s != w.c_str();
}

#endif // wx prior to version 2.5.4 .

#endif // wx_workarounds_hpp

