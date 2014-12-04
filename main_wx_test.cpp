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
#include "force_linking.hpp"
#include "handle_exceptions.hpp"        // stealth_exception
#include "main_common.hpp"              // initialize_application()
#include "msw_workarounds.hpp"
#include "obstruct_slicing.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "skeleton.hpp"
#include "uncopyable_lmi.hpp"
#include "wx_test_case.hpp"

#include <wx/docview.h>
#include <wx/fileconf.h>
#include <wx/frame.h>
#include <wx/init.h>                    // wxEntry()
#include <wx/scopeguard.h>
#include <wx/stopwatch.h>
#include <wx/uiaction.h>
#include <wx/wfstream.h>

#include <boost/scoped_ptr.hpp>

#include <algorithm>                    // std::sort()
#include <cstring>                      // std::strcmp()
#include <exception>                    // uncaught_exception()
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

LMI_FORCE_LINKING_EX_SITU(file_command_wx)
LMI_FORCE_LINKING_EX_SITU(progress_meter_wx)
LMI_FORCE_LINKING_EX_SITU(system_command_wx)

#if !wxCHECK_VERSION(3,1,0)
#   error wxWidgets 3.1.0 or later is required for the test suite.
#endif

static char const* const LOG_PREFIX = "[TEST] ";

class SkeletonTest;
DECLARE_APP(SkeletonTest)

/// Implement this normally unimplemented function.
///
/// The implementation of this constructor is not provided to prevent
/// production code from creating objects of this type, but we do need to use
/// this exception here, for the special testing purposes, so explicitly opt in
/// into using it by provide this implementation.
stealth_exception::stealth_exception(std::string const& what_arg)
    :std::runtime_error(what_arg)
{}

namespace
{

/// Exception thrown if a wxWidgets assertion fails during the test code
/// execution. It must inherit from stealth_exception to avoid this exception
/// being caught, reported and ignored by well-meaning but harmful in this case
/// exception handling code elsewhere.
///
/// Implicitly-declared special member functions do the right thing.

class test_assertion_failure_exception
    :public stealth_exception
{
  public:
    test_assertion_failure_exception
        (wxChar const* msg
        ,wxChar const* file
        ,int line
        ,wxChar const* func
        )
        :stealth_exception
            (wxString::Format
                ("Assertion failure: %s [file %s, line %d, in %s()]."
                ,msg
                ,file
                ,line
                ,func
                ).ToStdString()
            )
        {
        }
};

/// Exception thrown if the test needs to be skipped.
///
/// This exception doesn't carry any extra information but just needs to have a
/// distinct type to allow treating it differently in run().
class test_skipped_exception
    :public stealth_exception
{
  public:
    test_skipped_exception(std::string const& what)
        :stealth_exception(what)
        {
        }
};

/// Simple struct collecting the statistics about the tests we ran.
///
/// Implicitly-declared special member functions do the right thing.
struct TestsResults
{
    TestsResults()
        :total(0)
        ,passed(0)
        ,skipped(0)
        ,failed(0)
    {
    }

    // The sum of passed, skipped and failed is the same as total (except when
    // a test is in process of execution and its result is yet unknown).
    int total,
        passed,
        skipped,
        failed;
};

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
    // list the available tests or if the standard "help" option was given.
    bool process_command_line(int& argc, char* argv[]);

    // Run all the tests that were configured to be executed (all by default).
    //
    // This function consumes all the exceptions thrown during its execution
    // and never throws itself.
    TestsResults run() /* noexcept */;

    // Used by LMI_WX_TEST_CASE() macro to register the individual test cases.
    void add_test(wx_base_test_case* test);

    // Used by tests to retrieve their configuration parameters.
    wxConfigBase const& get_config_for(char const* name);

  private:
    application_test();

    // Sort all tests in alphabetical order of their names.
    void sort_tests();

    // List all tests on standard output.
    void list_tests();

    // Include of exclude the given test depending on the value given on the
    // command line.
    void process_test_name(char const* name);

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
        // The pointer must be non-NULL but we don't take ownership of it.
        test_descriptor(wx_base_test_case* test)
            :test(test)
            ,run(run_default)
        {
        }

        char const* get_name() const { return test->get_name(); }

        void run_test() const { test->run(); }

        // Comparator used for sorting the tests.
        static bool Compare
            (test_descriptor const& t1
            ,test_descriptor const& t2)
            {
            return std::strcmp(t1.get_name(), t2.get_name()) < 0;
            }

        wx_base_test_case* test;
        test_run run;
    };

    std::vector<test_descriptor> tests_;

    boost::scoped_ptr<wxFileConfig> config_;

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

