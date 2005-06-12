// Startup code common to all interfaces.
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

// $Id: main_common.cpp,v 1.2 2005-06-12 16:58:36 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "main_common.hpp"

#include "fenv_lmi.hpp"
#include "sigfpe.hpp"

#include <csignal>
#include <stdexcept>

// TODO ?? Remove other uses of fenv_lmi, initialize_fpu()?

//============================================================================
void initialize_application()
{
    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(0);

    // TODO ?? Instead, consider a singleton that checks the control
    // word upon destruction, or a shared_ptr with a custom deleter
    // that does that.
    initialize_fpu();

    if(SIG_ERR == std::signal(SIGFPE, floating_point_error_handler))
        {
        throw std::runtime_error
            ("Cannot install floating point error signal handler."
            );
        }
}

