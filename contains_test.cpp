// Ascertain whether a "container" includes a given element--unit test.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "contains.hpp"

#include "test_tools.hpp"

#include <boost/static_assert.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

struct HasFind   {void find();};
struct LacksFind {            };

void test_has_member_find()
{
#if !defined LMI_NO_SFINAE
    BOOST_STATIC_ASSERT( has_member_find<HasFind  >::value);
    BOOST_STATIC_ASSERT(!has_member_find<LacksFind>::value);

    BOOST_STATIC_ASSERT( has_member_find<std::string>::value);

    BOOST_STATIC_ASSERT(( has_member_find<std::map   <int,int> >::value));
    BOOST_STATIC_ASSERT ( has_member_find<std::set   <int    > >::value) ;
    BOOST_STATIC_ASSERT (!has_member_find<std::vector<int    > >::value) ;
#endif // !defined LMI_NO_SFINAE
}

void test_contains()
{
    std::string const s("etaoin shrdlu");
    std::string const t("lorem ipsum");
    BOOST_TEST( contains(s, s));
    BOOST_TEST(!contains(s, t));
    BOOST_TEST( contains(s, "eta"));
    BOOST_TEST(!contains(s, "zeta"));

#if 0 // This is where has_member_find<> will help.
    std::set<std::string> u;
    u.insert("one");
    BOOST_TEST( contains(u, "one"));
    BOOST_TEST(!contains(u, "two"));
#endif // 0

    std::map<std::string, int> m;
    m["one"] = 1;
    BOOST_TEST( contains(m, "one"));
    BOOST_TEST(!contains(m, "two"));

    std::vector<double> v;
    v.push_back(3.14);
    BOOST_TEST( contains(v, 3.14));
    BOOST_TEST(!contains(v, 0.00));
}

int test_main(int, char*[])
{
    test_has_member_find();
    test_contains();

    return 0;
}

