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

// $Id: wx_utility.cpp,v 1.3 2006-07-10 13:15:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "wx_utility.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"

#include <wx/datetime.h>
#include <wx/window.h>

#include <sstream>

namespace
{
/// This MVC framework uses only dates, and ignores times, but wx
/// combines both concepts in one class, the time portion of which,
/// as used here, should always represent midnight. It is worthwhile
/// to assert such a postcondition for date conversions: formerly,
/// date conversions transformed wx's UTC AJDN to lmi's civil CJDN,
/// and wx timezone problems caused much grief.

void AssertWxTimeIsMidnight(wxDateTime const& wx_date)
{
    wxDateTime::Tm tm = wx_date.GetTm();
    LMI_ASSERT(0 == tm.hour);
    LMI_ASSERT(0 == tm.min);
    LMI_ASSERT(0 == tm.sec);
    LMI_ASSERT(0 == tm.msec);
}
} // Unnamed namespace.

calendar_date ConvertDateFromWx(wxDateTime const& wx_date)
{
    AssertWxTimeIsMidnight(wx_date);
    return calendar_date
        (wx_date.GetYear()
        ,wx_date.GetMonth() + 1
        ,wx_date.GetDay()
        );
}

wxDateTime ConvertDateToWx(calendar_date const& lmi_date)
{
    wxDateTime wx_date
        (lmi_date.day()
        ,static_cast<wxDateTime::Month>(lmi_date.month() - 1)
        ,lmi_date.year()
        );
    AssertWxTimeIsMidnight(wx_date);
    return wx_date;
}

bool operator==(calendar_date const& lmi_date, wxDateTime const& wx_date)
{
    return lmi_date == ConvertDateFromWx(wx_date);
}

bool operator==(wxDateTime const& wx_date, calendar_date const& lmi_date)
{
    return lmi_date == ConvertDateFromWx(wx_date);
}

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

