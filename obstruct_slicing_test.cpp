// Obstruct slicing.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#if defined __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // defined __BORLANDC__

#include "obstruct_slicing.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

#if !defined __BORLANDC__
#   include <boost/bind.hpp>
#endif // !defined __BORLANDC__

#include <string>

// Here are all the techniques discussed in the header.

// Technique 1.

class no_derive0
{
  protected:
    no_derive0() {}
};

class A0
    :private virtual no_derive0
{};

class A1
    :public A0
    ,private virtual no_derive0 // Could be accident rather than fraud.
{};

// Technique 2.

template<typename T>
class no_derive1
{
  protected:
    no_derive1() {}
    no_derive1(no_derive1 const&) {}
};

class B0
    :private virtual no_derive1<B0>
{};

class B1
    :public B0
    ,private virtual no_derive1<B1>
//    ,private virtual no_derive1<B0> // Either fraud, or really careless.
{};

// Technique 3.

class C0;

class C0_no_derive
{
    friend class C0;

  private:
    C0_no_derive() {}
    C0_no_derive(C0_no_derive const&) {}
};

class C0
    :public virtual C0_no_derive
{};

class C1
    :public C0
    ,public virtual C0_no_derive
{};

void test_all_techniques()
{
// Technique 1.
    A0 a0;
    A1 a1;     // This is allowed, but an error was desired.
    A1 a2(a1); // This is allowed, but an error was desired.

// Technique 2.
    B0 b0;
//    B1 b1;     // Error, as desired.
//    B1 b2(b1); // Error, as desired.

// Technique 3.
    C0 c0;
//    C1 c1;     // Error, as desired.
//    C1 c2(c1); // Error, as desired.
}

struct X0
{
    X0(): i(4), s("This is a test.") {}
    void foo();
    int i;
    std::string s;
};

struct X1
    :virtual private obstruct_slicing<X1>
{
    X1(): i(4), s("This is a test.") {}
    void foo();
    int i;
    std::string s;
};

template<typename T>
void mete(volatile int vi)
{
    T t0;
    T t1(t0);
    t0.i = vi;
    t0 = t1;
    vi = t1.i;
}

template<typename T>
void test_cost_of_obstruction(std::string const& s)
{
#if !defined __BORLANDC__
    std::cout
        << "  Class " << s << " has size " << sizeof(T) << '\n'
        << "  Speed test: \n      "
        << TimeAnAliquot(boost::bind(mete<T>, 0))
        << '\n'
        ;
#endif // !defined __BORLANDC__
}

int test_main(int, char*[])
{
    test_all_techniques();

    // Correctness is verifiable only at compile time. At run time,
    // there's nothing to test except the cost.

#if !defined __BORLANDC__
    test_cost_of_obstruction<X0>("X0");
    test_cost_of_obstruction<X1>("X1");
#endif // !defined __BORLANDC__

    return EXIT_SUCCESS;
}

