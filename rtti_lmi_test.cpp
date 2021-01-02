// Extensions to C++ run-time type information--unit test.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "rtti_lmi.hpp"

#include "test_tools.hpp"

#include <map>
#include <sstream>
#include <vector>

struct RttiLmiTest
{
    class X {};
    static void TestTypeInfo();
};

void RttiLmiTest::TestTypeInfo()
{
    // Assignable.

    lmi::TypeInfo ti0(typeid(double));
    lmi::TypeInfo ti1(typeid(int));
    ti0 = ti1;
    BOOST_TEST(typeid(int) == *ti0.ti_);

    // CopyConstructible.

    lmi::TypeInfo ti2(ti1);
    BOOST_TEST(typeid(int) == *ti2.ti_);

    // EqualityComparable.

    BOOST_TEST(ti1 == ti2);

    // LessThanComparable.

    BOOST_TEST(!(ti1 < ti2));

    // StreamInsertible.

    std::ostringstream oss;
    oss << ti1;
    BOOST_TEST_EQUAL(oss.str(), lmi::detail::Demangle(typeid(int).name()));

    // Usable with UDTs.

    ti1 = typeid(X); // Converting ctor.
    BOOST_TEST_EQUAL(ti1.Name(), lmi::detail::Demangle(typeid(X).name()));

    // Usable with containers.

    std::vector<lmi::TypeInfo> v;
    std::map<std::string, lmi::TypeInfo> m;

    // Note on the libstdc++ demangler:
    // For gcc-3.4.2 at least, this erroneously prints 'i'; see:
    //   http://gcc.gnu.org/ml/libstdc++/2004-02/msg00326.html
//    std::cout << lmi::TypeId<int>() << std::endl;
}

int test_main(int, char*[])
{
    RttiLmiTest::TestTypeInfo();
    return 0;
}
