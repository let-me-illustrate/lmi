// Show progress on lengthy operations--unit test.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: progress_meter_test.cpp,v 1.1 2005-04-19 14:02:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter_cli.hpp"
#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

int test_main(int, char*[])
{
    int const max_count = 3;
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter(max_count, "Test")
        );
    for(int i = 0; i < max_count; ++i)
        {
        // Intended use: do some work, then update meter at end of block.
        BOOST_TEST(meter->reflect_progress());
        // Of course, you could do more work here, too, if you like.
        }

    BOOST_TEST_THROW(meter->reflect_progress(), std::logic_error, "");

    return EXIT_SUCCESS;
}

