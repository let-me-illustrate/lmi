// Test calculation summary.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "ssize_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_utility.hpp"

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/crt.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/html/htmlpars.h>
#include <wx/html/htmlwin.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cerrno>                       // errno
#include <cstddef>                      // size_t, byte
#include <cstdio>                       // remove()
#include <cstring>                      // strerror()
#include <memory>                       // unique_ptr
#include <new>                          // nothrow

namespace
{

struct name_and_title
{
    char const* name;
    char const* title;
};

// Names and titles of the columns used by default.
name_and_title const default_columns_info[] =
    {{ "PolicyYear"             , "Policy Year"                 }
    ,{ "Outlay"                 , "Net Outlay"                  }
    ,{ "AcctVal_Current"        , "Curr Account Value"          }
    ,{ "CSVNet_Current"         , "Curr Net Cash Surr Value"    }
    ,{ "EOYDeathBft_Current"    , "Curr EOY Death Benefit"      }
    };

int const number_of_default_columns = lmi::ssize(default_columns_info);

// Names and titles of the columns used when not using the built-in calculation
// summary.
name_and_title const custom_columns_info[] =
    {{ "PolicyYear"             , "Policy Year"                 }
    ,{ "NewCashLoan"            , "Annual Loan"                 }
    };

int const number_of_custom_columns = lmi::ssize(custom_columns_info);

// Special name used when the column is not used at all. This is the same
// string used in preferences_model.cpp, but we duplicate it here as we don't
// have access to it.
char const* const empty_column_name = "[none]";

// Total number of configurable summary columns. This, again, duplicates the
// number [implicitly] used in preferences_model.cpp.
int const total_number_of_columns = 12;

// Base class for all the tests working with the preferences dialog. It
// defines both a simpler interface for the derived classes to define the
// tests with the preferences dialog, and also provides a helper run() method
// which shows the preferences dialog and performs these checks.
class expect_preferences_dialog_base
    :public wxExpectModalBase<MvcController>
{
  public:
    expect_preferences_dialog_base()
        :dialog_(nullptr)
        ,use_checkbox_(nullptr)
        {
        }

    expect_preferences_dialog_base(expect_preferences_dialog_base const&) = delete;
    expect_preferences_dialog_base& operator=(expect_preferences_dialog_base const&) = delete;

    void run() const
        {
        wxUIActionSimulator ui;
        ui.Char('f', wxMOD_CONTROL);    // "File|Preferences"

        wxTEST_DIALOG(wxYield(), *this);
        }

    int OnInvoked(MvcController* dialog) const override
        {
        // OnInvoked() is const but it doesn't make much sense for
        // OnPreferencesInvoked() to be const as it is going to modify the
        // dialog, so cast away this constness once and for all.
        expect_preferences_dialog_base* const
            self = const_cast<expect_preferences_dialog_base*>(this);

        self->dialog_ = dialog;

        dialog->Show();
        wxYield();

        wxWindow* const use_window = wx_test_focus_controller_child
            (*dialog
            ,"UseBuiltinCalculationSummary"
            );

        self->use_checkbox_ = dynamic_cast<wxCheckBox*>(use_window);
        LMI_ASSERT(use_checkbox_);

        return self->OnPreferencesInvoked();
        }

    wxString GetDefaultDescription() const override
        {
        return "preferences dialog";
        }

  protected:
    virtual int OnPreferencesInvoked() = 0;

    // Helpers for the derived classes OnPreferencesInvoked().
    void set_use_builtin_summary(bool use)
        {
        // Under MSW we could use "+" and "-" keys to set the checkbox value
        // unconditionally, but these keys don't work under the other
        // platforms, so it's simpler to use the space key which can be used on
        // all platforms to toggle the checkbox -- but then we must do it only
        // if really needed.
        if(use_checkbox_->GetValue() != use)
            {
            use_checkbox_->SetFocus();

            wxUIActionSimulator ui;
            ui.Char(WXK_SPACE);
            wxYield();
            }
        }

    wxComboBox* focus_column_combobox(int n)
        {
            wxWindow* const column_window = wx_test_focus_controller_child
                (*dialog_
                ,wxString::Format("CalculationSummaryColumn%02u", n).c_str()
                );

            wxComboBox* const
                column_combobox = dynamic_cast<wxComboBox*>(column_window);
            LMI_ASSERT_WITH_MSG
                (column_combobox
                ,"control for column #" << n << "is not a wxComboBox"
                );

            return column_combobox;
        }

    // These variables are only valid inside the overridden
    // OnPreferencesInvoked() method.
    MvcController* dialog_;
    wxCheckBox* use_checkbox_;
};

void check_calculation_summary_columns
    (std::size_t number_of_columns
    ,name_and_title const columns_info[]
    )
{
    // Create a new illustration.
    wx_test_new_illustration ill;

    // Find the window displaying HTML contents of the illustration view.
    wxWindow* const focus = wxWindow::FindFocus();
    LMI_ASSERT(focus);

    wxHtmlWindow* const htmlwin = dynamic_cast<wxHtmlWindow*>(focus);
    LMI_ASSERT(htmlwin);

    // And get the HTML from it.
    wxHtmlParser* const parser = htmlwin->GetParser();
    LMI_ASSERT(parser);
    LMI_ASSERT(parser->GetSource());

    wxString const html = *parser->GetSource();

    // We don't need the window any more.
    ill.close();

    // Find the start of the table after the separating line.
    std::size_t pos = html.find("<hr>\n<table");
    LMI_ASSERT(pos != wxString::npos);

    pos = html.find("\n<td", pos);
    LMI_ASSERT(pos != wxString::npos);

    ++pos;                                          // skip the new line

    // We have found the place where the columns are described in the HTML,
    // iterate over all of them.
    for(std::size_t n = 0; n < number_of_columns; ++n)
        {
        LMI_ASSERT_EQUAL(wxString(html, pos, 3), "<td");

        pos = html.find(">", pos);                     // end of the <td> tag
        LMI_ASSERT(pos != wxString::npos);

        ++pos;                                         // <td> tag contents

        std::size_t const next = html.find("\n", pos); // the next line start
        LMI_ASSERT(next != wxString::npos);

        // Extract the column title from the rest of the line.
        wxString title;
        LMI_ASSERT(wxString(html, pos, next - pos).EndsWith(" </td>", &title));

        LMI_ASSERT_EQUAL(title, columns_info[n].title);

        pos = next + 1;
        }

    LMI_ASSERT_EQUAL(wxString(html, pos, 5), "</tr>");
}

/// Helper class preserving the contents of the given file by making a copy of
/// it in a temporary file in its ctor and restoring this copy in its dtor.

class file_contents_preserver
{
  public:
    explicit file_contents_preserver(std::string const& path)
    {
        // This outer try block is used to give a better error message in case
        // of failure by combining the top-level error from the catch clause
        // below with the more detailed error message thrown as wxString object
        // from inside it and also to delete the temporary file, as it won't be
        // done by the dtor if the ctor throws.
        try
            {
            temp_.path_ = wxFileName::CreateTempFileName
                            ("lmi_wx_test"
                            ,&temp_.file_
                            );
            if(temp_.path_.empty())
                {
                throw wxString{"failed to create temporary file"};
                }

            orig_.path_ = wxString::FromUTF8(path);
            if(orig_.path_.empty())
                {
                // This can happen either because an empty path was passed in,
                // which is just a programming error, or because conversion
                // from UTF-8 failed, which shouldn't happen for any ASCII file
                // names used by lmi, but still needs to be reported if it
                // does.
                if(path.empty())
                    {
                    throw wxString{"file path cannot be empty"};
                    }
                else
                    {
                    throw wxString::Format
                            ("file path \"%s\" is not a valid UTF-8 string"
                            ,wxString::From8BitData(path.c_str())
                            );
                    }
                }

            // Note that while temp_.file_ can (and probably should, precisely
            // in order to prevent it from being modified) be kept opened
            // during this entire object lifetime, we can't keep the original
            // file opened, as this would prevent the code using this class
            // from modifying it, so we need to open -- and close! -- it both
            // here and in the dtor.
            if(!orig_.file_.Open(orig_.path_))
                {
                throw wxString::Format
                        ("failed to open file \"%s\" for reading (%s)"
                        ,orig_.path_
                        ,std::strerror(orig_.file_.GetLastError())
                        );
                }

            if(auto const error = copy_contents(orig_, temp_); !error.empty())
                {
                throw error;
                }

            if(!orig_.file_.Close())
                {
                throw wxString::Format
                        ("unexpectedly failed to close \"%s\" (%s)"
                        ,orig_.path_
                        ,std::strerror(orig_.file_.GetLastError())
                        );
                }

            // Finally, rewind the temporary file, so that we could read from
            // it in the dtor.
            if(temp_.file_.Seek(0) == wxInvalidOffset)
                {
                throw wxString::Format
                        ("failed to rewind temporary file \"%s\" (%s)"
                        ,temp_.path_
                        ,std::strerror(temp_.file_.GetLastError())
                        );
                }
            }
        catch(wxString error)
            {
            if(!temp_.path_.empty())
                {
                if(auto const error_rm = remove_temp_file(); !error_rm.empty())
                    {
                    error += wxString::Format(" (additionally, %s)", error_rm);
                    }
                }

            throw std::runtime_error
                    {wxString::Format
                        ("Error preserving contents of the file \"%s\": %s."
                        ,orig_.path_
                        ,error
                        ).ToStdString()
                    };
            }
    }

