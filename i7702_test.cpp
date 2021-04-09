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
#include "timer.hpp"

class i7702_test
{
  public:
    static void test()
        {
        test0();
        test1();
        assay_speed();
        }

  private:
    static i7702 bland0();
    static i7702 bland1();
    static i7702 bland99();

    static void test0();
    static void test1();
    static void assay_speed();

    static void mete_construct();
    static void mete_initialize();
    static void mete_assert();
};

/// Bland initial values: a single duration; Em rounded down.

i7702 i7702_test::bland0()
{
    return
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
}

/// Bland initial values: two durations; Em rounded up.

i7702 i7702_test::bland1()
{
    return
        {2                      // length
        ,0.04                   // A0
        ,0.06                   // A1
        ,{0.03     , 0.03     } // Bgen
        ,{0.00     , 0.00     } // Bsep
        ,{0.02     , 0.02     } // Bflr
        ,{0.02     , 0.02     } // Bvlr
        ,{0.00     , 0.00     } // Cgen
        ,{0.00     , 0.00     } // Csep
        ,{0.00     , 0.00     } // Cflr
        ,{0.00     , 0.00     } // Cvlr
        ,{0.00     , 0.00     } // Dgen
        ,{0.00     , 0.00     } // Dsep
        ,{0.00     , 0.00     } // Dflr
        ,{0.00     , 0.00     } // Dvlr
        ,{0.0032738, 0.0032738} // Em
        ,{1.0      , 1.0      } // use_gen
        ,{1.0      , 1.0      } // use_sep
        ,{1.0      , 1.0      } // use_flr
        ,{1.0      , 1.0      } // use_vlr
        };
}

/// Bland initial values: ninety-nine durations; Em rounded up.

i7702 i7702_test::bland99()
{
    constexpr int length {99};
    return i7702
        (length                // length
        ,0.04                  // A0
        ,0.06                  // A1
        ,std::vector<double>(length, 0.03     ) // Bgen
        ,std::vector<double>(length, 0.00     ) // Bsep
        ,std::vector<double>(length, 0.02     ) // Bflr
        ,std::vector<double>(length, 0.02     ) // Bvlr
        ,std::vector<double>(length, 0.00     ) // Cgen
        ,std::vector<double>(length, 0.00     ) // Csep
        ,std::vector<double>(length, 0.00     ) // Cflr
        ,std::vector<double>(length, 0.00     ) // Cvlr
        ,std::vector<double>(length, 0.00     ) // Dgen
        ,std::vector<double>(length, 0.00     ) // Dsep
        ,std::vector<double>(length, 0.00     ) // Dflr
        ,std::vector<double>(length, 0.00     ) // Dvlr
        ,std::vector<double>(length, 0.0032738) // Em
        ,std::vector<double>(length, 1.0      ) // use_gen
        ,std::vector<double>(length, 1.0      ) // use_sep
        ,std::vector<double>(length, 1.0      ) // use_flr
        ,std::vector<double>(length, 1.0      ) // use_vlr
        );
}

void i7702_test::test0()
{
    i7702 z {bland0()};
    LMI_TEST(materially_equal(0.0032737, z.ig_usual()[0], 0.0000125));
    LMI_TEST(materially_equal(0.00327373978219886374239, z.ig_usual()[0]));

    // Class i7702 is copy- and move-constructible, but not assignable
    // because of const data members.
    i7702 x = z;
    i7702 y = std::move(z);
//  x = y;            // operator=(i7702 const&) implicitly deleted
//  x = std::move(y); // operator=(i7702&&)      implicitly deleted
}

void i7702_test::test1()
{
    i7702 z {bland1()};
    z.Cgen_ = {0.05, 0.00};
    z.initialize();
    LMI_TEST(materially_equal(0.00407412378364830143895, z.ig_usual()[0]));
    LMI_TEST(materially_equal(0.0032738                , z.ig_usual()[1]));

    // Test idempotence.
    z.initialize();
    LMI_TEST(materially_equal(0.00407412378364830143895, z.ig_usual()[0]));
    LMI_TEST(materially_equal(0.0032738                , z.ig_usual()[1]));
}

void i7702_test::assay_speed()
{
    std::cout
        << "\n  Speed tests..."
        << "\n  Construct : " << TimeAnAliquot(mete_construct)
        << "\n  Initialize: " << TimeAnAliquot(mete_initialize)
        << "\n  Assert    : " << TimeAnAliquot(mete_assert)
        << std::endl
        ;
}

void i7702_test::mete_construct()
{
    i7702 z {bland99()};
}

void i7702_test::mete_initialize()
{
    static i7702 z {bland99()};
    z.initialize();
}

void i7702_test::mete_assert()
{
    static i7702 z {bland99()};
    z.assert_preconditions();
}

int test_main(int, char*[])
{
    i7702_test::test();
    return EXIT_SUCCESS;
}
