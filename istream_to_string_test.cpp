// Read stream into a string: unit test.
//
// Copyright (C) 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: istream_to_string_test.cpp,v 1.2 2008-09-21 18:23:34 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "istream_to_string.hpp"

#include "miscellany.hpp" // ios_in_binary()
#include "test_tools.hpp"
#include "timer.hpp"

#include <cstdio>         // std::remove()
#include <fstream>
#include <iterator>

template<typename Char_t, typename Traits, typename Allocator>
void istream_to_string_0
    (std::istream&                               is
    ,std::basic_string<Char_t,Traits,Allocator>& s
    )
{
    typedef std::basic_string<Char_t,Traits,Allocator> string_type;
    typedef std::istreambuf_iterator<Char_t,Traits> bisbi;
    string_type((bisbi(is)), bisbi()).swap(s);
    if(!is)
        {
        throw std::runtime_error("Unable to read stream into string.");
        }
}

template<typename Char_t, typename Traits, typename Allocator>
void istream_to_string_1
    (std::istream const&                         is
    ,std::basic_string<Char_t,Traits,Allocator>& s
    )
{
    typedef std::basic_string<Char_t,Traits,Allocator> string_type;
    std::basic_ostringstream<Char_t,Traits,Allocator> oss;
    if(is.rdbuf()->in_avail())
        {
        oss << is.rdbuf();
        }
    oss.str().swap(s);
    if(!is || !oss)
        {
        throw std::runtime_error("Unable to read stream into string.");
        }
}

template<char* filename>
void mete_0()
{
    std::ifstream ifs(filename, ios_in_binary());
    std::string s;
    istream_to_string_0(ifs, s);
}

template<char* filename>
void mete_1()
{
    std::ifstream ifs(filename, ios_in_binary());
    std::string s;
    istream_to_string_1(ifs, s);
}

namespace
{
char f10      [] = "eraseme.10";
char f100     [] = "eraseme.100";
char f1000    [] = "eraseme.1000";
char f10000   [] = "eraseme.10000";
char f100000  [] = "eraseme.100000";
char f1000000 [] = "eraseme.1000000";
char f10000000[] = "eraseme.10000000";
} // Unnamed namespace.

int test_main(int, char*[])
{
    // Test an empty file.

    char const* p = "/tmp/eraseme.empty";
    {
    std::ofstream ofs(p, ios_out_trunc_binary());
    }
    std::ifstream ifs(p, ios_in_binary());
    std::string s;
    istream_to_string_0(ifs, s);
    BOOST_TEST(s.empty());
    istream_to_string_1(ifs, s);
    BOOST_TEST(s.empty());
    std::remove(p);

    // Test speed.

    std::string const digits("0123456789");
    for(int i = 1; i < 10000000; i *= 10)
        {
        std::ostringstream oss;
        oss << "eraseme." << i * digits.size();
        std::ofstream ofs(oss.str().c_str(), ios_out_trunc_binary());
        for(int j = 0; j < i; ++j)
            {
            ofs << digits;
            }
        }

    std::cout << "\n  Speed tests for method 0...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_0<f10      >) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_0<f100     >) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_0<f1000    >) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_0<f10000   >) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_0<f100000  >) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_0<f1000000 >) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_0<f10000000>) << '\n'
        ;

    std::cout << "\n  Speed tests for method 1...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_1<f10      >) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_1<f100     >) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_1<f1000    >) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_1<f10000   >) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_1<f100000  >) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_1<f1000000 >) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_1<f10000000>) << '\n'
        ;

    std::remove(f10      );
    std::remove(f100     );
    std::remove(f1000    );
    std::remove(f10000   );
    std::remove(f100000  );
    std::remove(f1000000 );
    std::remove(f10000000);

    return 0;
}

