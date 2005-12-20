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

// $Id: file_command.hpp,v 1.3 2005-12-20 00:46:41 chicares Exp $

#ifndef file_command_hpp
#define file_command_hpp

#include "config.hpp"

#include "callback.hpp"
#include "expimp.hpp"

#include <boost/utility.hpp>

#include <string>

/// Execute a named command for a file, choosing the appropriate
/// action based on the file's mime type (for gnome or kde) or its
/// "association" for msw. The commands currently supported are
/// 'print' and 'open'.
///
/// This command is determined by wx, so it works only for programs
/// that link wx; otherwise, no action is taken at all. This seems
/// reasonable enough, because lmi's command-line and cgi-bin
/// interfaces don't need to open files in their associated (GUI)
/// applications or print them. The wx behavior could easily be
/// extended to other interfaces by linking only the non-GUI part of
/// wx, if that ever becomes necessary.

typedef void (*file_command_fp_type)(std::string const&, std::string const&);

bool LMI_EXPIMP file_command_initialize(file_command_fp_type);

struct LMI_EXPIMP file_command
    :private boost::noncopyable
{
    void operator()(std::string const&, std::string const&) const;
};

#endif // file_command_hpp

