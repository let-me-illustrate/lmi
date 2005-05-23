// Symbolic member names--unit test.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: any_member_test.cpp,v 1.7 2005-05-23 12:16:49 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "any_member.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <cmath> // std::exp()
#include <functional>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>

struct base_datum
{
    base_datum() :sane(7) {}
    virtual ~base_datum() {}              // Just to make it polymorphic.
    virtual void abstract_function() = 0; // Just to make it abstract.
    bool base_function()
        {
        std::cout << "base_datum::base_function() called " << sane << std::endl;
        return true;
        }
    int sane;
};

// Unused stub.
std::istream& operator>>(std::istream& is, base_datum&      )
{
    return is;
}

std::ostream& operator<<(std::ostream& os, base_datum const& z)
{
    return os << z.sane << '\n';
}

struct derived_datum
    :public base_datum
{
    void abstract_function() {}
};

// Unused stub.
std::istream& operator>>(std::istream& is, derived_datum&      )
{
    return is;
}

std::ostream& operator<<(std::ostream& os, derived_datum const& z)
{
    return os << z.sane << '\n';
}

struct X
{
    X() {str_ = "ERROR";}
    X(double) {}
    void set_str(std::string const& s) {str_ = s;}
    std::string const& str() const {return str_;}
    int foo(std::string s)
        {std::cout << "[" << str() << "] X::foo() called: " << s << std::endl; return 0;}
    void void_foo(std::string s)
        {std::cout << "[" << str() << "] X::void_foo() called: " << s << std::endl;}
    std::string const& string_foo(std::string s)
        {std::cout << "[" << str() << "] X::string_foo() called: " << s << std::endl; return str();}
    void bar()
        {std::cout << "[" << str() << "] X::bar() called." << std::endl;}
    int int_bar() const
        {std::cout << "[" << str() << "] X::bar() called." << std::endl; return 0;}
    void bar_int(int)
        {std::cout << "[" << str() << "] X::bar_int() called." << std::endl;}
    std::string str_;
};

std::istream& operator>>(std::istream& is, X&        ) {return is;}
std::ostream& operator<<(std::ostream& os, X const& x) {return os << x.str() << '\n';}

class S
    :public MemberSymbolTable<S>
{
  public:
    S();
    std::ostream& write(std::ostream&);

  public: // Exposed for testing.
    int i0;
    int i1;
    double d0;
    std::string s0;

    X x0;
    derived_datum dd;
};

S::S()
    :i0(0)
    ,i1(0)
    ,d0(0.0)
{
    ascribe("i0", &S::i0);
    ascribe("i1", &S::i1);
    ascribe("d0", &S::d0);
    ascribe("s0", &S::s0);
    ascribe("x0", &S::x0);
    ascribe("dd", &S::dd);
}

std::ostream& S::write(std::ostream& os)
{
    os
        << i0
        << " "
        << i1
        << " "
        << d0
        << " "
        << s0
        ;
    return os;
}

template<typename D>
class V
{
    typedef std::map<std::string, any_member<D> > member_map;
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

struct T : public Q, public MemberSymbolTable<T>
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
        ascribe("q0", static_cast<float T::*>(&T::q0));
        }
};

