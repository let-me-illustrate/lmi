// Get startup path from argv[0] (not guaranteed to work portably).
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: argv0.hpp,v 1.2 2005-01-29 02:47:41 chicares Exp $

#ifndef argv0_hpp
#define argv0_hpp

#include "config.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

// TODO ?? Consider adding streaming operators.

namespace boost
{
namespace filesystem
{

// Fails nicely if no argv

inline path const& startup_path(char const* argv0 = 0)
{
    static path p;
    if(p.empty())
        {
        // BOOST !! It appears that complete() might be less zany,
        // yet this is boost's recommended usage.
        boost::filesystem::path z(argv0);
        p = boost::filesystem::system_complete(z).branch_path();
        }
    return p;
}

} // namespace boost
} // namespace filesystem

#endif // argv0_hpp

