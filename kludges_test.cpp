// TODO ?? Goofy compiler workarounds that should be expunged: unit test.
//
// Copyright (C) 2001, 2004, 2005 Gregory W. Chicares.
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

// $Id: kludges_test.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <fstream>
#include <ios>
#include <iostream>

int test_main(int, char*[])
{
#ifdef BC_BEFORE_5_5
    // See kludges_bc502.cpp .

    // The values of these constants are implementation defined. To
    // determine how this compiler specifies them, we run this program:
    // #include <fstream>
    // #include <iostream>
    //   // 'using namespace std;' would be an error here with
    //   // this compiler, as would 'std::cout' below; we write
    //   // as closely to the standard as the compiler permits.
    //
    // int main()
    // {
    //     cout << ios::adjustfield  << '\n';
    //     cout << ios::basefield    << '\n';
    //     cout << ios::floatfield   << '\n';
    //     cout << filebuf::openprot << '\n';
    // }
    // and obtain the answer
    //   14
    //   112
    //   6144
    //   384

    BOOST_TEST(  14 == std::ios::adjustfield      );
    BOOST_TEST( 112 == std::ios::basefield        );
    BOOST_TEST(6144 == std::ios::floatfield       );
    BOOST_TEST(  14 == std::ios_base::adjustfield );
    BOOST_TEST( 112 == std::ios_base::basefield   );
    BOOST_TEST(6144 == std::ios_base::floatfield  );
    BOOST_TEST( 384 == std::filebuf::openprot     );
#endif

// Conditionally write any tests for other compilers here.

    return 0;
}