void application_test::process_test_name(char const* name)
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

    bool any_tests_matched = false;

    typedef std::vector<test_descriptor>::iterator tdi;
    for(tdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        if (wxString(i->get_name()).Matches(name))
            {
            i->run = run;
            any_tests_matched = true;
            }
        }

    if (!any_tests_matched)
        {
        warning()
            << "Test specification '"
            << name
            << "', didn't match any tests.\n"
            << "Use --list command line option to list all tests."
            << std::flush
            ;
        }
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
        else if
            (
               0 == std::strcmp(arg, "-h")
            || 0 == std::strcmp(arg, "--help")
            )
            {
            warning()
                << "Run automated GUI tests.\n"
                   "\n"
                   "Usage: "
                << argv[0]
                << "\n"
                   "  -h,\t--help  \tdisplay this help and exit\n"
                   "  -l,\t--list  \tlist all available tests and exit\n"
                   "  -t <name> or \trun only the specified test (may occur\n"
                   "  --test <name>\tmultiple times); default: run all tests\n"
                   "\n"
                   "Additionally, all command line options supported by the\n"
                   "main lmi executable are also supported."
                << std::flush;
            return false;
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

TestsResults application_test::run()
{
    // Always run the tests in the same, predictable order (we may want to add
    // a "random shuffle" option later, but even then predictable behaviour
    // should arguably remain the default).
    sort_tests();

    TestsResults results;

    typedef std::vector<test_descriptor>::const_iterator ctdi;
    for(ctdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        if ((run_all_ && i->run != run_no) || i->run == run_yes)
            {
            std::string error;
            results.total++;

            char const* const name = i->get_name();

            try
                {
                wxLogMessage("%s%s: started", LOG_PREFIX, name);
                wxStopWatch sw;
                i->run_test();
                wxLogMessage("%stime=%ldms (for %s)", LOG_PREFIX, sw.Time(), name);
                wxLogMessage("%s%s: ok", LOG_PREFIX, name);
                results.passed++;
                }
            catch(test_skipped_exception const& e)
                {
                wxLogMessage("%s%s: skipped (%s)", LOG_PREFIX, name, e.what());
                results.skipped++;
                }
            catch(std::exception const& e)
                {
                error = e.what();
                }
            catch(...)
                {
                error = "unknown exception";
                }

            if (!error.empty())
                {
                results.failed++;

                // Keep everything on a single line to ensure the full text of
                // the error appears if the output is filtered by the test name
                // using standard line-oriented tools such as grep.
                wxString one_line_error(error);
                one_line_error.Replace("\n", " ");

                wxLogMessage
                    ("%s%s: ERROR (%s)"
                    ,LOG_PREFIX
                    ,name
                    ,one_line_error
                    );
                }
            }
        }

    return results;
}

void application_test::add_test(wx_base_test_case* test)
{
    tests_.push_back(test_descriptor(test));
}

void application_test::sort_tests()
{
    std::sort(tests_.begin(), tests_.end(), test_descriptor::Compare);
}

void application_test::list_tests()
{
    sort_tests();

    std::cerr << "Available tests:\n";

    typedef std::vector<test_descriptor>::const_iterator ctdi;
    for(ctdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        std::cerr << '\t' << i->get_name() << '\n';
        }

    std::cerr << tests_.size() << " test cases.\n";
}

wxConfigBase const& application_test::get_config_for(char const* name)
{
    if(!config_)
        {
        wxFFileInputStream is("wx_test.conf", "r");
        config_.reset(new wxFileConfig(is));
        }

    config_->SetPath(wxString("/") + name);

    return *config_;
}

} // Unnamed namespace.

wx_base_test_case::wx_base_test_case(char const* name)
    :m_name(name)
{
    application_test::instance().add_test(this);
}

wxConfigBase const& wx_base_test_case::config() const
{
    return application_test::instance().get_config_for(get_name());
}

void wx_base_test_case::skip_if_not_supported(char const* file)
{
    const wxString p(file);
    if(!wxDocManager::GetDocumentManager()->FindTemplateForPath(p))
        {
        throw test_skipped_exception
                (wxString::Format
                    ("documents with extension \"%s\" not supported"
                    ,p.AfterLast('.')
                    ).ToStdString()
                );
        }
}

// Application to drive the tests
class SkeletonTest : public Skeleton
{
  public:
    SkeletonTest()
        :is_running_tests_(false)
    {
    }

  protected:
    // wxApp overrides.
    virtual bool OnInit                 ();
    virtual bool OnExceptionInMainLoop  ();
    virtual bool StoreCurrentException  ();
    virtual void RethrowStoredException ();
    virtual void OnAssertFailure
        (wxChar const* file
        ,int line
        ,wxChar const* func
        ,wxChar const* cond
        ,wxChar const* msg
        );

  private:
    void RunTheTests();

