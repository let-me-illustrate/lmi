// Utilities for use with wxWidgets.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "wx_utility.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "wx_new.hpp"

#include <wx/app.h>                     // wxTheApp
#include <wx/bookctrl.h>
// Include <wx/dataobj.h> before <wx/clipbrd.h> as explained here:
//   https://lists.nongnu.org/archive/html/lmi/2010-12/msg00002.html
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/datetime.h>
#include <wx/msgdlg.h>
#include <wx/toplevel.h>
#include <wx/utils.h>                   // wxSafeYield()
#include <wx/window.h>

#include <sstream>

/// Return whatever plain text the clipboard contains, or an empty
/// string if it contains none, replacing "\r\n" with "\n".
///
/// Throw an exception if the clipboard cannot be locked.

std::string ClipboardEx::GetText()
{
    wxClipboardLocker lock;
    if(!lock)
        {
        alarum() << "Unable to lock clipboard." << LMI_FLUSH;
        }

    wxTextDataObject z;
    wxTheClipboard->GetData(z);
    std::string s(z.GetText().ToStdString(wxConvUTF8));

    static std::string const crlf("\r\n");
    static std::string const   lf(  "\n");
    std::string::size_type position = s.find(crlf);
    while(std::string::npos != position)
        {
        s.replace(position, crlf.length(), lf);
        position = s.find(crlf, 1 + position);
        }

    return s;
}

/// Place plain text on the clipboard.
///
/// Throw an exception if the clipboard cannot be locked.

void ClipboardEx::SetText(std::string const& s)
{
    wxClipboardLocker lock;
    if(!lock)
        {
        alarum() << "Unable to lock clipboard." << LMI_FLUSH;
        }

    wxTextDataObject* TextDataObject = new(wx) wxTextDataObject(s);
    wxTheClipboard->SetData(TextDataObject);
}

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
    wxDateTime::Tm z = wx_date.GetTm();
    LMI_ASSERT(0 == z.hour);
    LMI_ASSERT(0 == z.min);
    LMI_ASSERT(0 == z.sec);
    LMI_ASSERT(0 == z.msec);
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
        (bourn_cast <wxDateTime::wxDateTime_t>(lmi_date.day())
        ,static_cast<wxDateTime::Month>       (lmi_date.month() - 1)
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

/// Test conversions between classes wxDateTime and calendar_date.
///
/// This function tests ConvertDateFromWx() and ConvertDateToWx().
/// It is invoked via a menuitem: it's not a standalone command-line
/// program like most other unit tests because it depends on wx.

void TestDateConversions()
{
    calendar_date const z {jdn_t(calendar_date::min_verified_jdn)};
    int const low  = z               .julian_day_number();
    int const high = last_yyyy_date().julian_day_number();
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
        calendar_date const lmi_date0 {jdn_t(j)};
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
            alarum()
                << "Date conversion failed:\n"
                << lmi_date0.str() << " original\n"
                << lmi_date1.str() << " converted\n"
                << LMI_FLUSH
                ;
            }

        std::string const lmi_str(lmi_date0.str());
        std::string const wx_str
            (ConvertDateToWx(lmi_date0)
            .FormatISODate()
            .ToStdString(wxConvUTF8)
            );
        if(lmi_str != wx_str)
            {
            alarum()
                << "ISO8601 representations differ:\n"
                << lmi_str << " lmi\n"
                << wx_str  << " wx\n"
                << LMI_FLUSH
                ;
            }

        int const step = 10000;
        if(0 == j % step)
            {
            status() << (high - j) / step << std::flush;
            wxSafeYield();
            }
        }
    status() << "Date-conversion test succeeded." << std::flush;
}

std::vector<std::string> EnumerateBookPageNames(wxBookCtrlBase const& book)
{
    std::vector<std::string> z;
    for(int j = 0; j < bourn_cast<int>(book.GetPageCount()); ++j)
        {
        std::string name(book.GetPageText(j).ToStdString(wxConvUTF8));
        LMI_ASSERT(!contains(z, name));
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
    for(auto const& c : w->GetChildren())
        {
        LMI_ASSERT(nullptr != c);
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

/// Safe accessor for wxTheApp: throws if null.

wxApp& TheApp()
{
    if(!wxTheApp)
        {
        safely_show_message("Application object unavailable.");
        throw 0;
        }
    return *wxTheApp;
}

/// Safe cover function for wxApp::GetTopWindow(): throws if null.
///
/// If GetTopWindow() returns nullptr, then wx is probably starting up
/// or shutting down; therefore, diagnostics are displayed through a
/// mechanism that should work even in such circumstances.

wxTopLevelWindow& TopWindow()
{
    wxWindow* w = TheApp().GetTopWindow();
    if(!w)
        {
        safely_show_message("No top window found.");
        throw 0;
        }
    wxTopLevelWindow* t = dynamic_cast<wxTopLevelWindow*>(w);
    if(!t)
        {
        safely_show_message("Top window is not a wxTopLevelWindow.");
        throw 0;
        }
    return *t;
}

/// Convert a filename to an NTBS std::string, throwing upon failure.
///
/// An operating system might hand an NTMBS or an NTWCS to wx.
/// When wx hands that in turn to lmi in a context where a
/// std::basic_fstream is wanted, data loss may occur because
/// std::basic_fstream requires an NTBS argument--see:
///   https://lists.nongnu.org/archive/html/lmi/2010-05/msg00023.html
/// This function throws if that problem would occur.

std::string ValidateAndConvertFilename(wxString const& w)
{
    if(w.IsEmpty())
        {
        alarum() << "Filename is empty." << LMI_FLUSH;
        }
    std::string s(w.mb_str());
    if(s.empty())
        {
        wxString x =
              "Filename '"
            + w
            + "' contains multi-byte characters, but only"
            + " single-byte characters are supported."
            ;
        wxMessageBox(x, "Problematic filename");
        throw hobsons_choice_exception();
        }
    return s;
}
