// Startup code common to all interfaces.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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
#endif // __BORLANDC__

#include "main_common.hpp"

#include "fenv_lmi.hpp"
#include "handle_exceptions.hpp"
#include "sigfpe.hpp"

#include <csignal>
#include <exception> // std::set_terminate()
#include <stdexcept>

#if defined __MINGW32__
/// Conform to C99 [7.19.6.1/8]:
///   "The exponent always contains at least two digits, and only as
///   many more digits as necessary to represent the exponent."
/// See:
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/28747

extern "C" int _get_output_format(void) {return 1;}
#endif // defined __MINGW32__

/// Common application initialization.
///
/// Also see the similar code in 'cpp_main.cpp' (for unit tests).
///
/// Don't initialize boost::filesystem here, to avoid creating a
/// dependency on its object files for applications that don't use it.

void initialize_application()
{
    std::set_terminate(lmi_terminate_handler);
    try
        {
        // This line forces mpatrol to link when it otherwise might not.
        // It has no other effect according to C99 7.20.3.2/2, second
        // sentence.
        std::free(0);

        fenv_initialize();

        if(SIG_ERR == std::signal(SIGFPE, floating_point_error_handler))
            {
            throw std::runtime_error
                ("Cannot install floating point error signal handler."
                );
            }
        }
    catch(...)
        {
        report_exception();
        }
}

