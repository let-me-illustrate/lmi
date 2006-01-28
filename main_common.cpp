// Startup code common to all interfaces.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: main_common.cpp,v 1.6 2006-01-28 14:46:02 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "main_common.hpp"

#include "fenv_lmi.hpp"
#include "handle_exceptions.hpp"
#include "sigfpe.hpp"

#include <csignal>
#include <exception> // std::set_terminate()
#include <stdexcept>

/// Common application initialization.
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

