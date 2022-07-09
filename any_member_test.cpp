// Symbolic member names--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "any_member.hpp"

#include "test_tools.hpp"

#include <cmath>                        // exp()
#include <functional>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>

struct base_datum
{
    base_datum() :sane(7) {}

    base_datum(base_datum const&) = default;
    base_datum(base_datum&&) = default;
    base_datum& operator=(base_datum const&) = default;
    base_datum& operator=(base_datum&&) = default;
    virtual ~base_datum() = default;    // Just to make it polymorphic.

    virtual int virtual_function() = 0; // Just to make it abstract.
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
    bool operator==(derived_datum const& z) const
        {return 7 == sane && 7 == z.sane;}
    int virtual_function() override {return 1729;}
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
    bool operator==(X const& x) const {return x.str_ == str_;}
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
    friend class any_member_test;

  public:
    S();

    bool operator==(S const&) const;
    bool operator!=(S const&) const;

    std::ostream& write(std::ostream&);

  private:
    int i0;
    int i1;
    double d0;
    std::string s0;

    X x0;
    derived_datum dd;
};

S::S()
    :i0 {0}
    ,i1 {0}
    ,d0 {0.0}
{
    ascribe("i0", &S::i0);
    ascribe("i1", &S::i1);
    ascribe("d0", &S::d0);
    ascribe("s0", &S::s0);
    ascribe("x0", &S::x0);
    ascribe("dd", &S::dd);
}

bool S::operator==(S const& s) const
{
    return MemberSymbolTable<S>::equals(s);
}

bool S::operator!=(S const& s) const
{
    return !operator==(s);
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
    typedef std::map<std::string, any_member<D>> member_map;
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
        :i0 {0}
        ,i1 {0}
        ,d0 {0.0}
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
// Don't need to cast base-class members explicitly:
//        ascribe("q0", static_cast<float T::*>(&T::q0));
        ascribe("q0", &T::q0);
        }
};

template<> struct reconstitutor<base_datum,S>
{
    typedef base_datum DesiredType;
    static DesiredType* reconstitute(any_member<S>& m)
        {
        DesiredType* z = nullptr;
        z = exact_cast<DesiredType  >(m); if(z) return z;
        z = exact_cast<derived_datum>(m); if(z) return z;
        return z;
        }
};

struct any_member_test
{
    static void test_any_member();
    static void supplemental_test0();
    static void supplemental_test1();
    static void como_433_test();
};

int test_main(int, char*[])
{
    any_member_test::test_any_member();
    any_member_test::supplemental_test0();
    any_member_test::supplemental_test1();
    any_member_test::como_433_test();
    return 0;
}