void test0()
{
    T r0;
    BOOST_TEST_EQUAL(0  , r0.i0);
    BOOST_TEST_EQUAL(0  , r0.i1);
    BOOST_TEST_EQUAL(0.0, r0.d0);
    BOOST_TEST_EQUAL("" , r0.s0);

    // Test writing through a pointer to member.
    int T::*x = &T::i0;
    r0.*x = 5;
    BOOST_TEST_EQUAL(5  , r0.i0);
    BOOST_TEST_EQUAL(0  , r0.i1);
    BOOST_TEST_EQUAL(0.0, r0.d0);

    // Test writing through the map of ascribed member names.
//    r0["i0"] = "999.9"; // TODO ??
    r0["i0"] = "999";
//    r0["i1"] = "888e3"; // TODO ??
    r0["i1"] = "888000";
    r0["d0"] = "777";
    r0["s0"] = "hello";
    BOOST_TEST_EQUAL(999    , r0.i0);
    BOOST_TEST_EQUAL(888000 , r0.i1);
    BOOST_TEST_EQUAL(777.0  , r0.d0);
    BOOST_TEST_EQUAL("hello", r0.s0);

    T r1;
    r1["i0"] = "135";
    r1["i1"] = "246";
    r1["d0"] = "888";
    r1["s0"] = "world";
    BOOST_TEST_EQUAL(135    , r1.i0);
    BOOST_TEST_EQUAL(246    , r1.i1);
    BOOST_TEST_EQUAL(888.0  , r1.d0);
    BOOST_TEST_EQUAL("world", r1.s0);

    // Make sure that didn't affect the other instance of class T.
    BOOST_TEST_EQUAL(999    , r0.i0);
    BOOST_TEST_EQUAL(888e3  , r0.i1);
    BOOST_TEST_EQUAL(777.0  , r0.d0);
    BOOST_TEST_EQUAL("hello", r0.s0);

    r0["i0"] = "1234567";
    BOOST_TEST_EQUAL(1234567, r0.i0);
    BOOST_TEST_EQUAL(888e3  , r0.i1);
    BOOST_TEST_EQUAL(777.0  , r0.d0);
    BOOST_TEST_EQUAL("hello", r0.s0);

    // Make sure that didn't affect the other instance of class T.
    BOOST_TEST_EQUAL(135    , r1.i0);
    BOOST_TEST_EQUAL(246    , r1.i1);
    BOOST_TEST_EQUAL(888.0  , r1.d0);
    BOOST_TEST_EQUAL("world", r1.s0);

    // Works for base class Q too.
    T r2;
    BOOST_TEST_EQUAL(0.0f, r2.q0);

    r2["q0"] = "123.456";
    BOOST_TEST_EQUAL(123.456f, r2.q0);

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
    BOOST_TEST_EQUAL("copy-constructed", copy_constructed.s0);
    BOOST_TEST_EQUAL("world", r1.s0);

    // Make sure that didn't affect the thing-copied-from.
    BOOST_TEST_EQUAL(135    , r1.i0);
    BOOST_TEST_EQUAL(246    , r1.i1);
    BOOST_TEST_EQUAL(888.0  , r1.d0);
    BOOST_TEST_EQUAL("world", r1.s0);

    // Test implicit assignment operator.
    T copy_assigned;
    copy_assigned = r1;
    copy_assigned["s0"] = "copy-assigned";
    BOOST_TEST_EQUAL("copy-assigned", copy_assigned   .s0);
    BOOST_TEST_EQUAL("world", r1.s0);

    // Make sure that didn't affect the thing-copied-from.
    BOOST_TEST_EQUAL(135    , r1.i0);
    BOOST_TEST_EQUAL(246    , r1.i1);
    BOOST_TEST_EQUAL(888.0  , r1.d0);
    BOOST_TEST_EQUAL("world", r1.s0);
#endif // 0

    BOOST_TEST_THROW(r2["unknown_member"], std::runtime_error, "");
}

