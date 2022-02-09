// Read stream into a string: unit test.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "istream_to_string.hpp"

#include "miscellany.hpp"               // ios_in_binary()
#include "test_tools.hpp"
#include "timer.hpp"

#include <cstddef>                      // size_t
#include <cstdio>
#include <fstream>
#include <iterator>                     // istreambuf_iterator

template<typename Char_t, typename Traits, typename Allocator>
void istream_to_string_1
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
void istream_to_string_2
    (std::istream const&                         is
    ,std::basic_string<Char_t,Traits,Allocator>& s
    )
{
    std::basic_ostringstream<Char_t,Traits,Allocator> oss;
    if(Traits::eof() != is.rdbuf()->sgetc())
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
std::string mete_0()
{
    std::ifstream ifs(filename, ios_in_binary());
    std::string s;
    istream_to_string(ifs, s);
    return s;
}

template<char* filename>
std::string mete_1()
{
    std::ifstream ifs(filename, ios_in_binary());
    std::string s;
    istream_to_string_1(ifs, s);
    return s;
}

template<char* filename>
std::string mete_2()
{
    std::ifstream ifs(filename, ios_in_binary());
    std::string s;
    istream_to_string_2(ifs, s);
    return s;
}

/// This is a naive implementation that doesn't check return codes.

template<char* filename>
std::string mete_3()
{
    std::FILE* fp = std::fopen(filename, "rb");
    static std::size_t const buffer_size = 4096;
    static char buffer[buffer_size];
    std::string s;
    while(!std::feof(fp))
        {
        std::size_t bytes_read = std::fread(buffer, 1, buffer_size, fp);
        if(0 != bytes_read)
        s.append(buffer, bytes_read);
        }
    std::fclose(fp);
    return s;
}

namespace
{
std::string const alphabet("abcdefghijklmnopqrstuvwxyz\n");

char empty_file   [] = "eraseme.empty";
char nonempty_file[] = "eraseme.nonempty";

char f10          [] = "eraseme.10";
char f100         [] = "eraseme.100";
char f1000        [] = "eraseme.1000";
char f10000       [] = "eraseme.10000";
char f100000      [] = "eraseme.100000";
char f1000000     [] = "eraseme.1000000";
char f10000000    [] = "eraseme.10000000";
} // Unnamed namespace.

void test_empty_file()
{
    std::ofstream ofs(empty_file, ios_out_trunc_binary());
    ofs.close();

    LMI_TEST(mete_0<empty_file>().empty());
    LMI_TEST(mete_1<empty_file>().empty());
    LMI_TEST(mete_2<empty_file>().empty());
    LMI_TEST(mete_3<empty_file>().empty());

    std::remove(empty_file);
}

void test_nonempty_file()
{
    std::ofstream ofs(nonempty_file, ios_out_trunc_binary());
    ofs << alphabet;
    ofs.close();

    LMI_TEST_EQUAL(alphabet, mete_0<nonempty_file>());
    LMI_TEST_EQUAL(alphabet, mete_1<nonempty_file>());
    LMI_TEST_EQUAL(alphabet, mete_2<nonempty_file>());
    LMI_TEST_EQUAL(alphabet, mete_3<nonempty_file>());

    std::remove(nonempty_file);
}

void test_speed()
{
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

    // Read each file once to flush disk cache.
    mete_0<f10      >();
    mete_0<f100     >();
    mete_0<f1000    >();
    mete_0<f10000   >();
    mete_0<f100000  >();
    mete_0<f1000000 >();
    mete_0<f10000000>();

    // Limit each measurement, to make the unit test acceptably fast.
    double const t = 0.01;

    std::cout << "\n  Speed tests for production code...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_0<f10      >, t) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_0<f100     >, t) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_0<f1000    >, t) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_0<f10000   >, t) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_0<f100000  >, t) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_0<f1000000 >, t) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_0<f10000000>, t) << '\n'
        ;

    std::cout << "\n  Speed tests for istreambuf_iterator...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_1<f10      >, t) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_1<f100     >, t) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_1<f1000    >, t) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_1<f10000   >, t) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_1<f100000  >, t) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_1<f1000000 >, t) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_1<f10000000>, t) << '\n'
        ;

    std::cout << "\n  Speed tests for extraction from streambuf...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_2<f10      >, t) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_2<f100     >, t) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_2<f1000    >, t) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_2<f10000   >, t) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_2<f100000  >, t) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_2<f1000000 >, t) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_2<f10000000>, t) << '\n'
        ;

    std::cout << "\n  Speed tests for C equivalent...\n"
        << "       10 bytes: " << TimeAnAliquot(mete_3<f10      >, t) << '\n'
        << "      100 bytes: " << TimeAnAliquot(mete_3<f100     >, t) << '\n'
        << "     1000 bytes: " << TimeAnAliquot(mete_3<f1000    >, t) << '\n'
        << "    10000 bytes: " << TimeAnAliquot(mete_3<f10000   >, t) << '\n'
        << "   100000 bytes: " << TimeAnAliquot(mete_3<f100000  >, t) << '\n'
        << "  1000000 bytes: " << TimeAnAliquot(mete_3<f1000000 >, t) << '\n'
        << " 10000000 bytes: " << TimeAnAliquot(mete_3<f10000000>, t) << '\n'
        ;

    std::remove(f10      );
    std::remove(f100     );
    std::remove(f1000    );
    std::remove(f10000   );
    std::remove(f100000  );
    std::remove(f1000000 );
    std::remove(f10000000);
}

int test_main(int, char*[])
{
    test_empty_file();
    test_nonempty_file();
    test_speed();

    return 0;
}
