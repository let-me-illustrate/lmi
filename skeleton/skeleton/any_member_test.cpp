// Symbolic member names.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: any_member_test.cpp,v 1.1.1.1 2004-05-15 19:58:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "any_member.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <functional>
#include <iostream>
#include <ostream>

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

std::istream& operator>>(std::istream& is, X&      ) {return is;}
std::ostream& operator<<(std::ostream& os, X const& x) {return os << x.str() << '\n';}

class S
    :public MemberSymbolTable<S>
{
  public:
    S();
    std::ostream& write(std::ostream&);

  private:
    int i0;
    int i1;
    double d0;
    std::string s0;

  public: // TODO ?? Temporarily.
    X x0;
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
        << "\n"
        ;
    return os;
}

void test();

int test_main(int, char*[])
{
    std::free(0); // Force mpatrol to link.

    S s;
// TODO ?? Fails    s["i0"] = "999.9"; // Truncates to int.
    s["i0"] = "999";
// TODO ?? Fails    s["i1"] = "888e3"; // Converts to int.
    s["i1"] = "888000"; // Converts to int.
    s["d0"] = "777";   // Converts to double.
    s["s0"] = "hello";

    s.write(std::cout);
    std::cout << std::endl;

    // TODO ?? Need to be able to unify a subobject with a pmf, e.g.
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
// Expect '999 888000 777 hello'

    test();

    return 0;
}

void test()
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

    std::cout << std::endl;
    }

    // Want to write something like
    //   s["x0"].foo; // `class any_member<S>' has no member named `foo'
    // s["x0"].cast_to<X>().foo("example 0");
}