void any_member_test::test_any_member()
{
    S s;
    s["i0"] = "999";
    LMI_TEST_EQUAL(s.i0, 999);

    s["i0"] = "077";   // Interpreted as decimal, not as octal.
    LMI_TEST_EQUAL(s.i0, 77);

    s["i0"] = "09";    // Valid decimal, not invalid octal.
    LMI_TEST_EQUAL(s.i0, 9);

    s["i1"] = "888000";
    LMI_TEST_EQUAL(s.i1, 888000);

//  s["i0"] = "999.9"; // Invalid integer-literal.
//  s["i1"] = "888e3"; // Invalid integer-literal.

    s["d0"] = "777";
    LMI_TEST_EQUAL(s.d0, 777);

    s["d0"] = "777.";
    LMI_TEST_EQUAL(s.d0, 777);

    s["d0"] = "777E3";
    LMI_TEST_EQUAL(s.d0, 777000);

    s["d0"] = ".777E3";
    LMI_TEST_EQUAL(s.d0, 777);

    s["s0"] = "hello";
    LMI_TEST_EQUAL(s.s0, "hello");

    std::stringstream ss;
    s.write(ss);
    LMI_TEST_EQUAL(ss.str(), "9 888000 777 hello");

    // Test const operator[]().

    S const s_const;
    LMI_TEST_EQUAL("0", s_const["i0"].str());

    // Test operator==() and operator!=().

    // operator==(): same objects, same members.

    LMI_TEST(s_const["i0"] == s_const["i0"]);
    LMI_TEST(s_const["d0"] == s_const["d0"]);
    LMI_TEST(s_const["s0"] == s_const["s0"]);
    LMI_TEST(s_const["x0"] == s_const["x0"]);

    // operator!=(): different objects.

    LMI_TEST(s["i0"] != s_const["i0"]);
    LMI_TEST(s["i1"] != s_const["i0"]);

    // operator==(): different members of same type.

    LMI_TEST(s_const["i0"] == s_const["i1"]);

    LMI_TEST(s["i0"] != s["i1"]);

    s["i0"] = s["i1"];
    LMI_TEST(s["i0"] == s["i1"]);

    s["i0"] = s_const["i0"];
    s["i1"] = s_const["i0"];
    LMI_TEST(s["i0"] == s["i1"]);

    // operator==(): different objects, same member.

    LMI_TEST(s["i0"] == s_const["i0"]);

    // operator==(): different objects, different members of same type.

    LMI_TEST(s["i1"] == s_const["i0"]);

    // operator==(): different objects, different member types.

    LMI_TEST(s_const["x0"] != s["i0"]);
    LMI_TEST(s_const["x0"] != s["d0"]);
    LMI_TEST(s_const["x0"] != s["s0"]);

    // operator==(): same object, different member types.

    LMI_TEST(s_const["x0"] != s_const["i0"]);

    // Make sure distinct object identity is preserved by assignment.

    s["i1"] = "909090";
    LMI_TEST(s["i1"] != s_const["i1"]);
    s["i1"] = s_const["i1"];
    LMI_TEST(s["i1"] == s_const["i1"]);
    s["i1"] = "909090";
    LMI_TEST(s["i1"] != s_const["i1"]);

    s["i0"] = s["i1"];
    LMI_TEST(s["i0"] == s["i1"]);
    s["i1"] = "9";
    LMI_TEST(s["i0"] != s["i1"]);

    // Test MemberSymbolTable::equals().

    LMI_TEST(s_const == s_const);
    LMI_TEST(s_const != s      );
    LMI_TEST(s       == s      );

    // Test MemberSymbolTable::assign().

    s.MemberSymbolTable<S>::assign(s_const);
    LMI_TEST(s_const == s      );

    // Test no-such-member diagnostic for both const and non-const
    // subscripting operators.

    std::string err("Symbol table for class ");
    err += lmi::TypeInfo(typeid(S)).Name();
    err += " ascribes no member named 'nonexistent'.";

    LMI_TEST_THROW(s_const["nonexistent"], std::runtime_error, err);
    LMI_TEST_THROW(s      ["nonexistent"], std::runtime_error, err);

    // Make sure numeric_io_cast is used for writing arithmetic types
    // to std::string, for any compiler that has IEC 60559 doubles and
    // can handle value_cast correctly (borland, for example, fails).

    s.d0 = std::exp(1.0);
    double d1 = *s["d0"].exact_cast<double>();
    LMI_TEST_EQUAL(numeric_io_cast<std::string>(d1), "2.718281828459045");
    LMI_TEST_EQUAL(s["d0"].str(), "2.718281828459045");

// COMPILER !! The borland compiler can build the program with the
// following line, but a run-time exception results.
//    std::cout << lmi::TypeInfo(typeid(s["i0"])) << std::endl;
// That crashes drmingw if it's installed as the jit debugger.
//
// However, no problem is observed with these two lines:
//    std::cout << lmi::TypeInfo(typeid(int)) << std::endl;
//    std::cout << lmi::TypeInfo(typeid(s["i0"])) << std::endl;
// the second of which is identical to the offending line above.
//
// Because the problem seems confined to borland tools, I will guess
// that it's simply a compiler defect.
}