    ~file_contents_preserver()
    {
        try
            {
            if(!orig_.file_.Open(orig_.path_, wxFile::write))
                {
                throw wxString::Format
                        ("failed to open file \"%s\" for writing (%s)"
                        ,orig_.path_
                        ,std::strerror(orig_.file_.GetLastError())
                        );
                }

            if(auto const error = copy_contents(temp_, orig_); !error.empty())
                {
                throw error;
                }

            if(auto const error_rm = remove_temp_file(); !error_rm.empty())
                {
                // This doesn't merit throwing in any case, and see also the
                // comment below.
                wxPrintf("WARNING: %s\n", error_rm);
                }
            }
        catch(wxString const& error)
            {
            // We don't want to throw from a dtor: even though it could be done
            // safely when not unwinding, there would still be a problem of not
            // being able to do it if the test had actually failed and so this
            // dtor runs as part of stack unwinding. To keep things consistent,
            // just log a message, as this is something we can do in any case.
            wxPrintf
                ("WARNING: error restoring contents of the file \"%s\": %s.\n"
                 "Its contents was possibly modified, original contents "
                 "preserved in \"%s\".\n"
                ,orig_.path_
                ,error
                ,temp_.path_
                );

            // Do not close the temporary file here.
            }
    }

    file_contents_preserver(file_contents_preserver const&) = delete;
    file_contents_preserver& operator=(file_contents_preserver const&) = delete;

