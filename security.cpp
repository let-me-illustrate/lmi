// Moderately secure system date validation--tells whether system has expired.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: security.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "security.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "global_settings.hpp"
#include "secure_date.hpp"

#include <cstdlib> // std::exit()
#include <sstream>
#include <string>

void validate_security(bool do_validate)
{
    if(!do_validate)
        {
        return;
        }

    int invalid = secure_date::instance()->validate
        (calendar_date()
        ,global_settings::instance().data_directory
        );

    // TODO ?? It would be better either to return a string from
    // validate(), or to let it throw an exception. In particular,
    // in the 'date_out_of_range' case it would be nice to specify
    // the current date and range.
    std::string problem;
    switch(invalid)
        {
        case 0:
            {
            // Do nothing.
            }
            break;
        case secure_date::ill_formed_passkey:
            {
            problem =
                "Passkey has unexpected length."
                " Try reinstalling."
                ;
            }
            break;
        case secure_date::date_out_of_range:
            {
            problem =
                "Current date is outside permitted range."
                " Contact the home office."
                ;
            }
            break;
        case secure_date::md5sum_error:
            {
            problem =
                "At least one file is missing, altered, or invalid."
                " Try reinstalling."
                ;
            }
            break;
        case secure_date::incorrect_passkey:
            {
            problem =
                "Passkey is incorrect for this version."
                " Contact the home office."
                ;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << invalid
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // TODO ?? Instead of making this file depend on any gui, either
    // add an untrappable exit facility to 'alert.hpp', or return
    // a string.
    //
    if(invalid)
        {
        std::ostringstream msg;
        msg << "Passkey validation failed.";
        warning() << msg.str();
        std::exit(invalid);
        }
}

# if 0

TODO ?? Perhaps "alert*.?pp" should be extended to provide a behavior
like this, for situations that call for unconditional termination:

#include <wx/app.h>
#include <wx/log.h>

        wxSafeShowMessage
            (problem.c_str()
            ,msg.str().c_str()
            );
        wxExit();
#endif // 0

