// Make sure the default input file can be opened, modified, and saved.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "path.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_statusbar.hpp"

#include <wx/checkbox.h>
#include <wx/datectrl.h>
#include <wx/radiobox.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

/// Make sure the default input file can be opened, modified, and saved.
///
/// Run this test only if the '--distribution' option is given.
///
/// Load the default input file, using its special command.
///
/// Change its "DateOfBirth" option. This particular option is used because it
/// is available for any life insurance product as the date of birth is a field
/// of such central importance.
///
/// Save the changed file; make sure the appropriate message appears
/// on the status bar. Make sure the saved file exists in its
/// configured directory.

LMI_WX_TEST_CASE(default_update)
{
    skip_if_not_distribution();

    wxUIActionSimulator ui;

    ui.Char('t', wxMOD_CONTROL); // "File|Default"

    struct change_dob_in_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            wxWindow* const dob_window = wx_test_focus_controller_child
                (*dialog
                ,"DateOfBirth"
                );

            // We need to ensure that the "Date Of Birth" field is enabled
            // by triggering the value of "Use Date Of Birth" if necessary.
            wxWindow* const usedob_window = wx_test_focus_controller_child
                (*dialog
                ,"UseDOB"
                );

            if(!dob_window->IsEnabled())
                {
                ToggleUseDOB(usedob_window);
                }

            // Entering the target date into a wxDatePickerCtrl using
            // wxUIActionSimulator is too difficult: different sequences of
            // keys are required depending on the graphical toolkit used and
            // also depending on the current locale, so just cheat and put the
            // date directly into the control.
            wxDatePickerCtrl* const dob = dynamic_cast<wxDatePickerCtrl*>
                (dob_window
                );
            LMI_ASSERT_WITH_MSG
                (dob
                ,"\"DateOfBirth\" field is expected to be a wxDatePickerCtrl"
                );

            dob->SetValue(wxDateTime(13, wxDateTime::Jan, 1956));
            wxYield();

            // We also need to modify some field interactively to make the
            // dialog "notice" that something has changed and even making
            // wxDatePickerCtrl dirty is difficult using wxUIActionSimulator as
            // it has very different keyboard interfaces under MSW and GTK, so
            // reuse the "UseDOB" check or radio box: we don't actually change
            // anything by toggling it twice, but doing this updates the value
            // of the "DateOfBirth" field as a side effect.
            ToggleUseDOB(usedob_window);
            ToggleUseDOB(usedob_window);

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "defaults dialog";
            }

        // Helper function toggling the value of the "UseDOB" field which can be
        // represented by either a check box or a two element radio box
        // depending on the skin used.
        static void ToggleUseDOB(wxWindow* usedob_window)
            {
            wxUIActionSimulator ui;

            if(dynamic_cast<wxRadioBox*>(usedob_window))
                {
                // Just selecting the other button is sufficient to toggle
                // the value of a 2 element radio box under MSW, but under
                // GTK we also have to explicitly check it by pressing
                // Space or Enter and as it doesn't do anything under MSW, we
                // just do it unconditionally to avoid conditional compilation.
                ui.Char(WXK_DOWN);
                ui.Char(WXK_SPACE);
                }
            else if(dynamic_cast<wxCheckBox*>(usedob_window))
                {
                ui.Char(WXK_SPACE);
                }
            else
                {
                throw std::runtime_error("\"UseDOB\" field has unknown type");
                }

            wxYield();
            }
    };

    wxTEST_DIALOG
        (wxYield()
        ,change_dob_in_defaults_dialog()
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
