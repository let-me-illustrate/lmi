// Run a command against a file, respecting its extension.
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

// $Id: file_command.cpp,v 1.2 2005-05-13 23:58:10 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "file_command.hpp"

namespace
{
    callback<file_command_fp_type> file_command_callback;
}

bool file_command_initialize(file_command_fp_type f)
{
    file_command_callback.initialize(f);
    return true;
}

void file_command::operator()
    (std::string const& file
    ,std::string const& action
    ) const
    {
    file_command_callback()(file, action);
    }