  private:
    // Simple struct containing a file together with its full path, which is
    // useful for generating more informative error messages.
    struct named_file
    {
        wxFile file_;
        wxString path_;
    };

    wxString remove_temp_file() noexcept
    {
        temp_.file_.Close();
        if(wxRemove(temp_.path_) != 0)
            {
            return wxString::Format
                    ("temporary file \"%s\" couldn't be removed: %s"
                     ,temp_.path_
                     ,std::strerror(errno)
                    );
            }

        return wxString{};
    }

    // Copies contents of one open file to another one, returns empty string on
    // success or the error message on failure.
    wxString copy_contents(named_file& src, named_file& dst) noexcept
    {
        auto const length = src.file_.Length();
        if(length == wxInvalidOffset)
            {
            return wxString::Format
                    ("failed to get the length of the file \"%s\" (%s)"
                    ,src.path_
                    ,std::strerror(src.file_.GetLastError())
                    );
            }

        // Don't throw if allocation fails, this function is noexcept.
        auto const buf = new(std::nothrow) std::byte[length];
        if(!buf)
            {
            return wxString::Format
                    ("failed to allocate %llu bytes of memory", length
                    );
            }

        std::unique_ptr<std::byte[]> buf_ptr{buf}; // Ensure it is freed.

        // Doing a single read and write is not the best way of copying huge
        // files (we could run out of memory) and not the most generic way
        // neither (it would fail for not seekable pipes), but still should be
        // good enough for any files we need to work with here. And it's by far
        // the simplest.
        if(src.file_.Read(buf, bourn_cast<std::size_t>(length)) != length)
            {
            return wxString::Format
                    ("failed to read %llu bytes from \"%s\" (%s)"
                     ,length
                     ,src.path_
                     ,std::strerror(src.file_.GetLastError())
                    );
            }

        if(dst.file_.Write(buf, bourn_cast<std::size_t>(length)) != length)
            {
            return wxString::Format
                    ("failed to write %llu bytes to \"%s\" (%s)"
                    ,length
                    ,dst.path_
                    ,std::strerror(dst.file_.GetLastError())
                    );
            }

        if(!dst.file_.Flush())
            {
            return wxString::Format
                    ("failed to flush file \"%s\" (%s)"
                    ,dst.path_
                    ,std::strerror(dst.file_.GetLastError())
                    );
            }

        return wxString{};
    }

    named_file temp_;
    named_file orig_;
};

} // Unnamed namespace.

// Deferred ideas:
//
// Someday, test supplemental-report column selections similarly.
//
// To test backward compatibility, modify 'configurable_settings'
// directly, adding a field that was formerly removed, and setting
// the version number to a version that offered that field.
//
// Columns whose names end with "Zero" are available iff inforce
// general and separate account value are both nonzero. This could be
// tested here; however, it would be a vastly better use of limited
// time to generate the special report that uses them automatically
// rather than manually, and then to expunge those columns from
// 'mc_enum_types.?pp'.

