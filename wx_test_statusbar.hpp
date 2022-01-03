// Helpers for checking status bar contents in unattended GUI tests.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef wx_test_statusbar_hpp
#define wx_test_statusbar_hpp

#include "config.hpp"

#include "assert_lmi.hpp"

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/statusbr.h>

/// Return the status bar of the main window throwing an exception if anything
/// goes wrong.

inline wxStatusBar& get_main_window_statusbar()
{
    wxWindow* const MainWin = wxTheApp->GetTopWindow();
    LMI_ASSERT(MainWin);

    wxFrame* const frame = wxDynamicCast(MainWin, wxFrame);
    LMI_ASSERT(frame);

    wxStatusBar* const status = frame->GetStatusBar();
    LMI_ASSERT(status);

    return *status;
}

/// Return the contents of the status of the main window throwing an exception
/// if it can't be retrieved.

inline wxString get_main_window_statusbar_text()
{
    return get_main_window_statusbar().GetStatusText();
}

#endif // wx_test_statusbar_hpp
