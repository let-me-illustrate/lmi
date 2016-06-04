// Instruct the operating system to execute a command.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "system_command.hpp"

#include "callback.hpp"

namespace
{
    callback<system_command_fp_type> system_command_callback;
}

typedef system_command_fp_type FunctionPointer;
template<> FunctionPointer callback<FunctionPointer>::function_pointer_ = nullptr;

bool system_command_initialize(system_command_fp_type f)
{
    system_command_callback.initialize(f);
    return true;
}

void system_command(std::string const& command_line)
{
    system_command_callback()(command_line);
}

