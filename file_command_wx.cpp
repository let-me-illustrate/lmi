// Run file command--wx interface.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: file_command_wx.cpp,v 1.4 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "file_command.hpp"

#include "alert.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>

// WX !! wx/mimetype.h should include wx/arrstr.h; until it does,
// include the required header here explicitly:
#include <wx/arrstr.h>
#include <wx/mimetype.h>
#include <wx/utils.h> // wxExecute()

// Implementing this function in a gui module means the wx facility
// can be used. Currently, no other interface needs to implement it.

namespace
{
void concrete_file_command
    (std::string const& file
    ,std::string const& action
    )
{
    fs::path path(file.c_str());
    std::string extension = fs::extension(path);

    boost::scoped_ptr<wxFileType> ft
        (wxTheMimeTypesManager->GetFileTypeFromExtension(extension.c_str())
        );

    if(!ft)
        {
        fatal_error()
            << "File type '"
            << extension
            << "' unknown."
            << LMI_FLUSH
            ;
        }

    wxString cmd;
    bool okay(false);
    if("open" == action)
        {
        okay = ft->GetOpenCommand
            (&cmd
            ,wxFileType::MessageParameters
                (path.native_file_string().c_str()
                ,""
                )
            );
        }
    else if("print" == action)
        {
        okay = ft->GetPrintCommand
            (&cmd
            ,wxFileType::MessageParameters
                (path.native_file_string().c_str()
                ,""
                )
            );
        }
    else
        {
        fatal_error()
            << "Action '"
            << action
            << "' unrecognized."
            << LMI_FLUSH
            ;
        return;
        }

    if(!okay)
        {
        fatal_error()
            << "Unable to determine command to '"
            << action
            << "' file '"
            << path.native_file_string().c_str()
            << "'."
            << LMI_FLUSH
            ;
        }
    okay = wxExecute(cmd);
    if(!okay)
        {
        fatal_error()
            << "Unable to '"
            << action
            << "' file '"
            << path.native_file_string().c_str()
            << "'. Return code: '"
            << okay
            << "'."
            << LMI_FLUSH
            ;
        }
}

// See
//   <news:1006352851.15484.0.nnrp-08.3e31d362@news.demon.co.uk>
// and Kanze's reply.
//
volatile bool ensure_setup = file_command_initialize(concrete_file_command);
} // Unnamed namespace.

