// Ascertain whether a "container" includes a given element--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "contains.hpp"

#include "miscellany.hpp"               // lmi_array_size()
#include "test_tools.hpp"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>                      // std::pair
#include <vector>

struct HasFind   {void find();};
struct LacksFind {            };

void test_has_member_find()
{
    static_assert( has_member_find<HasFind  >::value, "");
    static_assert(!has_member_find<LacksFind>::value, "");

    static_assert( has_member_find<std::string>::value, "");

    static_assert( has_member_find<std::map   <int,int>>::value, "");
    static_assert( has_member_find<std::set   <int    >>::value, "");
    static_assert(!has_member_find<std::vector<int    >>::value, "");
}

/// Test standard "containers" for which find() makes sense.
///
/// std::queue and std::stack are not traversable; std::bitset has
/// any() and none() already.

void test_contains()
{
    std::string const w("etaoin shrdlu");
    std::string const x("lorem ipsum");
    std::string const y[] = {"O Sibili", "si ergo", "fortibus es", "in ero"};
    std::pair<std::string,std::string> const z[] =
        {std::pair<std::string,std::string>("O Nobili", "demis trux")
        ,std::pair<std::string,std::string>("uatis inem", "causendux")
        };

    // Strings.

    BOOST_TEST( contains(w, w));
    BOOST_TEST(!contains(w, x));
    BOOST_TEST( contains(w, "eta"));
    BOOST_TEST(!contains(w, "zeta"));
    BOOST_TEST( contains(w, 'e'));
    BOOST_TEST(!contains(w, 'q'));

    // Associative containers.

    std::map<std::string,std::string> const m(z, z + lmi_array_size(z));
    BOOST_TEST( contains(m, "uatis inem"    ));
    BOOST_TEST(!contains(m, "cows and ducks"));

    std::set<std::string> const s(y, y + lmi_array_size(y));
    BOOST_TEST( contains(s, "si ergo" ));
    BOOST_TEST(!contains(s, "fortibus"));

    // Sequences.

    std::deque<double> const d(1, 3.14);
    BOOST_TEST( contains(d, 3.14));
    BOOST_TEST(!contains(d, 0.00));

    std::list<double> const t(1, 3.14);
    BOOST_TEST( contains(t, 3.14));
    BOOST_TEST(!contains(t, 0.00));

    std::vector<double> const v(1, 3.14);
    BOOST_TEST( contains(v, 3.14));
    BOOST_TEST(!contains(v, 0.00));
}

int test_main(int, char*[])
{
    test_has_member_find();
    test_contains();

    return 0;
}

