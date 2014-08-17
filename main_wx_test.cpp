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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "alert.hpp"
#include "main_common.hpp"              // initialize_application()
#include "msw_workarounds.hpp"
#include "obstruct_slicing.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "skeleton.hpp"
#include "uncopyable_lmi.hpp"
#include "wx_test_case.hpp"

#include <wx/frame.h>
#include <wx/init.h>                    // wxEntry()
#include <wx/link.h>
#include <wx/stopwatch.h>

#include <cstring>                      // std::strcmp()
#include <iostream>
#include <vector>

wxFORCE_LINK_MODULE(file_command_wx)
wxFORCE_LINK_MODULE(progress_meter_wx)
wxFORCE_LINK_MODULE(system_command_wx)

class SkeletonTest;
DECLARE_APP(SkeletonTest)

namespace
{
/// Run the tests.
///
/// This is a simple Meyers singleton.
class application_test
    :        private lmi::uncopyable  <application_test>
    ,virtual private obstruct_slicing <application_test>
{
  public:
    static application_test& instance();

    // Check the command line for the test-specific options, handle them and
    // remove them from argv.
    //
    // Return false if the program execution shouldn't continue, currently this
    // is only the case if the "list" option was specified requesting just to
    // list the available tests.
    bool process_command_line(int& argc, char* argv[]);

    // Run all the tests that were configured to be executed (all by default).
    void run();

    // Used by LMI_WX_TEST_CASE() macro to register the individual test cases.
    bool add_test(void (*test_func)(), char const* test_name);

  private:
    application_test();

    // List all tests on standard output.
    void list_tests();

    // Include of exclude the given test depending on the value given on the
    // command line.
    void process_test_name(const char* name);


    // A test can be explicitly included, explicitly excluded or not
    // mentioned, in which case it will run if all tests are ran by default.
    enum test_run
        {run_yes
        ,run_no
        ,run_default
        };

    /// Contains everything we need to store for an individual test.
    struct test_descriptor
    {
        // Notice that the constructor doesn't copy the test name, it's a
        // literal anyhow, so we just store the pointer.
        test_descriptor(void (*func)(), char const* name)
            :func(func)
            ,name(name)
            ,run(run_default)
        {
        }

        void (*func)();
        char const* name;
        test_run run;
    };

    std::vector<test_descriptor> tests_;

    bool run_all_;
};

application_test::application_test()
    :run_all_(true)
{
}

application_test& application_test::instance()
{
    static application_test z;
    return z;
}

void application_test::process_test_name(const char* name)
{
    // A test can be specified either as "test" to run it, or "-test" to avoid
    // running it, check which one have we got.
    test_run run;
    if (name[0] == '-')
        {
        run = run_no;
        name++; // Skip the leading minus sign.
        }
    else
        {
        // If some test is explicitly requested, all the other ones are
        // implicitly disabled, otherwise it wouldn't make sense.
        run_all_ = false;
        run = run_yes;
        }

    typedef std::vector<test_descriptor>::iterator tdi;
    for(tdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        if (std::strcmp(i->name, name) == 0)
            {
            i->run = run;
            return;
            }
        }

    warning()
        << "Unrecognized test name '"
        << name
        << "', use --list command line option to list all tests."
        << std::flush
        ;
}

// Remove the argument at the given (assumed valid) position from (argc, argv).
void remove_arg(int n, int& argc, char* argv[])
{
    // We include argv[argc] in the elements being copied, this guarantees that
    // the array remains 0-terminated.
    std::memmove(argv + n, argv + n + 1, (argc - n)*sizeof(char*));

    argc--;
}

bool application_test::process_command_line(int& argc, char* argv[])
{
    // This variable is used both as a flag indicating that the last option was
    // the one selecting the test to run and so must be followed by the test
    // name, but also for the diagnostic message at the end of this function.
    char const* last_test_option = 0;

    for(int n = 1; n < argc; )
        {
        char const* const arg = argv[n];

        if (last_test_option)
            {
            last_test_option = 0;
            process_test_name(arg);
            remove_arg(n, argc, argv);
            continue;
            }
        else if
            (  0 == std::strcmp(arg, "-l")
            || 0 == std::strcmp(arg, "--list")
            )
            {
            list_tests();
            return false;
            }
        else if
            (  0 == std::strcmp(arg, "-t")
            || 0 == std::strcmp(arg, "--test")
            )
            {
            last_test_option = arg;
            remove_arg(n, argc, argv);
            }
        else
            {
            n++;
            }
        }

    if (last_test_option)
        {
        warning()
            << "Option '"
            << last_test_option
            << "' must be followed by the test name."
            << std::flush
            ;
        }

    return true;
}

void application_test::run()
{
    typedef std::vector<test_descriptor>::const_iterator ctdi;
    for(ctdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        if ((run_all_ && i->run != run_no) || i->run == run_yes)
            {
            (*i->func)();
            }
        }
}

bool application_test::add_test(void (*test_func)(), char const* test_name)
{
    tests_.push_back(test_descriptor(test_func, test_name));

    return true;
}

void application_test::list_tests()
{
    std::cerr << "Available tests:\n";

    typedef std::vector<test_descriptor>::const_iterator ctdi;
    for(ctdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        std::cerr << '\t' << i->name << '\n';
        }

    std::cerr << tests_.size() << " test cases.\n";
}

} // Unnamed namespace.

bool add_wx_test_case(void (*test_func)(), char const* test_name)
{
    return application_test::instance().add_test(test_func, test_name);
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

    wxStopWatch sw;
    wxLogMessage("Starting automatic tests.");
    application_test::instance().run();
    wxLogMessage("Tests successfully completed in %ldms.", sw.Time());

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

    // We need to handle test-specific options and remove them from argv before
    // letting wxEntry() instantiate Skeleton application object that would
    // give an error for these, unknown to it, options.
    if (!application_test::instance().process_command_line(argc, argv))
        {
        return 0;
        }

    return wxEntry(argc, argv);
}

