// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#include "pchfile_wx.hpp"

#include "alert.hpp"                    // safely_show_message()
#include "fenv_lmi.hpp"
#include "force_linking.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "main_common.hpp"              // initialize_application()
#include "skeleton.hpp"

#include <wx/init.h>                    // wxEntry()

#include <string>

LMI_FORCE_LINKING_EX_SITU(alert_wx)
LMI_FORCE_LINKING_EX_SITU(file_command_wx)
LMI_FORCE_LINKING_EX_SITU(group_quote_pdf_generator_wx)
LMI_FORCE_LINKING_EX_SITU(pdf_command_wx)
LMI_FORCE_LINKING_EX_SITU(progress_meter_wx)
LMI_FORCE_LINKING_EX_SITU(system_command_wx)

IMPLEMENT_APP_NO_MAIN(Skeleton)
IMPLEMENT_WX_THEME_SUPPORT

#if !defined LMI_MSW
int main(int argc, char* argv[])
#else  // defined LMI_MSW
int WINAPI WinMain
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     lpCmdLine
    ,int       nCmdShow
    )
#endif // defined LMI_MSW
{
    // (Historical notes.) Using wx-2.5.1 and mpatrol-1.4.8, both
    // dynamically linked to this application built with gcc-3.2.3,
    // three memory leaks are reported with:
    //   MPATROL_OPTIONS='SHOWUNFREED'
    // It's easier to trace them with:
    //   MPATROL_OPTIONS='LOGALL SHOWUNFREED USEDEBUG'
    // Two are apparently mpatrol artifacts traceable to symbols:
    // [space follows leading underscores in reserved names]
    //   "_ _ _ mp_findsource"
    //   "_ _ _ mp_init"
    // The third is traceable in 'mpatrol.log' with 'USEDEBUG' to
    //   Skeleton::GetEventHashTable() const
    // (although stepping through the code in gdb suggests it's really
    // WinMain(), and mpatrol or libbfd just got the symbol wrong)
    // and seems to be triggered the first time the program allocates
    // memory. The next line forces that to occur here; otherwise,
    // tracing this 'leak' becomes cumbersome and mysterious.
    std::string unused("Seems to trigger initialization of something.");

    int result = EXIT_FAILURE;

    try
        {
        initialize_application();
#if !defined LMI_MSW
        result = wxEntry(argc, argv);
#else  // defined LMI_MSW
        result = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#endif // defined LMI_MSW
        }
    catch(...)
        {
        try
            {
            report_exception();
            }
        catch(...)
            {
            safely_show_message("Logic error: untrapped exception.");
            }
        }

    fenv_validate();

    return result;
}
