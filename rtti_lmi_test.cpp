// Extensions to C++ run-time type information--unit test.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
#include <utility>                      // std::move()
#include <vector>

struct RttiLmiTest
{
    class X {};
    static void TestTypeInfo();
    static void test_particularized_type();
};

void RttiLmiTest::TestTypeInfo()
{
    // Assignable.

    lmi::TypeInfo ti0(typeid(double));
    lmi::TypeInfo ti1(typeid(int));
    ti0 = ti1;
    LMI_TEST(typeid(int) == *ti0.ti_);

    // CopyConstructible.

    lmi::TypeInfo ti2(ti1);
    LMI_TEST(typeid(int) == *ti2.ti_);

    // MoveAssignable.
    lmi::TypeInfo mti0(typeid(double));
    lmi::TypeInfo mti1(typeid(int));
    mti0 = std::move(mti1);
    LMI_TEST(typeid(int) == *mti0.ti_);

    // MoveConstructible.

    lmi::TypeInfo mti2(std::move(mti0));
    LMI_TEST(typeid(int) == *mti2.ti_);

    // EqualityComparable.

    LMI_TEST(ti1 == ti2);

    // LessThanComparable.

    LMI_TEST(!(ti1 < ti2));

    // StreamInsertible.

    std::ostringstream oss;
    oss << ti1;
    LMI_TEST_EQUAL(oss.str(), lmi::detail::Demangle(typeid(int).name()));

    // Usable with UDTs.

    ti1 = typeid(X); // Converting ctor.
    LMI_TEST_EQUAL(ti1.Name(), lmi::detail::Demangle(typeid(X).name()));

    // Usable with containers.

    std::vector<lmi::TypeInfo> v;
    std::map<std::string, lmi::TypeInfo> m;

    // Note on the libstdc++ demangler:
    // For gcc-3.4.2 at least, this erroneously prints 'i'; see:
    //   http://gcc.gnu.org/ml/libstdc++/2004-02/msg00326.html
//    std::cout << lmi::TypeId<int>() << std::endl;
}

namespace
{
// Shorter name for testing.
//
// C++20 generic lambdas can be written tersely:
//   auto f = []<typename T>(){return lmi::particularized_type<T>();};
// but must be called verbosely:
//   f.template operator()<int const volatile&>();
// which isn't as tidy an alias as "p_t".
template<typename T>
std::string p_t()
{
    return lmi::particularized_type<T>();
}
} // Unnamed namespace.

void RttiLmiTest::test_particularized_type()
{
    LMI_TEST_EQUAL("int const volatile"   , p_t<int const volatile  >());
    LMI_TEST_EQUAL("int volatile"         , p_t<int volatile        >());
    LMI_TEST_EQUAL("int const"            , p_t<int const           >());
    LMI_TEST_EQUAL("int"                  , p_t<int                 >());
    LMI_TEST_EQUAL("int const volatile&"  , p_t<int const volatile& >());
    LMI_TEST_EQUAL("int volatile&"        , p_t<int volatile&       >());
    LMI_TEST_EQUAL("int const&"           , p_t<int const&          >());
    LMI_TEST_EQUAL("int&"                 , p_t<int&                >());
    LMI_TEST_EQUAL("int const volatile&&" , p_t<int const volatile&&>());
    LMI_TEST_EQUAL("int volatile&&"       , p_t<int volatile&&      >());
    LMI_TEST_EQUAL("int const&&"          , p_t<int const&&         >());
    LMI_TEST_EQUAL("int&&"                , p_t<int&&               >());
}

int test_main(int, char*[])
{
    RttiLmiTest::TestTypeInfo();
    RttiLmiTest::test_particularized_type();
    return 0;
}
