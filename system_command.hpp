// Instruct the operating system to execute a command.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef system_command_hpp
#define system_command_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

typedef void (*system_command_fp_type)(std::string const&);

LMI_SO bool system_command_initialize(system_command_fp_type);

/// Wrapper for std::system() that throws on failure.
///
/// Postcondition: the command returned EXIT_SUCCESS.
///
/// Throws if the postcondition was not established.
///
/// Rationale for throwing: A failing system command is generally an
/// irrecoverable error that makes it pointless to continue normally.
/// Throwing an informative exception favors uniform presentation and
/// obviates testing a return code downstream. Richer diagnostic
/// information may be available only where the command is invoked;
/// see the wx implementation for an example.
///
/// For the wx interface, it's implemented in terms of wxExecute().
///
/// Otherwise, on posix platforms, it simply forwards its argument to
/// std::system(). On msw, however, it must work around a serious
/// problem.
///
/// The standard function often is implemented on msw to invoke
///   $ComSpec /c command
/// , which of course may fail if $shell is aliased to $ComSpec,
/// eliciting a diagnostic like
///   /c: can't open input file: /c
/// as described here:
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/7396
///     [2003-05-26T19:06:41Z from Greg Chicares]
/// The only reliable alternative is to use a platform-specific system
/// call instead of std::system().
///
/// Thus, the elaborate implementation provided for msw does not favor
/// that platform: it just makes it do what a posix platform would do
/// without such complicated workarounds.

LMI_SO void system_command(std::string const& command_line);

#endif // system_command_hpp
