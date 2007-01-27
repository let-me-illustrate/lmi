// Instruct the operating system to execute a command.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: system_command.hpp,v 1.8 2007-01-27 00:00:52 wboutin Exp $

#ifndef system_command_hpp
#define system_command_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

/// This is a cover function for std::system(). On posix platforms, it
/// simply forwards its argument to std::system(). On msw, however, it
/// must work around a serious problem.
///
/// The standard function often is implemented on msw to invoke
///   $ComSpec /c command
/// , which of course may fail if $shell is aliased to $ComSpec,
/// eliciting a diagnostic like
///   /c: can't open input file: /c
/// as described here:
///   http://sourceforge.net/mailarchive/message.php?msg_id=13234342
/// The only reliable alternative is to use a platform-specific system
/// call instead of std::system().
///
/// Thus, the elaborate implementation provided for msw does not favor
/// that platform: it just makes it do what a posix platform would do
/// without such complicated workarounds.

int LMI_SO system_command(std::string const& command_line);

#endif // system_command_hpp

