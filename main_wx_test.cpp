// Main file for automated testing of wx interface.
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "docmanager_ex.hpp"
#include "force_linking.hpp"
#include "handle_exceptions.hpp"        // stealth_exception
#include "main_common.hpp"              // initialize_application()
#include "path.hpp"
#include "skeleton.hpp"
#include "ssize_lmi.hpp"                // sstrlen()
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"

#include <wx/docview.h>
#include <wx/fileconf.h>
#include <wx/frame.h>
#include <wx/init.h>                    // wxEntry()
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include <wx/testing.h>
#include <wx/uiaction.h>
#include <wx/wfstream.h>

#include <algorithm>                    // sort()
#include <cstring>                      // strcmp()
#include <exception>                    // uncaught_exceptions()
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

LMI_FORCE_LINKING_EX_SITU(file_command_wx)
LMI_FORCE_LINKING_EX_SITU(group_quote_pdf_generator_wx)
LMI_FORCE_LINKING_EX_SITU(pdf_command_wx)
LMI_FORCE_LINKING_EX_SITU(progress_meter_wx)
LMI_FORCE_LINKING_EX_SITU(system_command_wx)

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
        :total   {0}
        ,passed  {0}
        ,skipped {0}
        ,failed  {0}
    {
    }

    // The sum of passed, skipped and failed is the same as total (except when
    // a test is in process of execution and its result is yet unknown).
    int total;
    int passed;
    int skipped;
    int failed;
};

/// Run the tests.
///
/// This is a simple Meyers singleton.

class application_test final
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
    TestsResults run(); // noexcept

    // Used by LMI_WX_TEST_CASE() macro to register the individual test cases.
    void add_test(wx_base_test_case* test);

    // Return the configured directory (current one by default) to use for the
    // test files.
    fs::path const& get_test_files_path() const { return test_files_path_; }

    // Used to check if distribution tests should be enabled.
    bool is_distribution_test() const { return is_distribution_test_; }

    // Returns the exit code based of tests results.
    int get_exit_code() const { return exit_code_; }

  private:
    application_test() = default;
    application_test(application_test const&) = delete;
    application_test& operator=(application_test const&) = delete;

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
        test_descriptor(wx_base_test_case* t)
            :test {t}
            ,run  {run_default}
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

    bool run_all_              {true};

    bool is_distribution_test_ {false};

    int  exit_code_            {EXIT_FAILURE};
};

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
    if(name[0] == '-')
        {
        run = run_no;
        ++name; // Skip the leading minus sign.
        }
    else
        {
        // If some test is explicitly requested, all the other ones are
        // implicitly disabled, otherwise it wouldn't make sense.
        run_all_ = false;
        run = run_yes;
        }

    bool any_tests_matched = false;

    for(auto& i : tests_)
        {
        if(wxString(i.get_name()).Matches(name))
            {
            i.run = run;
            any_tests_matched = true;
            }
        }

    if(!any_tests_matched)
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
    std::memmove(argv + n, argv + n + 1, (argc - n) * sizeof(char*));
    --argc;
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
    char const* last_test_option = nullptr;

    char const* opt_gui_test_path = "--gui_test_path";
    int const opt_gui_test_path_length = lmi::sstrlen(opt_gui_test_path);

    for(int n = 1; n < argc; )
        {
        char const* const arg = argv[n];

        if(last_test_option)
            {
            last_test_option = nullptr;
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
            if(arg[opt_gui_test_path_length]=='=')
                {
                test_files_path_ = arg + opt_gui_test_path_length + 1;
                }
            else
                {
                if(n == argc - 1)
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
            ++n;
            }
        }

    if(last_test_option)
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

    if(!fs::exists(test_files_path_))
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
        test_files_path_ = fs::absolute(test_files_path_);
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

    for(auto const& i : tests_)
        {
        if((run_all_ && i.run != run_no) || i.run == run_yes)
            {
            std::string error;
            ++results.total;

            char const* const name = i.get_name();

            try
                {
                std::cout << name << ": started" << std::endl;
                wxStopWatch sw;
                i.run_test();
                // Check that no messages were unexpectedly logged during this
                // test execution.
                wxLog::FlushActive();
                std::cout
                    << "time="
                    << sw.Time()
                    << "ms (for "
                    << name
                    << ")"
                    << std::endl
                    ;
                std::cout << name << ": ok" << std::endl;
                ++results.passed;
                }
            catch(test_skipped_exception const& e)
                {
                std::cout
                    << name
                    << ": skipped ("
                    << e.what()
                    << ")"
                    << std::endl
                    ;
                ++results.skipped;
                }
            catch(std::exception const& e)
                {
                error = e.what();
                }
            catch(...)
                {
                error = "unknown exception";
                }

            if(!error.empty())
                {
                ++results.failed;

                // Keep everything on a single line to ensure the full text of
                // the error appears if the output is filtered by the test name
                // using standard line-oriented tools such as grep.
                wxString one_line_error(error);
                one_line_error.Replace("\n", " ");

                std::cout
                    << name
                    << ": ERROR ("
                    << one_line_error
                    << ")"
                    << std::endl
                    ;
                }
            }
        }

    exit_code_ = results.failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

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

    for(auto const& i : tests_)
        {
        std::cout << '\t' << i.get_name() << '\n';
        }

    std::cout << tests_.size() << " test cases.\n";
}

} // Unnamed namespace.

