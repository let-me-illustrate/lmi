// Test files for consistency with various rules: private taboos.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include <map>
#include <string>

/// Files containing this regex are exempt from private taboos.
///
/// Copy this file to '../products/src' and edit the copy to specify
/// a regex (such as a specific license notice) that grants an
/// indulgence to utter what would otherwise be taboo.
///
/// This dummy implementation exempts only files containing '\v',
/// which is always forbidden for files subject to taboos; thus,
/// it grants no effective indulgence at all.

std::string my_taboo_indulgence()
{
    return "\v";
}

/// List of private taboo regexen.
///
/// Copy this file to '../products/src' and edit the copy to express
/// private taboos--anything that shouldn't be uttered in the public
/// repository, even to forbid it, such as a client's name.

std::map<std::string, bool> my_taboos()
{
    std::map<std::string, bool> z;
    // taboo regex    ignore case
    // -----------    -----------
    z["Shibboleth"] = false;
    z["sibboleth" ] = true;
    return z;
}
