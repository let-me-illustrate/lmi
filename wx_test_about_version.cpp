// Version number test case for the GUI test suite.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
#include "calendar_date.hpp"
#include "version.hpp"
#include "wx_test_case.hpp"

#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <climits>                      // INT_MAX
#include <iostream>
#include <regex>

/// Validate version string (timestamp) from "About" dialog title.
///
/// Test that the version string matches the timestamp specified in
/// 'version.hpp'. That's not a really useful test because it can
/// hardly fail, but it's so inexpensive that there's no reason to
/// delete it.
///
/// Write the version string to stdout (along with other test output)
/// for comparison with previously-saved results.
///
/// Find the last copyright year listed in the dialog's client area,
/// and compare it to the year in the version string, and also to the
/// current calendar year: it should match both. (This means that
/// running a 2014 GUI test in 2015 will fail; that's okay.) Monthly
/// releases sometimes update only the 'expiry' file, but reuse the
/// last month's binaries with an unchanged version string; therefore,
/// only the version string's year is compared to the current date,
/// whereas month, day, and time are not.
///
/// Press the pushbutton to read the license, and check that the
/// license's dialog box is scrollable--to guard against this problem:
///   https://lists.nongnu.org/archive/html/lmi/2010-01/msg00001.html

namespace
{

// Convert a string known to consist of just 4 digits to a number.
//
// This function contains LMI_ASSERT() checks but they should be never
// triggered if the preconditions are filled as any string of 4 digits can be
// converted to an int value.
int year_from_string(wxString const& s)
{
    unsigned long int year;

    LMI_ASSERT(s.ToCULong(&year));
    LMI_ASSERT(year < INT_MAX);

    return static_cast<int>(year);
}

// Find the last copyright year in the given HTML license notices text.
//
// May throw if the input doesn't conform to the expectations.
int extract_last_copyright_year(wxString const& html)
{
    // Uttering this word without obfuscation would confuse the
    // 'make happy_new_year' copyright-update recipe.
    static std::string const unutterable {"C""opyright"};

    // Find the line starting with that unutterable word.
    wxString copyright_line;
    for(auto const& line : wxSplit(html ,'\n' ,'\0'))
        {
        if(line.StartsWith(unutterable.c_str()))
            {
            LMI_ASSERT_WITH_MSG
                (copyright_line.empty()
                ,"Unexpectedly found more than one copyright line in the "
                 "license notices text"
                );

            copyright_line = line;
            }
        }

    LMI_ASSERT_WITH_MSG
        (!copyright_line.empty()
        ,unutterable + " line not found in the license notices text"
        );

    // We suppose that we have a sequence of comma-separated (4 digit, let
    // someone else worry about Y10K problem) years and so the year we are
    // interested in is just the last one of them.
    //
    // Notice also the use of utf8_str() to ensure that conversion from
    // wxString never fails (it could if the string contained non-ASCII
    // characters such as the copyright sign), while avoiding the use of wide
    // char std::regex functions. As we are only interested in matching
    // ASCII characters such as digits, using UTF-8 is safe even though
    // std::regex has no real support for it.
    std::string const copyright_line_utf8(copyright_line.utf8_str());
    std::smatch m;
    LMI_ASSERT_WITH_MSG
        (std::regex_search
            (copyright_line_utf8
            ,m
            ,std::regex("(?:\\d{4}, )+(\\d{4})")
            )
        , unutterable
        + " line '"
        + copyright_line
        + "' doesn't contain copyright years"
        );

    return year_from_string(wxString(m[1]));
}

// Find the only wxHtmlWindow inside the given dialog.
//
// Throws if there are none, or more than one, windows of wxHtmlWindow type in
// the dialog. The dialog name is only used for diagnostic purposes.
wxHtmlWindow* find_html_window(wxWindow* parent, std::string const& dialog_name)
{
    wxHtmlWindow* html_win = nullptr;
    for(auto const& w : parent->GetChildren())
        {
        wxHtmlWindow* const maybe_html_win = dynamic_cast<wxHtmlWindow*>(w);
        if(maybe_html_win)
            {
            LMI_ASSERT_WITH_MSG
                (!html_win
                ,"Unexpectedly found more than one wxHtmlWindow in "
                 "the " + dialog_name + " dialog"
                );

            html_win = maybe_html_win;
            }
        }

    LMI_ASSERT_WITH_MSG
        (html_win
        ,"wxHtmlWindow showing the license notices not found in "
         "the " + dialog_name + " dialog"
        );

    return html_win;
}

} // Unnamed namespace.

LMI_WX_TEST_CASE(about_dialog_version)
{
    struct expect_about_dialog : public wxExpectModalBase<wxDialog>
    {
        int OnInvoked(wxDialog* d) const override
            {
            LMI_ASSERT(nullptr != d);

            // Extract the last word of the dialog title.
            wxString const last_word = d->GetTitle().AfterLast(' ');
            std::cout
                << "About dialog version string is '"
                << last_word
                << "'."
                << std::endl
                ;
            LMI_ASSERT_EQUAL(last_word, LMI_VERSION);

            // Find the wxHtmlWindow showing the license notices.
            wxHtmlWindow* const
                license_notices_win = find_html_window(d, "about");

            // Check that the years in the copyright, license notices and
            // version string are all the same.
            int const copyright_year =
                extract_last_copyright_year(license_notices_win->ToText());

            LMI_ASSERT_EQUAL(copyright_year, today().year());

            int const version_year = year_from_string(wxString(LMI_VERSION, 4));
            LMI_ASSERT_EQUAL(version_year, copyright_year);

            // Finally bring up the dialog showing the license itself: for this
            // we first need to show this dialog itself.
            d->Show();
            wxYield();

            // And then press the default button in it which opens the license.
            struct expect_license_dialog : public wxExpectModalBase<wxDialog>
            {
                int OnInvoked(wxDialog* d) const override
                    {
                    wxHtmlWindow* const
                        license_win = find_html_window(d, "license");

                    // This is a rather indirect -- because testing this
                    // directly is not easily possible -- test of the scrollbar
                    // presence in the license window: we try to scroll it and
                    // expect it to have a result, as the license text is known
                    // to be long enough to not fit on a single page, even in
                    // high vertical resolutions.
                    //
                    // The first test just checks that the return value of
                    // LineXXX() functions makes sense: it should return false if
                    // no scrolling is possible. The second test checks that
                    // scrolling down does actually work.
                    LMI_ASSERT_WITH_MSG
                        (!license_win->LineUp()
                        ,"License window unexpectedly scrolled up"
                        );

                    LMI_ASSERT_WITH_MSG
                        (license_win->LineDown()
                        ,"License window didn't scroll down"
                        );

                    return wxID_OK;
                    }

                wxString GetDefaultDescription() const override
                    {
                    return "license dialog";
                    }
            };

            wxUIActionSimulator z;
            z.Char(WXK_RETURN);
            wxTEST_DIALOG
                (wxYield()
                ,expect_license_dialog()
                );

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "about dialog";
            }
    };

    wxUIActionSimulator z;
    z.Char('h', wxMOD_ALT);
    z.Char('a'           );
    wxTEST_DIALOG
        (wxYield()
        ,expect_about_dialog()
        );
}
