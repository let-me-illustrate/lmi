// Miscellaneous wx workarounds.
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

// $Id: wx_workarounds.hpp,v 1.1 2005-03-11 03:09:22 chicares Exp $

#ifndef wx_workarounds_hpp
#define wx_workarounds_hpp

#include "config.hpp"

#include <wx/string.h>

#include <string>

// WX !! Consider adding these functions to the library.

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

#endif // wx_workarounds_hpp