void any_member_test::supplemental_test0()
{
    S s;
    X x;

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
    std::cout << "Testing member template exact_cast().\n";
    s.x0.set_str("Test 2");

    s["x0"].exact_cast<X>()->foo("example 0");

    X xx = *s["x0"].exact_cast<X>();
    xx.foo("example 1");

    (xx                      .*&X::foo)("example 2");
    (s["x0"].exact_cast<X>()->*&X::foo)("example 3");

    s["dd"].exact_cast<derived_datum>();
    s["dd"].exact_cast<derived_datum>()->base_function();
    }

    {
    std::cout << "Testing function template member_cast().\n";
    s.x0.set_str("Test 3");

    std::string err("Cannot cast from '");
    err += lmi::TypeInfo(typeid(int S::*)).Name();
    err += "' to '";
    err += lmi::TypeInfo(typeid(base_datum)).Name();
    err += "'.";

    LMI_TEST_THROW(member_cast<base_datum>(s["i0"]), std::runtime_error, err);

    base_datum* bp1 = member_cast<base_datum>(s["dd"]);
    LMI_TEST_EQUAL(1729, bp1->virtual_function());

    // This is appropriately forbidden: virtual_function() is not const.
//    base_datum const* bp2 = member_cast<base_datum>(s["dd"]);
//    LMI_TEST_EQUAL(1729, bp2->virtual_function());

    // COMPILER !! The borland compiler doesn't find the const overload;
    // presumably it is defective in this respect.
    S const& r = s;
    base_datum const* bp3 = member_cast<base_datum>(r["dd"]);
    LMI_TEST_EQUAL(7, bp3->sane);
    // This is appropriately forbidden: virtual_function() is not const.
//    LMI_TEST_EQUAL(1729, bp3->virtual_function());

    // If the original type is known, an explicit exact_cast is possible.
    LMI_TEST_EQUAL(1729, s["dd"].exact_cast<derived_datum>()->virtual_function());

    // If the original type isn't precisely unknown, but is known to
    // derive from a particular base class, then function template
    // member_cast() can reconstitute the actual pointer-to-member
    // type and safely return a pointer to the base class, provided
    // that it has sufficient knowledge of the inheritance hierarchy.
    // This obviates writing the type of the member pointed to.
    LMI_TEST_EQUAL(1729, member_cast<base_datum>(s["dd"])->virtual_function());

    // Of course, member_cast() should work with the exact type, too.
    LMI_TEST_EQUAL(1729, member_cast<derived_datum>(s["dd"])->virtual_function());

    // Function template is_reconstitutable_as() ascertains whether
    // the unknown original type is derived from a given base class.
    LMI_TEST(is_reconstitutable_as<base_datum>(s["dd"]));

    // is_reconstitutable_as() should not work with the exact type.
    LMI_TEST(!is_reconstitutable_as<derived_datum>(s["dd"]));
    }

    {
    std::cout << "Testing abstract-base-class member functions.\n";
    s.x0.set_str("Test 4");

    // Want to write something like:
    //   s["x0"].foo(); // no matching function for call to any_member<S>::foo()
    // but we need to supply the actual type:
    //   s["x0"].exact_cast<X>()->foo("example 0");
    // Yet we don't want to know the actual type of what 's["dd"]'
    // designates, much less exact_cast it to that type.

    derived_datum S::* pmd = &S::dd;
    // invalid conversion from `derived_datum S::*' to `base_datum S::*':
//    base_datum S::* pmdb = &S::dd;
    // invalid static_cast from type `derived_datum S::*' to type `base_datum S::*':
//    base_datum S::* pmdb = static_cast<base_datum S::*>(&S::dd);

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

    std::cout << std::endl;
    }
}

