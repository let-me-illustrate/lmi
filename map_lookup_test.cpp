// Look up a value in a std::map--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: map_lookup_test.cpp,v 1.1 2005-12-02 16:00:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "map_lookup.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

int test_main(int, char*[])
{
    std::map<int,std::string> polygons;
    polygons[3] = "triangle";
    polygons[4] = "square";
    polygons[6] = "hexagon";

    BOOST_TEST_EQUAL("hexagon", map_lookup(polygons, 6));

    BOOST_TEST_THROW
        (map_lookup(polygons, 5)
        ,std::runtime_error
        ,"map_lookup: key '5' not found."
        );

    return 0;
}

