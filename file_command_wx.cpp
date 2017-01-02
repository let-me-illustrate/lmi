// Run file command--wx interface.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "file_command.hpp"

#include "alert.hpp"
#include "force_linking.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>

#include <wx/mimetype.h>
#include <wx/utils.h>                   // wxExecute()

LMI_FORCE_LINKING_IN_SITU(file_command_wx)

// Implementing this function in a GUI module means the wx facility
// can be used. Currently, no other interface needs to implement it.

namespace
{
void concrete_file_command
    (std::string const& file
    ,std::string const& action
    )
{
    fs::path path(file);
    std::string extension = fs::extension(path);

    boost::scoped_ptr<wxFileType> ft
        (wxTheMimeTypesManager->GetFileTypeFromExtension(extension)
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
                (path.native_file_string()
                ,""
                )
            );
        }
    else if("print" == action)
        {
        okay = ft->GetPrintCommand
            (&cmd
            ,wxFileType::MessageParameters
                (path.native_file_string()
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
            << path.native_file_string()
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
            << path.native_file_string()
            << "'. Return code: '"
            << okay
            << "'."
            << LMI_FLUSH
            ;
        }
}

/// See:
///   http://groups.google.com/groups?selm=1006352851.15484.0.nnrp-08.3e31d362@news.demon.co.uk
/// and Kanze's reply:
///   http://groups.google.com/groups?selm=d6651fb6.0111221034.42e78b95@posting.google.com

volatile bool ensure_setup = file_command_initialize(concrete_file_command);
} // Unnamed namespace.

