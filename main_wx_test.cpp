// Main file for automated testing of wx interface.
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "docmanager_ex.hpp"
#include "force_linking.hpp"
#include "handle_exceptions.hpp"        // stealth_exception
#include "main_common.hpp"              // initialize_application()
#include "msw_workarounds.hpp"
#include "obstruct_slicing.hpp"
#include "path_utility.hpp"             // initialize_filesystem()
#include "skeleton.hpp"
#include "uncopyable_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"

#include <wx/crt.h>
#include <wx/docview.h>
#include <wx/fileconf.h>
#include <wx/frame.h>
#include <wx/init.h>                    // wxEntry()
#include <wx/msgdlg.h>
#include <wx/scopeguard.h>
#include <wx/stopwatch.h>
#include <wx/testing.h>
#include <wx/uiaction.h>
#include <wx/wfstream.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
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

    // Return the configured directory (current one by default) to use for the
    // test files.
    fs::path const& get_test_files_path() const { return test_files_path_; }

    // Used to check if distribution tests should be enabled.
    bool is_distribution_test() const { return is_distribution_test_; }

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

    fs::path test_files_path_;

    bool run_all_;

    bool is_distribution_test_;
};

application_test::application_test()
    :run_all_(true)
    ,is_distribution_test_(false)
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
    // THIRD_PARTY !! We have this long and error-prone code to parse the
    // command line manually here only because getopt_long() is not composable
    // and so we can't use it with our own options here while still leaving the
    // standard options for the base class to handle. It would be better to use
    // a standard command line parsing mechanism if it ever becomes possible.

    // This variable is used both as a flag indicating that the last option was
    // the one selecting the test to run and so must be followed by the test
    // name, but also for the diagnostic message at the end of this function.
    char const* last_test_option = 0;

    char const* opt_gui_test_path = "--gui_test_path";
    int const opt_gui_test_path_length = strlen(opt_gui_test_path);

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
        else if(0 == std::strcmp(arg, "--distribution"))
            {
            is_distribution_test_ = true;
            remove_arg(n, argc, argv);
            }
        else if(0 == std::strncmp(arg, opt_gui_test_path, opt_gui_test_path_length))
            {
            if (arg[opt_gui_test_path_length]=='=')
                {
                test_files_path_ = arg + opt_gui_test_path_length + 1;
                }
            else
                {
                if (n == argc - 1)
                    {
                    warning()
                        << "Option '"
                        << opt_gui_test_path
                        << "' must be followed by the path to use."
                        << std::flush
                        ;
                    }
                else
                    {
                    remove_arg(n, argc, argv);
                    test_files_path_ = argv[n];
                    }
                }

            remove_arg(n, argc, argv);
            }
        else if
            (
               0 == std::strcmp(arg, "-h")
            || 0 == std::strcmp(arg, "--help")
            )
            {
            std::ostringstream oss;
            oss
                << "Run automated GUI tests.\n"
                   "\n"
                   "Usage: "
                << argv[0]
                << "\n"
                   "  -h,\t--help           \tdisplay this help and exit\n"
                   "  -l,\t--list           \tlist all available tests and exit\n"
                   "  -t <name> or          \trun only the specified test (may occur\n"
                   "  --test <name>         \tmultiple times); default: run all tests\n"
                   "  --gui_test_path <path>\tpath to use for test files\n"
                   "  --distribution        \tenable distribution-specific tests\n"
                   "\n"
                   "Additionally, all command line options supported by the\n"
                   "main lmi executable are also supported."
                ;
            wxMessageBox(oss.str(), "Command-line options");
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

    // Ensure that the path used for the test files is always valid and
    // absolute, so that it doesn't change even if the program current
    // directory changes for whatever reason.
    if(test_files_path_.empty())
        {
        test_files_path_ = "/opt/lmi/gui_test";
        }

    if (!fs::exists(test_files_path_))
        {
        warning()
            << "Test files path '"
            << test_files_path_.native_file_string()
            << "' doesn't exist."
            << std::flush
            ;
        test_files_path_ = fs::current_path();
        }
    else
        {
        test_files_path_ = fs::system_complete(test_files_path_);
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
                wxPrintf("%s: started\n", name);
                wxStopWatch sw;
                i->run_test();
                // Check that no messages were unexpectedly logged during this
                // test execution.
                wxLog::FlushActive();
                wxPrintf("time=%ldms (for %s)\n", sw.Time(), name);
                wxPrintf("%s: ok\n", name);
                results.passed++;
                }
            catch(test_skipped_exception const& e)
                {
                wxPrintf("%s: skipped (%s)\n", name, e.what());
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

                wxPrintf("%s: ERROR (%s)\n", name, one_line_error);
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

    std::cout << "Available tests:\n";

    typedef std::vector<test_descriptor>::const_iterator ctdi;
    for(ctdi i = tests_.begin(); i != tests_.end(); ++i)
        {
        std::cout << '\t' << i->get_name() << '\n';
        }

    std::cout << tests_.size() << " test cases.\n";
}

} // Unnamed namespace.

