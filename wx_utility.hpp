// Utilities for use with wxWidgets.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef wx_utility_hpp
#define wx_utility_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <wx/event.h>
#include <wx/string.h>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

class LMI_SO_FWD_DECL calendar_date;
class WXDLLIMPEXP_FWD_CORE wxApp;
class WXDLLIMPEXP_FWD_CORE wxBookCtrlBase;
class WXDLLIMPEXP_FWD_CORE wxDateTime;
class WXDLLIMPEXP_FWD_CORE wxTopLevelWindow;
class WXDLLIMPEXP_FWD_CORE wxWindow;
class jdn_t;

/// Function template c_cast() performs an explicit type conversion
/// (C++98 5.4), which is inherently unsafe. Its sole virtue is that
/// it is easily searched for.

#if defined LMI_GCC
#   pragma GCC diagnostic push
#   if 8 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wcast-function-type"
#   endif // 8 <= __GNUC__
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#endif // defined LMI_GCC
template<typename To, typename From>
To c_cast(From z)
{
    return (To)(z);
}
#if defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined LMI_GCC

/// Encapsulate wx clipboard.
///
/// It has proven all too easy when using wxTheClipboard directly to
/// overlook locking the clipboard or checking return codes, or to be
/// misled by the misnomer IsSupported().

class ClipboardEx
{
  public:
    static std::string GetText();
    static void        SetText(std::string const&);
};

/// Function template Connect() forwards to wxEvtHandler::Connect().
/// It is simpler to write, because it casts its 'handler' argument
/// automatically, and performs some type checking, although the
/// unsafe cast apparently cannot be avoided.

template<typename Return, typename Class, typename Argument>
void Connect
    (wxEvtHandler*   object
    ,wxEventType     event
    ,Return (Class::*handler)(Argument)
    ,int             id = wxID_ANY
    ,wxEvtHandler*   event_sink = nullptr
    )
{
    static_assert(std::is_same_v<void,Return>);
    static_assert(std::is_base_of_v<wxEvtHandler,Class>);
    typedef typename std::remove_reference<Argument>::type argument_type;
    static_assert(std::is_base_of_v<wxEvent,argument_type>);

    if(!object)
        {
        throw std::runtime_error("Connect(): null pointer.");
        }

    typedef void (wxEvtHandler::*t0)(Argument);
    typedef wxObjectEventFunction t1;
    object->Connect
        (id
        ,event
        ,c_cast<t1>(static_cast<t0>(handler))
        ,nullptr
        ,event_sink
        );
}

calendar_date ConvertDateFromWx(wxDateTime const&);
wxDateTime ConvertDateToWx(calendar_date const&);
wxDateTime ConvertDateToWx(jdn_t const&);

bool operator==(calendar_date const& lmi_date, wxDateTime const& wx_date);
bool operator==(wxDateTime const& wx_date, calendar_date const& lmi_date);

// This function is grouped with the functions it tests.

void TestDateConversions();

/// Return a vector comprising the names of all book-control pages, in
/// the order in which the pages appear. The motivation being to
/// facilitate selecting pages by name, an exception is thrown if page
/// names are not unique.

std::vector<std::string> EnumerateBookPageNames(wxBookCtrlBase const&);

/// Return a vector populated with a window's children, grandchildren,
/// and so on, unto the ultimate generation: its entire lineage.
///
/// Motivation: Often it is necessary to iterate over a window's
/// entire lineage, but wx provides direct access only to the first
/// generation--immediate children. Writing a recursive traversal
/// inline wherever wanted is cumbersome, opaque, and error prone.
///
/// Commonly enough, subwindows don't change after a complex dialog
/// has been initialized; then, the result is suitable for caching.

std::vector<wxWindow*> Lineage(wxWindow const*);

/// Return a string comprising a window's name, label, and id.

std::string NameLabelId(wxWindow const*);

wxApp& TheApp();
wxTopLevelWindow& TopWindow();

std::string ValidateAndConvertFilename(wxString const&);

#endif // wx_utility_hpp
