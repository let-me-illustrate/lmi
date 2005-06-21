// Global settings.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: global_settings.cpp,v 1.2 2005-06-21 03:21:56 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

// Initialize directory strings to ".", not an empty string. Reason:
// objects of the boost filesystem library's path class are created
// from these strings, and that class deliberately rejects empty
// strings.

global_settings::global_settings()
    :mellon                    (false)
    ,ash_nazg                  (false)
    ,special_output            (false)
    ,data_directory            (".")
    ,regression_test_directory (".")
{}

global_settings::~global_settings()
{}

bool global_settings::regression_testing()
{
    return regression_test_directory.empty();
}

global_settings& global_settings::instance()
{
    static global_settings z;
    return z;
}