wx_base_test_case::wx_base_test_case(char const* name)
    :name_ {name}
{
    application_test::instance().add_test(this);
}

void wx_base_test_case::skip_if_not_supported(char const* file)
{
    wxString const p(file);
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
    wxWindow* const w = dialog.FindWindow(name);
    LMI_ASSERT_WITH_MSG(w, "window named \"" << name << "\" not found");

    // Then find the book control containing it by walking up the window chain
    // until we reach it.
    for(wxWindow* maybe_page = w;;)
        {
        LMI_ASSERT(maybe_page);
        wxWindow* const maybe_book = maybe_page->GetParent();

        // As we know that w is a descendant of the dialog, this check ensures
        // that the loop terminates as sooner or later we must reach the dialog
        // by walking up the parent chain.
        LMI_ASSERT_WITH_MSG
            (maybe_book != &dialog
            ,"book control containing window \"" << name << "\" not found"
            );

        if(wxBookCtrlBase* const book = dynamic_cast<wxBookCtrlBase*>(maybe_book))
            {
            // We found the notebook, now we can use it to make the page
            // containing the target window current.
            for(int n = 0; n < bourn_cast<int>(book->GetPageCount()); ++n)
                {
                if(book->GetPage(n) == maybe_page)
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
class SkeletonTest final : public Skeleton
{
  public:
    SkeletonTest()
        :is_running_tests_ {false}
    {
    }

  protected:
    // Override base class virtual function.
    DocManagerEx* CreateDocManager() override;

    // wxApp overrides.
    bool OnInit                 () override;
    int  OnRun                  () override;
    bool OnExceptionInMainLoop  () override;
    bool StoreCurrentException  () override;
    void RethrowStoredException () override;
    void OnAssertFailure
        (wxChar const* file
        ,int line
        ,wxChar const* func
        ,wxChar const* cond
        ,wxChar const* msg
        ) override;

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
        void FileHistoryLoad(wxConfigBase const&) override
            {
            // We could call the base class function here, but it doesn't seem
            // useful to do it and doing nothing here makes it more symmetric
            // with FileHistorySave().
            }

        void FileHistorySave(wxConfigBase&) override
            {
            // Do not save the history to persistent storage: we don't want the
            // files opened during testing replace the files actually opened by
            // the user interactively.
            }
    };

    // As in the base class version, notice that we must not use 'new(wx)' here
    // as this object is deleted by wxWidgets itself.
    return ::new DocManagerTest;
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

int SkeletonTest::OnRun()
{
    int exit_code = Skeleton::OnRun();

    // If the application exited successfully then return the exit code
    // based on test results.
    if(exit_code == 0)
        {
        exit_code = application_test::instance().get_exit_code();
        }

    return exit_code;
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
    catch(std::runtime_error const& e)
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
    if(is_running_tests_)
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
    if(is_running_tests_ && !std::uncaught_exceptions())
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
    wxWindow* const MainWin = GetTopWindow();
    if(!MainWin)
        {
        wxLogError("Failed to find the application main window.");
        ExitMainLoop();
        return;
        }

    // Whatever happens, ensure that the main window is closed and thus the
    // main loop terminated and the application exits at the end of the tests.
    class ensure_top_window_closed
    {
      public:
        explicit ensure_top_window_closed(wxApp* app)
            :app_ {app}
        {
        }

        ensure_top_window_closed(ensure_top_window_closed const&) = delete;
        ensure_top_window_closed& operator=(ensure_top_window_closed const&) = delete;

        ~ensure_top_window_closed()
        {
        wxWindow* const top = app_->GetTopWindow();
        if(top)
            {
            top->Close(true); // force close
            }
        }

      private:
        wxApp const* const app_;
    } close_top_window_on_scope_exit(this);

    // Close any initially opened dialogs (e.g. "About" dialog shown unless a
    // special command line option is specified).
    for(;;)
        {
        wxWindow* const ActiveWin = wxGetActiveWindow();
        if(!ActiveWin || ActiveWin == MainWin)
            break;

        // Try to close the dialog.
        wxUIActionSimulator ui;
        ui.Char(WXK_ESCAPE);
        wxYield();

        // But stop trying if it didn't work.
        if(wxGetActiveWindow() == ActiveWin)
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

    MainWin->SetFocus();

    std::cout << "NOTE: starting the test suite" << std::endl;
    wxStopWatch sw;

    // Notice that it is safe to use simple variable assignment here instead of
    // some RAII-based pattern because of application_test::run() noexcept
    // guarantee.
    is_running_tests_ = true;
    TestsResults const results = application_test::instance().run();
    is_running_tests_ = false;

    std::cout << "time=" << sw.Time() << "ms (for all tests)" << std::endl;

    if(results.failed == 0)
        {
        if(results.passed == 0)
            {
            std::cout << "WARNING: no tests have been executed." << std::endl;
            }
        else
            {
            std::cout
                << "SUCCESS: "
                << results.passed
                << " test"
                << (results.passed == 1 ? "" : "s")
                << " successfully completed."
                << std::endl
                ;
            }
        }
    else
        {
        std::cout
            << "FAILURE: "
            << results.failed
            << " out of "
            << results.total
            << " test"
            << (results.total == 1 ? "" : "s")
            << " failed."
            << std::endl
            ;
        }

    if(results.skipped)
        {
        std::cout
            << "NOTE: "
            << results.skipped
            << (results.skipped == 1 ? " test was" : " tests were")
            << " skipped"
            << std::endl
            ;
        }
}

/// Run automated GUI test.
///
/// Perform only the minimum necessary initialization that the lmi_wx
/// main() function would do; then preprocess the command line, before
/// calling wxEntry(), to handle and remove any GUI-test-specific
/// options (which Skeleton::ProcessCommandLine() must not see).
///
/// Don't call PreloadDesignatedDlls() here. Skeleton::OnInit() must
/// do that, because PreloadDesignatedDlls() instantiates class
/// configurable_settings, which must not be instantiated before
/// Skeleton::ProcessCommandLine() reads the '--data_path' option.

int main(int argc, char* argv[])
{
    initialize_application();

    if(!application_test::instance().process_command_line(argc, argv))
        {
        return 0;
        }

    return wxEntry(argc, argv);
}
