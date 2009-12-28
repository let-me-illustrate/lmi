// Startup code common to all interfaces except wx.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: main_common_non_wx.cpp,v 1.13 2008-12-27 02:56:47 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "main_common.hpp"

#include "fenv_lmi.hpp"
#include "handle_exceptions.hpp"

#include <iostream>
#include <ostream>

/// Common initialization for all non-wx applications.
///
/// Interfaces that always use a normal main() function share this
/// implementation, which performs common initialization and catches
/// stray exceptions to prevent them from causing drastic termination.
///
/// Exception: for msw at least, wx doesn't use main(). The way
/// diagnostic messages are displayed for wx is different enough to
/// warrant a parallel implementation.
///
/// Don't initialize boost::filesystem here, to avoid creating a
/// dependency on its object files for applications that don't use it.

int main(int argc, char* argv[])
{
    int result = EXIT_FAILURE;
    try
        {
        initialize_application();
        result = try_main(argc, argv);
        }
    catch(...)
        {
        report_exception();
        }

    fenv_validate();

    // COMPILER !! MinGW doesn't reliably flush streams on exit, so
    // flush them explicitly. Do this outside the try block because
    // there's no way to recover if explicit flushing fails.
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;

    return result;
}

