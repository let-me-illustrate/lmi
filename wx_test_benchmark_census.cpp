// Measure the speed of various operations on certain census files.
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
#include "path.hpp"
#include "wx_test_case.hpp"
#include "wx_test_statusbar.hpp"

#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/scopeguard.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cmath>                        // fabs()
#include <iostream>

namespace
{

class census_benchmark
{
  public:
    explicit census_benchmark(fs::path const& path)
        :status_ {get_main_window_statusbar()}
        ,name_   {path.leaf()}
        {
        wxUIActionSimulator z;
        z.Char('o', wxMOD_CONTROL); // "File|Open"
        wxTEST_DIALOG
            (wxYield()
            ,wxExpectModal<wxFileDialog>(path.native_file_string())
            );
        wxYield();
        }

    ~census_benchmark()
        {
        // Close the census window opened in the ctor itself.
        close_window();
        }

    void time_operation
        (char const* operation
        ,char key
        ,int mod
        )
        {
        // Clear any status text left over from the previous run.
        status_.SetStatusText(wxString());

        wxUIActionSimulator z;
        z.Char(key, mod);
        wxYield();

        wxString const status_text = status_.GetStatusText();
        wxString ms_text;
        LMI_ASSERT(status_text.EndsWith(" milliseconds", &ms_text));

        long int time_real;
        LMI_ASSERT(ms_text.ToLong(&time_real));

        std::cout
            << operation
            << " for "
            << name_
            << ": "
            << time_real
            << "ms elapsed"
            << std::endl
            ;
        }

    void close_window()
        {
        wxUIActionSimulator z;
        z.Char('l', wxMOD_CONTROL); // "File|Close"
        }

  private:
    census_benchmark(census_benchmark const&) = delete;
    census_benchmark& operator=(census_benchmark const&) = delete;

    wxStatusBar& status_;
    wxString const name_;
};

} // Unnamed namespace.

/// Measure the speed of various operations on certain census files.
///
/// Comparing the results of this test to a stored touchstone makes it
/// easy to see speed changes, and hence to guard against performance
/// regressions that might otherwise escape timely notice.
///
/// Write timing data to stdout. We had considered storing touchstone
/// timings in a configuration file and calculating relative error
/// here, but found that it's simpler just to print the timings and
/// compare to the results of prior runs.
///
/// These operations are measured because they are the most important:
///   Census | Run case
///   Census | Print case to PDF
///   Census | Print case to spreadsheet
/// We had considered running at least the "Run case" test several
/// times, discarding the first run and reporting an average (probably
/// the mode) of the others; that can be done at a later date if
/// experience demonstrates that it would be useful.
///
/// This test uses all files matching "gui_test_path/MSEC*.cns", which
/// may include proprietary products and should be designed to cover
/// different paths through the code. We had considered specifying the
/// input files in a configuration file, but the chosen way is simpler
/// and makes it even easier to change to change the input set.

LMI_WX_TEST_CASE(benchmark_census)
{
    fs::directory_iterator const end_i;
    for(fs::directory_iterator i(get_test_files_path()); i != end_i; ++i)
        {
        if(!wxString(i->leaf()).Matches("MSEC*.cns"))
            {
            continue;
            }

        census_benchmark b(*i);

        {
        // Ensure that the window doesn't stay opened (and possibly affects
        // negatively the subsequent tests) even if this test fails.
        wxON_BLOCK_EXIT_OBJ0(b, census_benchmark::close_window);

        b.time_operation
            ("Run case"
            ,'r'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );
        }

        b.time_operation
            ("Print case to PDF"
            ,'i'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );

        b.time_operation
            ("Print case to spreadsheet"
            ,'h'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );
        }
}
