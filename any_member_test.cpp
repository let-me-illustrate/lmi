// Symbolic member names (obsolete version)--unit test.
//
// Copyright (C) 2001, 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: any_member_test.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#ifdef BC_BEFORE_5_5
#   define BOOST_INCLUDE_MAIN
#   include "test_tools.hpp"
    int test_main(int, char*[])
    {
        BOOST_TEST(false);
        return 1;
    }
#else

#include "any_member_ihs.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

template<typename D>
class V
{
    typedef std::map<std::string, boost::any_member_ihs<D> > member_map;
};

class U
  : public V<U>
{
};

struct Q
{
    float q0;
    Q() : q0(0.0) {}
};

struct T : public Q, public MemberSymbolTable_ihs<T>
{
    int i0;
    int i1;
    double d0;
    std::string s0;

    T()
        :i0(0)
        ,i1(0)
        ,d0(0.0)
        {
        ascribe_members();
        }

  private:
    void ascribe_members()
        {
        ascribe("i0", &T::i0);
        ascribe("i1", &T::i1);
        ascribe("d0", &T::d0);
        ascribe("s0", &T::s0);
        ascribe("q0", (float T::*)&T::q0);
        }
};

// We could pass the object in. That means either
// rewriting 'any' to know its type, or using a
// member function template--which can't be virtual.

int test_main(int, char*[])
{
    T r0;
    BOOST_TEST(0   == r0.i0);
    BOOST_TEST(0   == r0.i1);
    BOOST_TEST(0.0 == r0.d0);
    BOOST_TEST(""  == r0.s0);

    // Test writing through a pointer to member.
    int T::*x = &T::i0;
    r0.*x = 5;
    BOOST_TEST(5   == r0.i0);
    BOOST_TEST(0   == r0.i1);
    BOOST_TEST(0.0 == r0.d0);

    // Test writing through the map of ascribed member names.
//    r0["i0"] = "999.9"; // TODO ??
    r0["i0"] = "999";
//    r0["i1"] = "888e3"; // TODO ??
    r0["i1"] = "888000";
    r0["d0"] = "777";
    r0["s0"] = "hello";
    BOOST_TEST(999     == r0.i0);
    BOOST_TEST(888000  == r0.i1);
    BOOST_TEST(777.0   == r0.d0);
    BOOST_TEST("hello" == r0.s0);

    T r1;
    r1["i0"] = "135";
    r1["i1"] = "246";
    r1["d0"] = "888";
    r1["s0"] = "world";
    BOOST_TEST(135     == r1.i0);
    BOOST_TEST(246     == r1.i1);
    BOOST_TEST(888.0   == r1.d0);
    BOOST_TEST("world" == r1.s0);

    // Make sure that didn't affect the other instance of class T.
    BOOST_TEST(999     == r0.i0);
    BOOST_TEST(888e3   == r0.i1);
    BOOST_TEST(777.0   == r0.d0);
    BOOST_TEST("hello" == r0.s0);

    r0["i0"] = "1234567";
    BOOST_TEST(1234567 == r0.i0);
    BOOST_TEST(888e3   == r0.i1);
    BOOST_TEST(777.0   == r0.d0);
    BOOST_TEST("hello" == r0.s0);

    // Make sure that didn't affect the other instance of class T.
    BOOST_TEST(135     == r1.i0);
    BOOST_TEST(246     == r1.i1);
    BOOST_TEST(888.0   == r1.d0);
    BOOST_TEST("world" == r1.s0);

    // Works for base class Q too.
    T r2;
    BOOST_TEST(0.0f == r2.q0);

    r2["q0"] = "123.456";
    BOOST_TEST(123.456f == r2.q0);

    // Test equality operator.
    BOOST_TEST(  r1["i0"] == r1["i0"]);
    BOOST_TEST(  r1["i0"] != r0["i0"]);
    BOOST_TEST(!(r1["i0"] == r0["i0"]));

    BOOST_TEST(  r1["d0"] == r1["d0"]);
    BOOST_TEST(  r1["d0"] != r0["d0"]);
    BOOST_TEST(!(r1["d0"] == r0["d0"]));

    r1["d0"] = r0["d0"];
    BOOST_TEST(  r1["d0"] == r1["d0"]);
    BOOST_TEST(  r1["d0"] == r0["d0"]);
    BOOST_TEST(!(r1["d0"] != r0["d0"]));

    // Test equality operator with empty strings--we once observed
    // a problem in that case.
    r1["s0"] = "";
    BOOST_TEST(  r1["s0"] == r1["s0"]);
    BOOST_TEST(  r1["s0"] != r0["s0"]);
    BOOST_TEST(!(r1["s0"] != r1["s0"]));
    BOOST_TEST(!(r1["s0"] == r0["s0"]));


#if 0
// TODO ?? This experimental stuff doesn't work....

    // Test implicit copy constructor.
    T copy_constructed(r1);
    copy_constructed["s0"] = "copy-constructed";
    BOOST_TEST("copy-constructed" == copy_constructed.s0);
    BOOST_TEST("world"  == r1.s0);
std::cout << "Should be 'copy-constructed': " << copy_constructed.s0 << '\n';
std::cout << "r1--should be 'world' : "       << r1.s0               << '\n';

    // Make sure that didn't affect the thing-copied-from.
    BOOST_TEST(135     == r1.i0);
    BOOST_TEST(246     == r1.i1);
    BOOST_TEST(888.0   == r1.d0);
    BOOST_TEST("world" == r1.s0);

    // Test implicit assignment operator.
    T copy_assigned;
    copy_assigned = r1;
    copy_assigned["s0"] = "copy-assigned";
    BOOST_TEST("copy-assigned" == copy_assigned   .s0);
    BOOST_TEST("world"  == r1.s0);
std::cout << "Should be 'copy-assigned': " << copy_assigned.s0 << '\n';
std::cout << "r1--should be 'world' : "    << r1.s0            << '\n';

    // Make sure that didn't affect the thing-copied-from.
    BOOST_TEST(135     == r1.i0);
    BOOST_TEST(246     == r1.i1);
    BOOST_TEST(888.0   == r1.d0);
    BOOST_TEST("world" == r1.s0);
#endif // 0

    int return_value = -1;

    try
        {
        // This should throw:
        r2["unknown_member"];
        // This shouldn't be reached:
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        return_value = 0;
        }

    return return_value;
}

#endif // Not old borland compiler.

