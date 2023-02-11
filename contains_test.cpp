// Ascertain whether a "container" includes a given element--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "contains.hpp"

#include "test_tools.hpp"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>                      // pair
#include <vector>

struct HasFind   {void find();};
struct LacksFind {            };

void test_has_member_find()
{
    static_assert( has_member_find<HasFind  >::value);
    static_assert(!has_member_find<LacksFind>::value);

    static_assert( has_member_find<std::string>::value);

    static_assert( has_member_find<std::map   <int,int>>::value);
    static_assert( has_member_find<std::set   <int    >>::value);
    static_assert(!has_member_find<std::vector<int    >>::value);

    // same, with has_member_find_v
    static_assert( has_member_find_v<HasFind  >);
    static_assert(!has_member_find_v<LacksFind>);

    static_assert( has_member_find_v<std::string>);

    static_assert( has_member_find_v<std::map   <int,int>>);
    static_assert( has_member_find_v<std::set   <int    >>);
    static_assert(!has_member_find_v<std::vector<int    >>);
}

/// Test standard "containers" for which find() makes sense.
///
/// std::queue and std::stack are not traversable; std::bitset has
/// any() and none() already.

void test_contains()
{
    std::string const w("etaoin shrdlu");
    std::string const x("lorem ipsum");

    // Strings.

    LMI_TEST( contains(w, w));
    LMI_TEST(!contains(w, x));
    LMI_TEST( contains(w, "eta"));
    LMI_TEST(!contains(w, "zeta"));
    LMI_TEST( contains(w, 'e'));
    LMI_TEST(!contains(w, 'q'));

    // Associative containers.

    std::set<std::string> const s
        {"O Sibili"
        ,"si ergo"
        ,"fortibus es"
        ,"in ero"
        };
    LMI_TEST( contains(s, "si ergo" ));
    LMI_TEST(!contains(s, "fortibus"));

    std::map<std::string,std::string> const m
        {{"O Nobili", "demis trux"}
        ,{"uatis inem", "causendux"}
        };
    LMI_TEST( contains(m, "uatis inem"    ));
    LMI_TEST(!contains(m, "cows and ducks"));

    // Sequences.

    std::deque<double> const d(1, 3.14);
    LMI_TEST( contains(d, 3.14));
    LMI_TEST(!contains(d, 0.00));

    std::list<double> const t(1, 3.14);
    LMI_TEST( contains(t, 3.14));
    LMI_TEST(!contains(t, 0.00));

    std::vector<double> const v(1, 3.14);
    LMI_TEST( contains(v, 3.14));
    LMI_TEST(!contains(v, 0.00));
}

int test_main(int, char*[])
{
    test_has_member_find();
    test_contains();

    return 0;
}
