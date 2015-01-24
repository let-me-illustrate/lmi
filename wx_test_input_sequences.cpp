// Validate a variety of input sequences in the GUI input dialog.
//
// Copyright (C) 2014, 2015 Gregory W. Chicares.
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
#include "wx_test_case.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

// ERASE THIS BLOCK COMMENT WHEN IMPLEMENTATION COMPLETE. The block
// comment below changes the original specification, and does not
// yet describe the present code. Desired changes:
//  - Hard code the sequences; get rid of 'InputSequences.cns'.
//  - Paste each test sequence into a temporary input dialog.
//  - Validate with ellipsis button and then with OK.

/// Validate a variety of input sequences in the GUI input dialog.
///
/// Test a broad variety of input sequences. For now, use the set in
/// the user manual:
///   file:///C:/lmi/src/web/lmi/sequence_input.html
/// but hard code them here--later they might differ, e.g. if we
/// decide to add extra tests here.
///
/// First, create a temporary '.ill' document:
///   File | New | Illustration
/// Then paste each input sequence into the appropriate field and test
/// it thus:
///  - Click the ellipsis button; press OK to close its dialog.
///  - Click OK to run the illustration. This step is tested because
///    it triggers downstream validation.
/// Reopen the tabbed dialog for each subsequent test. When done,
/// close the illustration without saving it.

LMI_WX_TEST_CASE(input_sequences)
{
    wxUIActionSimulator ui;

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(get_test_file_path_for("InputSequences.cns"))
        );

    ui.Char('r', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Run case"
    wxYield();

    // Close the illustration opened by "Run case".
    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    // And the census itself as well.
    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();
}