void test1()
{
    S s;
    X x;

    {
    std::cout << "Testing std::mem_fun family.\n";
    s.x0.set_str("Test 0");
    x.set_str("Test 0x");
    std::mem_fun1_t<int, X, std::string> x_memfun(&X::foo);
    X* px = &x;
//    px->*x_memfun; // Error: it's a functor, not a pmf.
    x_memfun(px, "example 0");
    std::mem_fun(&X::foo)(px, "example 1");
    std::cout << std::endl;
    }

    {
    std::cout << "Testing plain pointers to member data and function.\n";
    s.x0.set_str("Test 1");
    x.set_str("Test 1x");
    X S::* pmd = &S::x0;
    int(X::*pmf)(std::string) = &X::foo;

    // Parentheses required because pointer-to-member operators have
    // lower precedence than function-call.
    (x.*pmf)("Called pmf: example 0.");

    X datum = s.*pmd;
    (datum.*pmf)("Called pmf: example 1.");

    (s.*&S::x0.*&X::foo)("Called pmf through pmd: example 0.");
    (s.*&S::x0.*pmf)("Called pmf through pmd: example 1.");
    (s.*pmd.*pmf)("Called pmf through pmd: example 2.");

    std::cout << std::endl;
    }

    {
    std::cout << "Testing member function cast().\n";
    s.x0.set_str("Test 2");

    s["x0"].cast<X>().foo("example 0");

    X xx = s["x0"].cast<X>();
    xx.foo("example 1");

    (xx               .*&X::foo)("example 2");
    (s["x0"].cast<X>().*&X::foo)("example 3");

    // Cast derived-class member to its base.

    s["dd"].cast<derived_datum>();
    s["dd"].cast<derived_datum>().base_function();
    }

    {
    std::cout << "Testing abstract-base-class member functions.\n";
    s.x0.set_str("Test 3");

    // Want to write something like:
    //   s["x0"].foo(); // no matching function for call to any_member<S>::foo()
    // but we need to supply the actual type:
    //   s["x0"].cast<X>().foo("example 0");

    derived_datum S::* pmd = &S::dd;
//    base_datum S::* pmdb = &S::dd; // invalid conversion
    derived_datum datum = s.*pmd;
    dynamic_cast<base_datum&>(datum).base_function();
    // s["dd"]) --> any_member<S>
    // &S::dd   --> derived_datum S::*

    bool (derived_datum::*pmf0)() = &derived_datum::base_function;
    (s.dd.*pmf0)();
    bool (derived_datum::*pmf1)() = &   base_datum::base_function;
    (s.dd.*pmf1)();
    bool (   base_datum::*pmf2)() = &derived_datum::base_function;
    (s.dd.*pmf2)();
    bool (   base_datum::*pmf3)() = &   base_datum::base_function;
    (s.dd.*pmf3)();

    (s.*pmd.*pmf3)();
    (s.*&S::dd.*pmf3)();

    // We don't want to know the actual type of what 's["dd"]' designates,
    // much less cast to that type.

    // Contravariance:
    // 'T Base::*' -> 'T Derived::*' is a standard conversion
    // 'T Derived::*' -> 'T Base::*' is not.
    // The latter requires an explicit cast, which must [5.2.9/9]
    // be a static_cast to map
    //   Derived::*T --> Base::*T
    // which is what we want to do with the function member. And
    // 4.11/1 makes
    //   Base::*T --> Derived::*T
    // a standard conversion that needs no cast.

//    s["dd"].cast<base_datum>();
// Cannot cast from 'M1S13derived_datum' to 'M1S10base_datum'.
//
// /MinGW/bin/c++filt M1S13derived_datum
// derived_datum S::*
// /MinGW/bin/c++filt M1S10base_datum
// base_datum S::*

    std::cout << std::endl;
    }
}

int test_main(int, char*[])
{
    S s;
    s["i0"] = "999";
    BOOST_TEST_EQUAL(s.i0, 999);

    s["i0"] = "077";   // Interpreted as decimal, not as octal.
    BOOST_TEST_EQUAL(s.i0, 77);

    s["i0"] = "09";    // Valid decimal, not invalid octal.
    BOOST_TEST_EQUAL(s.i0, 9);

    s["i1"] = "888000";
    BOOST_TEST_EQUAL(s.i1, 888000);

//  s["i0"] = "999.9"; // Invalid integer-literal.
//  s["i1"] = "888e3"; // Invalid integer-literal.

    s["d0"] = "777";
    BOOST_TEST_EQUAL(s.d0, 777);

    s["d0"] = "777.";
    BOOST_TEST_EQUAL(s.d0, 777);

    s["d0"] = "777E3";
    BOOST_TEST_EQUAL(s.d0, 777000);

    s["d0"] = ".777E3";
    BOOST_TEST_EQUAL(s.d0, 777);

    s["s0"] = "hello";
    BOOST_TEST_EQUAL(s.s0, "hello");

    std::stringstream ss;
    s.write(ss);
    BOOST_TEST_EQUAL(ss.str(), "9 888000 777 hello");

    // Test const operator[]().
    S const s_const;
    BOOST_TEST_EQUAL("0", s_const["i0"].str());

    // Make sure numeric_io_cast is used for writing arithmetic types
    // to std::string (this test assumes IEC 60559 doubles).

    s.d0 = std::exp(1.0);
    double d1 = s["d0"].cast<double>();
    BOOST_TEST_EQUAL(numeric_io_cast<std::string>(d1), "2.718281828459045");
    BOOST_TEST_EQUAL(s["d0"].str(), "2.718281828459045");

    // Want to be able to unify a subobject with a pmf, e.g.
//    s["s0"].size();
    // no matching function for call to `any_member<S>::size()
    std::string str("xyzzy");

//    std::mem_fun(&std::string::size);
//    str.string_size();

//    str.(std::mem_fun(&std::string::size));

//    std::mem_fun_t sizer;
//    std::mem_fun_t sizer();
//    std::const_mem_fun_t(std::string::size);
    std::mem_fun(&std::string::size)(&str);

    test0();
    test1();

    return 0;
}

