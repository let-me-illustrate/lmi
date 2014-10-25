// Test benchmarking census operations.
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
#include "wx_test_case.hpp"
#include "wx_test_statusbar.hpp"
#include "uncopyable_lmi.hpp"

#include <wx/confbase.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/log.h>
#include <wx/scopeguard.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cmath>                    // std::fabs()

namespace
{

class census_benchmark
    :private lmi::uncopyable<census_benchmark>
{
  public:
    explicit census_benchmark(wxString const& name, wxString const& path)
        :status_(get_main_window_statusbar())
        ,name_(name)
        {
        wxUIActionSimulator z;
        z.Char('o', wxMOD_CONTROL); // "File|Open"
        wxTEST_DIALOG
            (wxYield()
            ,wxExpectModal<wxFileDialog>(path)
            );
        wxYield();
        }

    void time_operation
        (char const* operation
        ,long time_expected
        ,char key
        ,int mod
        )
        {
        wxUIActionSimulator z;
        z.Char(key, mod);
        wxYield();

        wxString const status_text = status_.GetStatusText();
        wxString ms_text;
        LMI_ASSERT(status_text.EndsWith(" milliseconds", &ms_text));

        long time_real;
        LMI_ASSERT(ms_text.ToLong(&time_real));

        // Compare the difference with the expected time if it's specified.
        wxString delta;
        if (time_expected)
            {
            double const diff_in_percents =
                100*(time_real - time_expected)
                    / static_cast<double>(time_expected);

            delta.Printf("%+.2f%%", diff_in_percents);

            LMI_ASSERT_WITH_MSG
                (std::fabs(diff_in_percents) < 10
                ,wxString::Format
                    (
                    "expected %ldms, but actually took %ldms, i.e. %s"
                    ,time_expected
                    ,time_real
                    ,delta
                    )
                );
            }
        else
            {
            delta = "not specified";
            }

        wxLogMessage
            ("%s for %s: %ldms elapsed (expected %s)"
            ,operation
            ,name_
            ,time_real
            ,delta
            );
        }

    void close_window()
        {
        wxUIActionSimulator z;
        z.Char('l', wxMOD_CONTROL); // "File|Close"
        }

    ~census_benchmark()
        {
        // Close the census window opened in the ctor itself.
        close_window();
        }

  private:
    wxStatusBar const& status_;
    wxString const name_;
};

} // Unnamed namespace.

LMI_WX_TEST_CASE(benchmark_census)
{
    wxConfigBase const& c = config();

    // Read the timing parameters.
    long const time_run = c.ReadLong("time_run", 0);
    long const time_disk = c.ReadLong("time_disk", 0);
    long const time_spreadsheet = c.ReadLong("time_spreadsheet", 0);

    // The censuses to benchmark are specified by the subgroups of the config
    // file, so iterate over all of them.
    wxString name;
    long z;
    for(bool ok = c.GetFirstGroup(name, z); ok; ok = c.GetNextGroup(name, z))
        {
        census_benchmark b(name, c.Read(name + "/path"));

        {
        // Ensure that the window doesn't stay opened (and possibly affects
        // negatively the subsequent tests) even if this test fails.
        wxON_BLOCK_EXIT_OBJ0(b, census_benchmark::close_window);

        b.time_operation
            ("Run case"
            ,time_run
            ,'r'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );
        }

        b.time_operation
            ("Print to disk"
            ,time_disk
            ,'k'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );

        b.time_operation
            ("Print to spreadsheet"
            ,time_spreadsheet
            ,'h'
            ,wxMOD_CONTROL | wxMOD_SHIFT
            );
        }
}