/// Test calculation summary.
///
/// Iff the '--distribution' option is specified, then:
///   File | Preferences
/// make sure that "Use built-in calculation summary" is checked, and
/// that the saved selections (those that would become active if the
/// checkbox were unchecked) exactly match the default selections
/// given by default_calculation_summary_columns().
///
/// Display an illustration, to see calculation-summary effects:
/// File | New | Illustration | OK
///
/// File | Preferences
/// uncheck "Use built-in calculation summary"
/// set all "Column" controls to "[none]"
/// in "Column 2" (two, not zero), select "NewCashLoan"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Annual Loan
///
/// File | Preferences
/// Verify that "NewCashLoan" has moved from "Column 2" to "Column 0"
/// check "Use built-in calculation summary"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Net Outlay
///   Curr Account Value
///   Curr Net Cash Surr Value
///   Curr EOY Death Benefit
///
/// File | Preferences
/// uncheck "Use built-in calculation summary"
/// Verify that "Column 0" is "NewCashLoan" and the rest are "[none]"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Annual Loan

LMI_WX_TEST_CASE(calculation_summary)
{
    // This test modifies the program settings in the global configuration
    // file. Ensure that these modifications are only temporary by making a
    // copy of the original file and restoring it when the test ends, either
    // successfully or due to an error.
    file_contents_preserver config_preserver{configuration_filepath()};

    if(is_distribution_test())
        {
        // Not only is this the expected value in the GUI, but we also want to be
        // sure that effective_calculation_summary_columns() returns the default
        // columns in the code below -- and this is only the case when we are using
        // the built-in calculation summary.
        LMI_ASSERT
            (configurable_settings::instance().use_builtin_calculation_summary()
            );

        struct verify_builtin_calculation_summary : expect_preferences_dialog_base
        {
            int OnPreferencesInvoked() override
                {
                LMI_ASSERT_EQUAL(use_checkbox_->GetValue(), true);

                std::vector<std::string> const&
                    summary_columns = effective_calculation_summary_columns();

                for(int n = 0; n < number_of_custom_columns; ++n)
                    {
                    wxString const& column = focus_column_combobox(n)->GetValue();
                    if(n < lmi::ssize(summary_columns))
                        {
                        LMI_ASSERT_EQUAL(column, summary_columns[n]);
                        }
                    else
                        {
                        LMI_ASSERT_EQUAL(column, empty_column_name);
                        }
                    }

                return wxID_CANCEL;
                }
        };

        verify_builtin_calculation_summary().run();
        }

    wx_test_new_illustration ill;

    // Use a single "NewCashLoan" custom column in third position.
    struct set_custom_columns_in_preferences_dialog : expect_preferences_dialog_base
    {
        int OnPreferencesInvoked() override
            {
            set_use_builtin_summary(false);

            wxUIActionSimulator ui;
            for(int n = 0; n < total_number_of_columns; ++n)
                {
                focus_column_combobox(n);
                ui.Select(n == 2 ? "NewCashLoan" : empty_column_name);
                }

            return wxID_OK;
            }
    };

    set_custom_columns_in_preferences_dialog().run();

    check_calculation_summary_columns
        (number_of_custom_columns
        ,custom_columns_info
        );

    // Now switch to using the default columns.
    struct use_builtin_calculation_summary : expect_preferences_dialog_base
    {
        int OnPreferencesInvoked() override
            {
            // Before returning to the built-in summary, check that our custom
            // value for the column #2 moved into the position #0 (because the
            // first two columns were left unspecified).
            LMI_ASSERT_EQUAL
                (focus_column_combobox(0)->GetValue()
                ,"NewCashLoan"
                );

            // And all the rest of the columns are (still) empty.
            for(int n = 1; n < total_number_of_columns; ++n)
                {
                LMI_ASSERT_EQUAL
                    (focus_column_combobox(n)->GetValue()
                    ,empty_column_name
                    );
                }

            set_use_builtin_summary(true);

            return wxID_OK;
            }
    };

    use_builtin_calculation_summary().run();

    check_calculation_summary_columns
        (number_of_default_columns
        ,default_columns_info
        );

    // Finally, switch back to the previously configured custom columns.
    struct use_custom_calculation_summary : expect_preferences_dialog_base
    {
      public:
        int OnPreferencesInvoked() override
            {
            set_use_builtin_summary(false);

            // The custom columns shouldn't have changed.
            LMI_ASSERT_EQUAL
                (focus_column_combobox(0)->GetValue()
                ,"NewCashLoan"
                );

            for(int n = 1; n < total_number_of_columns; ++n)
                {
                LMI_ASSERT_EQUAL
                    (focus_column_combobox(n)->GetValue()
                    ,empty_column_name
                    );
                }

            return wxID_OK;
            }
    };

    use_custom_calculation_summary().run();

    check_calculation_summary_columns
        (number_of_custom_columns
        ,custom_columns_info
        );

    ill.close();
}
