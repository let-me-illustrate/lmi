// Version number test case for the GUI test suite.
//
// Copyright (C) 2014 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "wx_test_case.hpp"
#include "version.hpp"

#include <wx/dialog.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

/// Validate version string (timestamp) from "About" dialog title.
///
/// Test that the version string matches the timestamp specified in
/// 'version.hpp'. That's not a really useful test because it can
/// hardly fail, but it's so inexpensive that there's no reason to
/// delete it.
///
/// Write the version string to stdout (along with other test output)
/// for comparison with previously-saved results.
///
/// Find the last copyright year listed in the dialog's client area,
/// and compare it to the year in the version string, and also to the
/// current calendar year: it should match both. (This means that
/// running a 2014 GUI test in 2015 will fail; that's okay.) Monthly
/// releases sometimes update only the 'expiry' file, but reuse the
/// last month's binaries with an unchanged version string; therefore,
/// only the version string's year is compared to the current date,
/// whereas month, day, and time are not.
///
/// Press the pushbutton to read the license, and check that the
/// license's dialog box is scrollable--to guard against this problem:
///   http://lists.nongnu.org/archive/html/lmi/2010-01/msg00001.html

LMI_WX_TEST_CASE(about_dialog_version)
{
    struct expect_about_dialog : public wxExpectModalBase<wxDialog>
    {
        virtual int OnInvoked(wxDialog* d) const
            {
            LMI_ASSERT(0 != d);
            LMI_ASSERT(d->GetTitle().EndsWith(LMI_VERSION));
            return wxID_OK;
            }
    };

    wxUIActionSimulator z;
    z.KeyDown('h', wxMOD_ALT);
    z.KeyUp  ('h', wxMOD_ALT);
    z.KeyDown('a'           );
    z.KeyUp  ('a'           );
    wxTEST_DIALOG
        (wxYield()
        ,expect_about_dialog()
        );
}

