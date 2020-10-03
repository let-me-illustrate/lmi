// Cache class instances constructed from files--unit test.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "cache_file_reads.hpp"

#include "istream_to_string.hpp"
#include "miscellany.hpp"               // ios_in_binary()
#include "path.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <fstream>

class X
    :public cache_file_reads<X>
{
  public:
    X() = default;
    X(std::string const& filename)
        {
        std::ifstream ifs(filename, ios_in_binary());
        istream_to_string(ifs, s_);
        }

    std::string const& s() const
        {
        return s_;
        }

  private:
    std::string s_;
};

class cache_file_reads_test
{
  public:
    static void test()
        {
        test_preconditions();
        assay_speed();
        }

  private:
    static void test_preconditions();
    static void assay_speed();

    static void mete_uncached();
    static void mete_cached  ();
};

void cache_file_reads_test::test_preconditions()
{
    // X() and X(filename) are required.
    X x0;
    X x1("sample.ill");

    // The cache is accessible with or without an object.
    BOOST_TEST_EQUAL
        (x0.read_via_cache("sample.ill")->s()
        ,X::read_via_cache("sample.ill")->s()
        );

    // The file must exist.
    BOOST_TEST_THROW
        (X::read_via_cache("no_such_file")
        ,boost::filesystem::filesystem_error
        ,lmi_test::what_regex("no_such_file")
        );
}

void cache_file_reads_test::assay_speed()
{
    std::cout
        << "\n  Speed tests..."
        << "\n  Uncached: " << TimeAnAliquot(mete_uncached)
        << "\n  Cached  : " << TimeAnAliquot(mete_cached  )
        << std::endl
        ;
}

void cache_file_reads_test::mete_uncached()
{
    X const x("sample.ill");
    std::string::size_type volatile z = x.s().size();
    stifle_warning_for_unused_value(z);
}

void cache_file_reads_test::mete_cached()
{
    X const& x(*X::read_via_cache("sample.ill"));
    std::string::size_type volatile z = x.s().size();
    stifle_warning_for_unused_value(z);
}

int test_main(int, char*[])
{
    cache_file_reads_test::test();

    return EXIT_SUCCESS;
}
