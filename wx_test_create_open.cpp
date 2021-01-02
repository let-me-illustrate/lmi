// Create, save, and reopen a file of each available type.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
#include "version.hpp"
#include "wx_test_case.hpp"
#include "wx_test_output.hpp"

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
void do_test_create_open
        (wx_base_test_case& test
        ,int key
        ,char const* basename
        ,bool uses_dialog)
{
    test.skip_if_not_supported(basename);

    wxString const file = test.get_test_file_path_for(basename);
    LMI_ASSERT(!wxFileExists(file));

    output_file_existence_checker output_file{file.ToStdString()};

    wxUIActionSimulator z;
    z.Char('n', wxMOD_CONTROL); // new file
    z.Char(key               ); // choose document type
    if(uses_dialog)
        {
        wxTEST_DIALOG
            (wxYield()
            ,wxExpectDismissableModal<MvcController>(wxID_OK).
                Describe("new file properties")
            );
        }
    wxYield();

    z.Char(uses_dialog ? 'a' : 's', wxMOD_CONTROL); // save or save as
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(file).Describe("save file dialog")
        );
    wxYield();

    LMI_ASSERT(output_file.exists());

    z.Char('l', wxMOD_CONTROL); // close document
    wxYield();

    z.Char('o', wxMOD_CONTROL); // and open it again

    if(uses_dialog)
        {
        wxTEST_DIALOG
            (wxYield()
            ,wxExpectModal<wxFileDialog>(file).Describe("open file dialog")
            ,wxExpectDismissableModal<MvcController>(wxID_OK).
                Describe("existing file properties")
            );
        }
    else
        {
        wxTEST_DIALOG
            (wxYield()
            ,wxExpectModal<wxFileDialog>(file).Describe("open file dialog")
            );
        }
    wxYield();

    z.Char('l', wxMOD_CONTROL); // close it finally
    wxYield();
}

/// Create, save, and reopen a file of each available type.
///
/// Validate each tested operation, then erase the file.
///
/// Put all files in 'gui_test_path'. In theory, they'll all be
/// deleted automatically, but in practice the program could crash.
///
/// Skip file types that are unavailable in context. For example,
/// '.txt' is available only with '--ash_nazg'.

LMI_WX_TEST_CASE(create_open_census)
{
    do_test_create_open(*this, 'c', "testfile.cns",  false);
}

LMI_WX_TEST_CASE(create_open_illustration)
{
    do_test_create_open(*this, 'i', "testfile.ill",  true);
}

LMI_WX_TEST_CASE(create_open_database)
{
    do_test_create_open(*this, 'd', "testfile.database", false);
}

LMI_WX_TEST_CASE(create_open_policy)
{
    do_test_create_open(*this, 'p', "testfile.policy",  false);
}

LMI_WX_TEST_CASE(create_open_rounding)
{
    do_test_create_open(*this, 'r', "testfile.rounding", false);
}

LMI_WX_TEST_CASE(create_open_strata)
{
    do_test_create_open(*this, 's', "testfile.strata", false);
}

LMI_WX_TEST_CASE(create_open_mec)
{
    auto const& tsv_ext = configurable_settings::instance().spreadsheet_file_extension();
    std::string const xml_ext = ".xml"; // define for consistency with tsv_ext

    // The generic function only tests for testfile.mec existence and only
    // removes it at the end, but saving a MEC document creates 2 other files
    // with the extra suffixes that we deal with here. Notice that these files
    // are created in the current directory and not in the directory normally
    // used for the test files.
    //
    // Additionally, just creating, without saving, a MEC document currently
    // creates 2 other files as a side-effect, so deal with them here as well.
    auto const number_of_extra_files = 4;
    output_file_existence_checker extra_output_files[number_of_extra_files] =
        {output_file_existence_checker{"testfile.mec" + tsv_ext}
        ,output_file_existence_checker{"testfile.mec" + xml_ext}
        ,output_file_existence_checker{"unnamed.mec" + tsv_ext}
        ,output_file_existence_checker{"unnamed.mec" + xml_ext}
        };

    do_test_create_open(*this, 'm', "testfile.mec", true);

    for(int n = 0; n < number_of_extra_files; ++n)
        {
        LMI_ASSERT_WITH_MSG
            (extra_output_files[n].exists()
            ,"file \"" << extra_output_files[n].path() << "\""
            );
        }
}

LMI_WX_TEST_CASE(create_open_gpt)
{
    // Similarly to MEC files above, creating and saving GPT documents creates
    // extra files in the current directory, but here there are only 2 of them,
    // so don't bother with using an array.
    output_file_existence_checker output_unnamed_gpt_xml{"unnamed.gpt.xml"};
    output_file_existence_checker output_testfile_gpt_xml{"testfile.gpt.xml"};

    do_test_create_open(*this, 'g', "testfile.gpt", true);

    LMI_ASSERT(output_unnamed_gpt_xml.exists());
    LMI_ASSERT(output_testfile_gpt_xml.exists());
}

LMI_WX_TEST_CASE(create_open_text)
{
    do_test_create_open(*this, 'x', "testfile.txt", false);
}
