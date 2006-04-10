// Test case for product editor guid component prototype.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: test_pedit.cpp,v 1.1.2.2 2006-04-10 20:26:03 etarassov Exp $

#include "pedit.hpp"

#include <boost/filesystem/path.hpp>

namespace
{   // anonymous namespace
    bool InitBoostFilesystem()
    {
        boost::filesystem::path::default_name_check( boost::filesystem::no_check );
        return true; // a dummy value
    }
    static bool dummy = InitBoostFilesystem();
}

IMPLEMENT_APP( Pedit )
