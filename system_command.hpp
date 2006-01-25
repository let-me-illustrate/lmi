// Instruct the operating system to execute a command.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: system_command.hpp,v 1.5 2006-01-25 07:20:01 chicares Exp $

#ifndef system_command_hpp
#define system_command_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

/// This is a cover function for std::system(). On posix platforms, it
/// simply forwards its argument to std::system(). On msw, however, it
/// must work around two serious problems.
///
/// The standard function often is implemented on msw to invoke
///   $ComSpec /c command
/// , which of course may fail if $shell is aliased to $ComSpec,
/// eliciting a diagnostic like
///   /c: can't open input file: /c
/// as described here:
///   http://sourceforge.net/mailarchive/message.php?msg_id=13234342
/// The only reliable alternative is to use a platform-specific system
/// call.
///
/// Another msw problem is that an alien process may reset the fpu
/// control word:
///   http://lists.gnu.org/archive/html/lmi/2006-01/msg00025.html
/// so the implementation tests the control word both before and after
/// it issues the command. If the precision setting was changed by the
/// command, it's restored to its desired value. This silently fixes
/// the problem expected from a rogue process built with ms tools,
/// which use the same settings as intel except for precision; that's
/// the only problem actually observed. Any other discrepancy is
/// reported.
///
/// Thus, the elaborate implementation provided for msw does not favor
/// that platform: it just makes it do what a posix platform would do
/// without such complicated workarounds.

int LMI_SO system_command(std::string const& command_line);

#endif // system_command_hpp

