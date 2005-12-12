// Startup code common to all interfaces except wx.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: main_common_non_wx.cpp,v 1.3 2005-12-12 17:57:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "main_common.hpp"

#include "alert.hpp" // report_exception()
#include "fenv_lmi.hpp"

#include <iostream>
#include <ostream>

/// Interfaces that always use a normal main() function share this
/// implementation, which performs common initialization and catches
/// stray exceptions to prevent them from causing drastic termination.
///
/// Exception: for msw at least, wx doesn't use main(). The way
/// diagnostic messages are displayed for wx is different enough to
/// warrant a parallel implementation.

//============================================================================
int main(int argc, char* argv[])
{
    int result = EXIT_FAILURE;
    try
        {
        initialize_application();
        result = try_main(argc, argv);
        // TODO ?? It would be better to do this in a singleton's dtor.
        validate_fenv();
        }
    catch(...)
        {
        report_exception();
        // Return a failure code explicitly because validate_fenv()
        // might have found a problem even after 'result' was
        // assigned a value indicating success.
        result = EXIT_FAILURE;
        }

    // COMPILER !! MinGW doesn't reliably flush streams on exit, so
    // flush them explicitly. Do this outside the try block because
    // there's no way to recover if explicit flushing fails.
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;

    return result;
}

