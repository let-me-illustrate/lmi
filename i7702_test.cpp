// 7702 (and 7702A) interest rates--unit test.
//
// Copyright (C) 2021 Gregory W. Chicares.
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

#include "i7702.hpp"

#include "materially_equal.hpp"
#include "test_tools.hpp"

class i7702_test
{
  public:
    static void test()
        {
        test0();
        test1();
        }

  private:
    static void test0();
    static void test1();
};

void i7702_test::test0()
{
    i7702 z
        {1           // length
        ,0.04        // A0
        ,0.06        // A1
        ,{0.03}      // Bgen
        ,{0.00}      // Bsep
        ,{0.02}      // Bflr
        ,{0.02}      // Bvlr
        ,{0.00}      // Cgen
        ,{0.00}      // Csep
        ,{0.00}      // Cflr
        ,{0.00}      // Cvlr
        ,{0.00}      // Dgen
        ,{0.00}      // Dsep
        ,{0.00}      // Dflr
        ,{0.00}      // Dvlr
        ,{0.0032737} // Em
        ,{1.0}       // use_gen
        ,{1.0}       // use_sep
        ,{1.0}       // use_flr
        ,{1.0}       // use_vlr
        };
    LMI_TEST(materially_equal(0.0032737, z.ig_usual()[0], 0.0000125));
}

void i7702_test::test1()
{
}

int test_main(int, char*[])
{
    i7702_test::test();
    return EXIT_SUCCESS;
}
