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
#include "configurable_settings.hpp"
#include "main_common.hpp"              // initialize_application()
#include "msw_workarounds.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "skeleton.hpp"
#include "version.hpp"

#include <boost/filesystem/operations.hpp>

#include <wx/dialog.h>
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
        }

  private:
    static void test_about_dialog_version();
    static void test_configurable_settings();
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
    application_test::test();
    ExitMainLoop();
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

