// Test case for creating new files of all types and opening them.
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


// Helper of test_new_file_and_save() which tests creating a new file of
// the type corresponding to the key argument, used to select this type in
// the "New" popup menu.
//
// The last argument indicates whether a dialog is shown when creating a
// new file of this type (e.g. true for illustrations, false for census).
// It affects this function behaviour in two ways: first, it needs to be
// ready for this dialog appearing and, second, "File|Save" menu command is
// disabled for the files created in this way and "File|Save as" needs to
// be used instead.
void do_test_new_file_and_save(int key, wxString const& file, bool uses_dialog)
{
    struct expect_config_dialog : public wxExpectModalBase<wxDialog>
    {
        virtual int OnInvoked(wxDialog* d) const
            {
            return wxID_OK;
            }
    };

    LMI_ASSERT(!wxFileExists(file));

    wxUIActionSimulator z;
    z.Char('n', wxMOD_CONTROL); // new file
    z.Char(key               ); // choose document type
    if (uses_dialog)
        {
        wxTEST_DIALOG
            (wxYield()
            ,expect_config_dialog()
            );
        }
    wxYield();

    z.Char(uses_dialog ? 'a' : 's', wxMOD_CONTROL); // save or save as
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(file)
        );
    wxYield();

    LMI_ASSERT(wxFileExists(file));
    wxRemoveFile(file);

    z.Char('l', wxMOD_CONTROL); // close document
    wxYield();
}

LMI_WX_TEST_CASE(new_file_and_save)
{
    do_test_new_file_and_save('c', "testfile.cns"     , false);
    do_test_new_file_and_save('i', "testfile.ill"     , true);
    do_test_new_file_and_save('d', "testfile.database", false);
    do_test_new_file_and_save('p', "testfile.policy"  , false);
    do_test_new_file_and_save('r', "testfile.rounding", false);
    do_test_new_file_and_save('s', "testfile.strata",   false);
    do_test_new_file_and_save('m', "testfile.mec",      true);
    do_test_new_file_and_save('g', "testfile.gpt",      true);
    do_test_new_file_and_save('x', "testfile.txt",      false);
}
