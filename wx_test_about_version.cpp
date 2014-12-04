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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "wx_test_case.hpp"
#include "version.hpp"

#include <wx/dialog.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

/*
    Test about dialog title.

    4. Confirm version number exists, which can change monthly.
        'Help | About' contains a date in the dialog's title bar,
         which is always identical to this date in 'version.hpp':
         #define LMI_VERSION "20131023T1837Z"
 */
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
