// Look up a value in a std::map--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "map_lookup.hpp"

#include "test_tools.hpp"

int test_main(int, char*[])
{
    std::map<int,std::string> polygons;
    polygons[3] = "triangle";
    polygons[4] = "square";
    polygons[6] = "hexagon";

    LMI_TEST_EQUAL("hexagon", map_lookup(polygons, 6));

    LMI_TEST_THROW
        (map_lookup(polygons, 5)
        ,std::runtime_error
        ,"map_lookup: key '5' not found."
        );

    return 0;
}
