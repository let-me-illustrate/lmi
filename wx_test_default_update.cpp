// Make sure the default input file can be opened, modified, and saved.
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
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "wx_test_case.hpp"
#include "wx_test_statusbar.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <boost/filesystem/operations.hpp>

/// Make sure the default input file can be opened, modified, and saved.
///
/// Load the default input file, using its special command.
///
/// Change its "MEC avoidance" option. This particular option is used
/// because it is available for almost any life insurance product.
/// Save the changed file; make sure the appropriate message appears
/// on the status bar. Make sure the saved file exists in its
/// configured directory.

LMI_WX_TEST_CASE(default_update)
{
    wxUIActionSimulator ui;

    // Change the "MEC avoidance" option in the first page of the defaults
    // dialog.
    ui.Char('t', wxMOD_CONTROL); // "File|Default"

    struct change_mec_avoidance_in_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        virtual int OnInvoked(MvcController* dialog) const
            {
            dialog->Show();
            wxYield();

            wxUIActionSimulator ui;

            // Go to the first page: as the dialog remembers its last opened
            // page, it might not open on it.
            ui.Char(WXK_HOME);
            wxYield();

            // Select the first button of the "MEC avoidance" radio box.
            ui.Char(WXK_TAB);
            ui.Char(WXK_TAB);
            wxYield();

            // Change its value: it doesn't matter which button is selected,
            // pressing the down arrow will always toggle the selection in a
            // radio box with two buttons.
            ui.Char(WXK_DOWN);
            wxYield();

            return wxID_OK;
            }
    };

    wxTEST_DIALOG
        (wxYield()
        ,change_mec_avoidance_in_defaults_dialog()
        );

    // Save the default document.
    ui.Char('s', wxMOD_CONTROL); // "File|Save"
    wxYield();

    // Verify that the expected message about saving it was given.
    std::string const
        filename = configurable_settings::instance().default_input_filename();

    LMI_ASSERT_EQUAL
        (get_main_window_statusbar_text()
        ,wxString::Format("Saved '%s'.", filename)
        );

    // Close the document now that it's not needed any more.
    ui.Char('l', wxMOD_CONTROL); // "File|Close"
    wxYield();

    // Finally also check that the file actually exists.
    LMI_ASSERT(fs::exists(filename));
}
