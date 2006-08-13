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

// $Id: wx_utility.cpp,v 1.5 2006-08-13 11:22:09 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "wx_utility.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"

#include <wx/bookctrl.h>
#include <wx/datetime.h>
#include <wx/window.h>

#include <algorithm> // std::find()
#include <cstddef>   // std::size_t
#include <sstream>

namespace
{
/// The MVC framework uses only dates, and ignores times, whereas wx
/// combines both concepts in one class--the time portion of which,
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

wxDateTime ConvertDateToWx(jdn_t const& j)
{
    return ConvertDateToWx(calendar_date(j));
}

bool operator==(calendar_date const& lmi_date, wxDateTime const& wx_date)
{
    return lmi_date == ConvertDateFromWx(wx_date);
}

bool operator==(wxDateTime const& wx_date, calendar_date const& lmi_date)
{
    return lmi_date == ConvertDateFromWx(wx_date);
}

void TestDateConversions()
{
    const int low  = gregorian_epoch().julian_day_number();
    const int high = last_yyyy_date ().julian_day_number();
    status()
        << "Testing conversion of all dates in the range ["
        << low
        << ", "
        << high
        << "]."
        << std::flush
        ;
    for(int j = low; j <= high; ++j)
        {
        // Double parentheses circumvent the most vexing parse.
        calendar_date const lmi_date0((jdn_t(j)));
        calendar_date const lmi_date1 =
            ConvertDateFromWx
                (ConvertDateToWx
                    (ConvertDateFromWx
                        (ConvertDateToWx
                            (lmi_date0
                            )
                        )
                    )
                );
        if(lmi_date1 != lmi_date0)
            {
            fatal_error()
                << "Date conversion failed:\n"
                << "  original  date: " << lmi_date0.str() << '\n'
                << "  converted date: " << lmi_date1.str() << '\n'
                << LMI_FLUSH
                ;
            }
        }
    status() << "Date-conversion test succeeded." << std::flush;
}

std::vector<std::string> EnumerateBookPageNames(wxBookCtrlBase const& book)
{
    std::vector<std::string> z;
    for(std::size_t j = 0; j < book.GetPageCount(); ++j)
        {
        std::string name(book.GetPageText(j));
        LMI_ASSERT(z.end() == std::find(z.begin(), z.end(), name));
        z.push_back(name);
        }
    return z;
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

std::string NameLabelId(wxWindow const* w)
{
    if(!w)
        {
        return "null window pointer";
        }

    std::ostringstream oss;
    oss
        << "window id " << w->GetId   ()
        << "; label '"  << w->GetLabel() << "'"
        << "; name '"   << w->GetName () << "'"
        ;
    return oss.str();
}

