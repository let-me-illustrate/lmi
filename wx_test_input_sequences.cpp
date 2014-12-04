// Test running the input sequences case.
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
#include "wx_test_case.hpp"

#include <wx/filename.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

/*
    Test that the input sequences census can be run.

    This implements the following item of the testing specification:

        10. Confirm Help file input sequences remain valid.

          A. File | Open | 'InputSequences.cns'
             Census | Run case
             Expected result:
               Case runs without any warnings or errors.
 */
LMI_WX_TEST_CASE(input_sequences)
{
    // Construct the path of the file to open, it's supposed to be in the same
    // directory as the default input filename.
    wxFileName fn(configurable_settings::instance().default_input_filename());
    fn.SetFullName("InputSequences.cns");

    wxUIActionSimulator ui;

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
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
