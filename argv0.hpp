// Get startup path from argv[0] if available.
//
// Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: argv0.hpp,v 1.5 2007-01-27 00:00:51 wboutin Exp $

#ifndef argv0_hpp
#define argv0_hpp

#include "config.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

// TODO ?? Some platforms provide the startup path in argv[0], and
// favor placing configuration or data files in the same directory.
// This behavior is not portable; the program's files should instead
// reside in locations sanctioned by FHS, such as
//   /etc/opt/lmi
//   /var/opt/lmi
//   /usr/local/share
//   /usr/share/sgml
// or at least their placement should be a configurable option.

// THIRD_PARTY !! Consider adding streaming operators to the filesystem
// library. It seems odd that boost didn't do that already. This file,
// of course, wouldn't be the right place to do that; they'd belong in
// a separate file for general extensions to that library.

namespace boost
{
namespace filesystem
{

// TODO ?? The default argument seems to engender undefined behavior.

inline path const& startup_path(char const* argv0 = 0)
{
    static path p;
    if(p.empty())
        {
        // THIRD_PARTY !! It appears that complete() might be less
        // zany, yet this is what boost's 'simple_ls.cpp' does.
        boost::filesystem::path z(argv0);
        p = boost::filesystem::system_complete(z).branch_path();
        }
    return p;
}

} // namespace boost
} // namespace filesystem

#endif // argv0_hpp

