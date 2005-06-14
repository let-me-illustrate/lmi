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

// $Id: main_common_non_wx.cpp,v 1.2 2005-06-14 00:29:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "main_common.hpp"

#include "fenv_lmi.hpp"

#include <iostream>
#include <ostream>
#include <exception>

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
    int result = EXIT_SUCCESS;
    try
        {
        initialize_application();
        result = try_main(argc, argv);
        }
    catch(std::exception& e)
        {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        result = EXIT_FAILURE;
        }
    // TODO ?? Consider the other cases treated in 'catch_exceptions.cpp'.
    catch(...)
        {
        std::cerr << "Fatal exception: [no detail available]" << std::endl;
        result = EXIT_FAILURE;
        }

    // TODO ?? It would be better to do this in a singleton's dtor.
    validate_fenv();

    // COMPILER !! MinGW doesn't reliably flush streams on exit.
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;

    return result;
}

