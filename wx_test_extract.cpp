// Test extract file formats.
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

#include <wx/filename.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <stdexcept>

/*
    Test extract file formats.

    This implements the following item of the testing specification:

        16. Open and run a variety of extract file formats; Close
             files and the program.

          A. File | Open | 'ExtractV6.ill' | press 'OK' twice [to
              dismiss input validation message boxes] | press 'OK' to
              run illustration
             File | Close | No [do not save changes]

          B. File | Open | 'ExtractV5.cns' | press 'OK' thrice [to
              dismiss input validation message boxes]
             Census | Run case
             File | Close
             File | Exit

    The only difference with the specification is that "File | Exit" is not tested
    as the program currently needs to remain running in order to display the test
    results to the user.
 */
LMI_WX_TEST_CASE(extract)
{
    wxUIActionSimulator ui;
    ui.Char('o', wxMOD_CONTROL);    // "File|Open"

    wxFileName fn(configurable_settings::instance().default_input_filename());
    fn.SetFullName("ExtractV6.ill");
    wxTEST_DIALOG
        (wxYield()
         ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
         ,wxExpectModal<wxMessageDialog>(wxOK)  // Dismiss first warning.
         ,wxExpectModal<wxMessageDialog>(wxOK)  // And the second one.
         ,wxExpectDismissableModal<MvcController>(wxID_OK)  // Accept defaults.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxMessageDialog>(wxNO)   // Don't save changes.
        );

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"

    fn.SetFullName("ExtractV5.cns");
    wxTEST_DIALOG
        (wxYield()
         ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
         ,wxExpectModal<wxMessageDialog>(wxOK)  // Dismiss first warning.
         ,wxExpectModal<wxMessageDialog>(wxOK)  // And the second one.
         ,wxExpectModal<wxMessageDialog>(wxOK)  // And the third one.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();
}
