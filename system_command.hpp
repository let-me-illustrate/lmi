// Instruct the operating system to execute a command.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: system_command.hpp,v 1.4 2005-12-22 13:59:49 chicares Exp $

// This is a cover function for std::system(). It simply forwards its
// argument to std::system() on posix platforms. The msw implementation
// works around a problem on that platform, where the standard function
// generally assumes a shell that's drastically incompatible with posix.
// One could write different commands for the msw platform everywhere,
// but that is gratuitously tedious; it is far better to write commands
// one way and make non-posix platforms interpret them correctly.
//
// The elaborate implementation provided for the msw platform does
// not provide any capability that posix does not provide natively.
// Instead, it works around a defect in common msw implementations
// of std::system() that makes them incompatible with /bin/sh .

#ifndef system_command_hpp
#define system_command_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

int LMI_SO system_command(std::string const& command_line);

#endif // system_command_hpp