    std::string runtime_error_;
    bool is_running_tests_;
};

IMPLEMENT_APP_NO_MAIN(SkeletonTest)
IMPLEMENT_WX_THEME_SUPPORT

bool SkeletonTest::OnInit()
{
    // The test output should be reproducible, so disable the time
    // stamps in the logs to avoid spurious differences due to them.
    wxLog::DisableTimestamp();

    // Log everything to stderr, both to avoid interacting with the user (who
    // might not even be present) and to allow redirecting the test output to a
    // file which may subsequently be compared with the previous test runs.
    delete wxLog::SetActiveTarget(new wxLogStderr);

    if(!Skeleton::OnInit())
        {
        return false;
        }

    // Run the tests at idle time, when the main loop is running, in order to
    // do it in as realistic conditions as possible.
    CallAfter(&SkeletonTest::RunTheTests);

    return true;
}

bool SkeletonTest::StoreCurrentException()
{
    try
        {
        throw;
        }
    catch (std::runtime_error const& e)
        {
        runtime_error_ = e.what();
        return true;
        }

    return false;
}

void SkeletonTest::RethrowStoredException()
{
    if (!runtime_error_.empty())
        {
        std::runtime_error const e(runtime_error_);
        runtime_error_.clear();
        throw e;
        }
}

bool SkeletonTest::OnExceptionInMainLoop()
{
    if (is_running_tests_)
        {
        // Don't let the base class catch, report and ignore the exceptions
        // that happen while running the tests, we need to ensure that the test
        // fails as the result of assert happening during its run.
        throw;
        }

    return Skeleton::OnExceptionInMainLoop();
}

void SkeletonTest::OnAssertFailure
    (wxChar const* file
    ,int line
    ,wxChar const* func
    ,wxChar const* cond
    ,wxChar const* msg
    )
{
    // Assertion during a test run counts as test failure but avoid throwing if
    // another exception is already in flight as this would just result in the
    // program termination without any useful information about the reason of
    // the failure whatsoever.
    if (is_running_tests_ && !std::uncaught_exception())
        {
        throw test_assertion_failure_exception(msg ? msg : cond, file, line, func);
        }
    else
        {
        Skeleton::OnAssertFailure(file, line, func, cond, msg);
        }
}

void SkeletonTest::RunTheTests()
{
    wxWindow* const mainWin = GetTopWindow();
    if (!mainWin)
        {
        wxLogError("Failed to find the application main window.");
        ExitMainLoop();
        return;
        }

    // Whatever happens, ensure that the main window is closed and thus the
    // main loop terminated and the application exits at the end of the tests.
    wxON_BLOCK_EXIT_OBJ1(*mainWin, wxWindow::Close, true /* force close */);

    // Close any initially opened dialogs (e.g. "About" dialog shown unless a
    // special command line option is specified).
    for (;;)
        {
        wxWindow* const activeWin = wxGetActiveWindow();
        if (!activeWin || activeWin == mainWin)
            break;

        // Try to close the dialog.
        wxUIActionSimulator ui;
        ui.Char(WXK_ESCAPE);
        wxYield();

        // But stop trying if it didn't work.
        if (wxGetActiveWindow() == activeWin)
            {
            wxLogError("Failed to close the currently opened window, "
                       "please ensure it doesn't appear on program startup.");
            return;
            }
        }

    mainWin->SetFocus();

    wxLogMessage("%sNOTE: starting the test suite", LOG_PREFIX);
    wxStopWatch sw;

    // Notice that it is safe to use simple variable assignment here instead of
    // some RAII-based pattern because of application_test::run() noexcept
    // guarantee.
    is_running_tests_ = true;
    TestsResults const results = application_test::instance().run();
    is_running_tests_ = false;

    wxLogMessage("%stime=%ldms (for all tests)", LOG_PREFIX, sw.Time());

    if(results.failed == 0)
        {
        if(results.passed == 0)
            {
            wxLogMessage("%sWARNING: no tests have been executed.", LOG_PREFIX);
            }
        else
            {
            wxLogMessage
                ("%sSUCCESS: %d test%s successfully completed."
                ,LOG_PREFIX
                ,results.passed
                ,results.passed == 1 ? "" : "s"
                );
            }
        }
    else
        {
        wxLogMessage
            ("%sFAILURE: %d out of %d test%s failed."
            ,LOG_PREFIX
            ,results.failed
            ,results.total
            ,results.total == 1 ? "" : "s"
            );
        }

    if(results.skipped)
        {
        wxLogMessage
            ("%sNOTE: %s skipped"
            ,LOG_PREFIX
            ,results.skipped == 1
                ? wxString("1 test was")
                : wxString::Format("%d tests were", results.skipped)
            );
        }
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