void any_member_test::supplemental_test1()
{
    T r0;
    LMI_TEST_EQUAL(0  , r0.i0);
    LMI_TEST_EQUAL(0  , r0.i1);
    LMI_TEST_EQUAL(0.0, r0.d0);
    LMI_TEST_EQUAL("" , r0.s0);

    // Test writing through a pointer to member.
    int T::*x = &T::i0;
    r0.*x = 5;
    LMI_TEST_EQUAL(5  , r0.i0);
    LMI_TEST_EQUAL(0  , r0.i1);
    LMI_TEST_EQUAL(0.0, r0.d0);
    double T::*y = &T::d0;
    r0.*y = 3.14159;
    LMI_TEST_EQUAL(3.14159, r0.d0);

    // Test writing through the map of ascribed member names.
    r0["i0"] = "999";
    r0["i1"] = "888000";
    r0["d0"] = "777";
    r0["q0"] = "8.125";
    r0["s0"] = "hello";
    LMI_TEST_EQUAL(999    , r0.i0);
    LMI_TEST_EQUAL(888000 , r0.i1);
    LMI_TEST_EQUAL(777.0  , r0.d0);
    LMI_TEST_EQUAL(8.125f , r0.q0);
    LMI_TEST_EQUAL("hello", r0.s0);

    T r1;
    r1["i0"] = "135";
    r1["i1"] = "246";
    r1["d0"] = "888";
    r1["s0"] = "world";
    LMI_TEST_EQUAL(135    , r1.i0);
    LMI_TEST_EQUAL(246    , r1.i1);
    LMI_TEST_EQUAL(888.0  , r1.d0);
    LMI_TEST_EQUAL("world", r1.s0);

    // Make sure that didn't affect the other instance of class T.
    LMI_TEST_EQUAL(999    , r0.i0);
    LMI_TEST_EQUAL(888e3  , r0.i1);
    LMI_TEST_EQUAL(777.0  , r0.d0);
    LMI_TEST_EQUAL("hello", r0.s0);

    r0["i0"] = "1234567";
    LMI_TEST_EQUAL(1234567, r0.i0);
    LMI_TEST_EQUAL(888e3  , r0.i1);
    LMI_TEST_EQUAL(777.0  , r0.d0);
    LMI_TEST_EQUAL("hello", r0.s0);

    // Make sure that didn't affect the other instance of class T.
    LMI_TEST_EQUAL(135    , r1.i0);
    LMI_TEST_EQUAL(246    , r1.i1);
    LMI_TEST_EQUAL(888.0  , r1.d0);
    LMI_TEST_EQUAL("world", r1.s0);

    // Works for base class Q too.
    T r2;
    LMI_TEST_EQUAL(0.0f, r2.q0);

    r2["q0"] = "123.456";
    LMI_TEST_EQUAL(123.456f, r2.q0);

    // Test equality operator.
    LMI_TEST(  r1["i0"] == r1["i0"]);
    LMI_TEST(  r1["i0"] != r0["i0"]);
    LMI_TEST(!(r1["i0"] == r0["i0"]));

    LMI_TEST(  r1["d0"] == r1["d0"]);
    LMI_TEST(  r1["d0"] != r0["d0"]);
    LMI_TEST(!(r1["d0"] == r0["d0"]));

    r1["d0"] = r0["d0"];
    LMI_TEST(  r1["d0"] == r1["d0"]);
    LMI_TEST(  r1["d0"] == r0["d0"]);
    LMI_TEST(!(r1["d0"] != r0["d0"]));

    // Test equality operator with empty strings--we once observed
    // a problem in that case.
    r1["s0"] = "";
    LMI_TEST(  r1["s0"] == r1["s0"]);
    LMI_TEST(  r1["s0"] != r0["s0"]);
    LMI_TEST(!(r1["s0"] != r1["s0"]));
    LMI_TEST(!(r1["s0"] == r0["s0"]));

    LMI_TEST_THROW(r2["unknown_member"], std::runtime_error, "");

    // Assigning a decimal-literal value to an integer isn't type
    // safe, and might require truncation, so it's forbidden.
    LMI_TEST_THROW(r2["i0"] = "888e3", std::invalid_argument, "");
    LMI_TEST_THROW(r2["i1"] = "999.9", std::invalid_argument, "");
}

// This test detects a problem with the original distribution of
// como-4.3.3 . I wrote to como on 2004-05-05T06:26Z, and got a
// fixed binary in his email of 2004-05-05T23:04Z. This test
// ensures that that fix is in use.

template<typename ClassType> class Bar
{
    std::map<int,int> m_; // Works OK if you remove this line.
};

struct R : public Bar<R>
{
    char   c; // Works OK if you remove this line...
    int    i; // ... or this one.
    double d;
};

void any_member_test::como_433_test()
{
    R s;
    LMI_TEST_EQUAL(&(s.c), &(s.*&R::c)); // Succeeds.
    LMI_TEST_EQUAL(&(s.i), &(s.*&R::i)); // Succeeds.
    LMI_TEST_EQUAL(&(s.d), &(s.*&R::d)); // Fails with original como.
}
