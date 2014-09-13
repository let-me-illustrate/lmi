// Test validation of input ranges in a census.
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

#include <stdexcept>

LMI_WX_TEST_CASE(input_validation)
{
    wxUIActionSimulator ui;
    ui.Char('o', wxMOD_CONTROL);    // "File|Open"

    wxFileName fn(configurable_settings::instance().default_input_filename());
    fn.SetFullName("CoiMultiplier.cns");
    wxTEST_DIALOG
        (wxYield()
         ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
        );

    ui.Char('r', wxMOD_CONTROL | wxMOD_SHIFT);  // "Census|Run case"

    // Test that the expected exception is generated.
    bool error_detected = false;
    try
        {
        wxYield();
        }
    catch(std::runtime_error& e)
        {
        error_detected = true;

        // The error message contains a line of the form "[file %s, line %d]"
        // at the end which we want to ignore, as the line number and possibly
        // the file name can change and are irrelevant to this check anyhow, so
        // find this line presence and ignore it in comparison.
        std::string const error_message = e.what();
        std::string::size_type loc_pos = error_message.find("\n[file");
        LMI_ASSERT(loc_pos != std::string::npos);

        LMI_ASSERT_EQUAL
            (error_message.substr(0, loc_pos),
             "Input validation problems for '':\n"
             "COI multiplier entered is '0', but it must contain at least one number other than zero.\n"
            );
        }

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    LMI_ASSERT(error_detected);
}
