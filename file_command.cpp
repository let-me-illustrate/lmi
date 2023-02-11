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

#include "pchfile.hpp"

#include "file_command.hpp"

#include "callback.hpp"

namespace
{
    callback<file_command_fp_type> file_command_callback;
} // Unnamed namespace.

typedef file_command_fp_type FunctionPointer;
template<> FunctionPointer callback<FunctionPointer>::function_pointer_ = nullptr;

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
