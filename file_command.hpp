// Run a command against a file, respecting its extension.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef file_command_hpp
#define file_command_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

typedef void (*file_command_fp_type)(std::string const&, std::string const&);

LMI_SO bool file_command_initialize(file_command_fp_type);

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

struct LMI_SO file_command
{
    void operator()(std::string const&, std::string const&) const;
};

#endif // file_command_hpp
