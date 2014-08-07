// Main file for automated testing of wx interface.
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
#include "calendar_date.hpp"
#include "configurable_settings.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "main_common.hpp"              // initialize_application()
#include "msw_workarounds.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "skeleton.hpp"
#include "version.hpp"

#include <boost/filesystem/operations.hpp>

#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/init.h>                    // wxEntry()
#include <wx/testing.h>
#include <wx/uiaction.h>

class SkeletonTest;
DECLARE_APP(SkeletonTest)

class application_test
{
  public:
    static void test()
        {
        test_about_dialog_version();
        test_configurable_settings();
        test_default_input();
        test_new_file_and_save();
        }

  private:
    static void test_about_dialog_version();
    static void test_configurable_settings();
    static void test_default_input();
    static void test_new_file_and_save();

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
    static void do_test_new_file_and_save(int key, wxString const& file, bool uses_dialog);
};

void application_test::test_about_dialog_version()
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

void application_test::test_configurable_settings()
{
    LMI_ASSERT(fs::exists("/etc/opt/lmi/configurable_settings.xml"));

    configurable_settings const& settings = configurable_settings::instance();
    LMI_ASSERT("" == settings.libraries_to_preload());
    LMI_ASSERT("CMD /c c:/fop-0.20.5/fop" == settings.xsl_fo_command());

    std::string skin = settings.skin_filename();
    std::string default_input = settings.default_input_filename();
    LMI_ASSERT(   "skin_coli_boli.xrc" == skin
               || "skin_group_carveout.xrc" == skin
               || "skin_group_carveout2.xrc" == skin
               || "reg_d.xrc" == skin);
    if ("skin_coli_boli.xrc" == skin)
        {
        LMI_ASSERT("c:/fop-0.20.5/coli_boli_default.ill" == default_input);
        }
    if ("skin_group_carveout.xrc" == skin)
        {
        LMI_ASSERT("c:/fop-0.20.5/group_carveout_default.ill" == default_input);
        }
    if ("skin_group_carveout2.xrc" == skin)
        {
        LMI_ASSERT("c:/fop-0.20.5/group_carveout_default.ill" == default_input);
        }
    if ("reg_d.xrc" == skin)
        {
        LMI_ASSERT("c:/fop-0.20.5/private_placement_default.ill" == default_input);
        }
}

void application_test::test_default_input()
{
    calendar_date const today;
    calendar_date const first_of_month(today.year(), today.month(), 1);

    Input const& cell = default_cell();
    calendar_date const effective_date = exact_cast<tnr_date>(cell["EffectiveDate"])->value();
    LMI_ASSERT(first_of_month == effective_date);

    std::string const general_account_date = exact_cast<numeric_sequence>(cell["GeneralAccountRate"])->value();
    LMI_ASSERT(!general_account_date.empty());
    wxLogMessage("GeneralAccountRate is \"%s\"", general_account_date.c_str());
}

void application_test::do_test_new_file_and_save(int key, wxString const& file, bool uses_dialog)
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

void application_test::test_new_file_and_save()
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

// Application to drive the tests
class SkeletonTest : public Skeleton
{
  public:
    SkeletonTest() {}

  protected:
    // wxApp overrides.
    virtual bool OnInit();

  private:
    void RunTheTests();
};

IMPLEMENT_APP_NO_MAIN(SkeletonTest)
IMPLEMENT_WX_THEME_SUPPORT

bool SkeletonTest::OnInit()
{
    if(!Skeleton::OnInit())
        {
        return false;
        }

    // Run the tests at idle time, when the main loop is running, in order to
    // do it in as realistic conditions as possible.
    CallAfter(&SkeletonTest::RunTheTests);

    return true;
}

void SkeletonTest::RunTheTests()
{
    // Create log window for output that should be checked by the user.
    class LogWindow : public wxLogWindow
    {
      public:
        LogWindow() : wxLogWindow(NULL, "Log Messages", true, false) {}
        virtual bool OnFrameClose(wxFrame* frame)
        {
            wxTheApp->ExitMainLoop();
            return wxLogWindow::OnFrameClose(frame);
        }
    };
    LogWindow *log = new LogWindow();

    wxWindow *mainWin = GetTopWindow();
    mainWin->SetFocus();

    application_test::test();

    // We want to show log output after the tests finished running and hide the
    // app window, which is no longer in use. This doesn't work out of the box,
    // because the main window is set application's top window and closing it
    // terminates the app. LogWindow's window, on the other hand, doesn't keep
    // the app running because it returns false from ShouldPreventAppExit().
    // This code (together with LogWindow::OnFrameClose above) does the right
    // thing: close the main window and keep running until the user closes the
    // log window.
    log->GetFrame()->SetFocus();
    SetExitOnFrameDelete(false);
    mainWin->Close();
}

int main(int argc, char* argv[])
{
    initialize_application();
    initialize_filesystem();
#ifdef LMI_MSW
    MswDllPreloader::instance().PreloadDesignatedDlls();
#endif
    return wxEntry(argc, argv);
}

