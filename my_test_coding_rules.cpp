// Test files for consistency with various rules: private taboos.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: my_test_coding_rules.cpp,v 1.1 2007-12-26 18:20:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include <map>
#include <string>

/// Public dummy for private taboo regexen.
///
/// Copy this file to '../products/src' and edit the copy to express
/// private taboos--anything that shouldn't be uttered in cvs, even to
/// forbid it, such as a client's name.

std::map<std::string, bool> my_taboos()
{
    std::map<std::string, bool> z;
    // taboo regex    ignore case
    // -----------    -----------
    z["Shibboleth"] = false;
    z["sibboleth" ] = true;
    return z;
}

