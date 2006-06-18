// Utilities for use with wxWidgets.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: wx_utility.cpp,v 1.1 2006-06-18 02:50:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "wx_utility.hpp"

#include "alert.hpp"

#include <wx/window.h>

#include <sstream>

std::string Describe(wxWindow const* w)
{
    std::ostringstream oss;
    oss
        << "id "       << w->GetId   ()
        << "; label '" << w->GetLabel() << "'"
        << "; name '"  << w->GetName () << "'"
        ;
    return oss.str();
}

namespace
{
/// Replace contents of vector 'v' with lineage of window 'z'.
///
/// This helper function lets Lineage() present a simple interface.

void EnumerateLineage
    (wxWindow const*         w
    ,std::vector<wxWindow*>& v
    )
{
    wxWindowList const& wl = w->GetChildren();
    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
        {
        wxWindow* c = *i;
        LMI_ASSERT(0 != c);
        v.push_back(c);
        EnumerateLineage(c, v);
        }
}
} // Unnamed namespace.

std::vector<wxWindow*> Lineage(wxWindow const* w)
{
    std::vector<wxWindow*> v;
    EnumerateLineage(w, v);
    return v;
}