wx_base_test_case::wx_base_test_case(char const* name)
    :m_name(name)
{
    application_test::instance().add_test(this);
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

fs::path wx_base_test_case::get_test_files_path() const
{
    return application_test::instance().get_test_files_path();
}

std::string
wx_base_test_case::get_test_file_path_for(std::string const& basename) const
{
    return (get_test_files_path() / basename).native_file_string();
}

bool wx_base_test_case::is_distribution_test() const
{
    return application_test::instance().is_distribution_test();
}

void wx_base_test_case::skip_if_not_distribution()
{
    if(!is_distribution_test())
        {
        throw test_skipped_exception("not running distribution tests");
        }
}

wxWindow* wx_test_focus_controller_child(MvcController& dialog, char const* name)
{
    // First find the window anywhere inside the dialog.
    wxWindow* const w = wxWindow::FindWindowByName(name, &dialog);
    LMI_ASSERT_WITH_MSG(w, "window named \"" << name << "\" not found");

    // Then find the book control containing it by walking up the window chain
    // until we reach it.
    for (wxWindow* maybe_page = w;;)
        {
        wxWindow* const maybe_book = maybe_page->GetParent();

        // As we know that w is a descendant of the dialog, this check ensures
        // that the loop terminates as sooner or later we must reach the dialog
        // by walking up the parent chain.
        LMI_ASSERT_WITH_MSG
            (maybe_book != &dialog
            ,"book control containing window \"" << name << "\" not found"
            );

        if (wxBookCtrlBase* const book = dynamic_cast<wxBookCtrlBase*>(maybe_book))
            {
            // We found the notebook, now we can use it to make the page
            // containing the target window current.
            size_t const num_pages = book->GetPageCount();
            for (size_t n = 0; n < num_pages; n++)
                {
                if (book->GetPage(n) == maybe_page)
                    {
                    book->SetSelection(n);
                    wxYield();

                    break;
                    }
                }

            break;
            }

        maybe_page = maybe_book;
        }

    // Finally set the focus to the target window and ensure all events
    // generated because of this are processed.
    w->SetFocus();
    wxYield();

    return w;
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
    // Override base class virtual method.
    virtual DocManagerEx* CreateDocManager();

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
    std::string domain_error_;
    bool is_running_tests_;
};

IMPLEMENT_APP_NO_MAIN(SkeletonTest)
IMPLEMENT_WX_THEME_SUPPORT

DocManagerEx* SkeletonTest::CreateDocManager()
{
    // Custom document manager allowing to intercept the behaviour of the
    // program for testing purposes.
    //
    // Currently it is used to prevent the files opened during testing from
    // being saved.
    class DocManagerTest : public DocManagerEx
    {
      public:
        virtual void FileHistoryLoad(wxConfigBase const&)
            {
            // We could call the base class method here, but it doesn't seem
            // useful to do it and doing nothing here makes it more symmetric
            // with FileHistorySave().
            }

        virtual void FileHistorySave(wxConfigBase&)
            {
            // Do not save the history to persistent storage: we don't want the
            // files opened during testing replace the files actually opened by
            // the user interactively.
            }
    };

    // As in the base class version, notice that we must not use 'new(wx)' here
    // as this object is deleted by wxWidgets itself.
    return new DocManagerTest;
}

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

bool SkeletonTest::StoreCurrentException()
{
    // We store all the exceptions that are expected to be caught by the tests
    // here, in order to be able to rethrow them later. Almost all tests need
    // just std::runtime_error, but the input validation one also can get a
    // domain_error in some cases, so we need to handle that one as well.
    try
        {
        throw;
        }
    catch(std::domain_error& e)
        {
        domain_error_ = e.what();
        return true;
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
    if(!domain_error_.empty())
        {
        std::domain_error const e(domain_error_);
        domain_error_.clear();
        throw e;
        }

    if(!runtime_error_.empty())
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

    // Any dialog shown during the tests should be accounted for and dismissed
    // before it gets to this outer hook by the testing hook expecting it, so
    // this hook should never be invoked and we install it to ensure that it
    // prevents any unexpected modal dialogs from being shown if they do
    // happen, this is important for the test to really run unattended.
    wxTestingModalHook expect_no_dialogs;

    mainWin->SetFocus();

    wxPuts("NOTE: starting the test suite");
    wxStopWatch sw;

    // Notice that it is safe to use simple variable assignment here instead of
    // some RAII-based pattern because of application_test::run() noexcept
    // guarantee.
    is_running_tests_ = true;
    TestsResults const results = application_test::instance().run();
    is_running_tests_ = false;

    wxPrintf("time=%ldms (for all tests)\n", sw.Time());

    if(results.failed == 0)
        {
        if(results.passed == 0)
            {
            wxPuts("WARNING: no tests have been executed.");
            }
        else
            {
            wxPrintf
                ("SUCCESS: %d test%s successfully completed.\n"
                ,results.passed
                ,results.passed == 1 ? "" : "s"
                );
            }
        }
    else
        {
        wxPrintf
            ("FAILURE: %d out of %d test%s failed.\n"
            ,results.failed
            ,results.total
            ,results.total == 1 ? "" : "s"
            );
        }

    if(results.skipped)
        {
        wxPrintf
            ("NOTE: %s skipped\n"
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

