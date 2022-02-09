// Death benefit option (DBO) rules--unit test.
//
// Copyright (C) 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "dbo_rules.hpp"

#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "test_tools.hpp"
#include "timer.hpp"

#include <vector>

int test_main(int, char*[])
{
    LMI_TEST( dbo_at_issue_is_allowed(mce_dbopt("A"  )));
    LMI_TEST( dbo_at_issue_is_allowed(mce_dbopt("B"  )));
    LMI_TEST(!dbo_at_issue_is_allowed(mce_dbopt("ROP")));
    LMI_TEST( dbo_at_issue_is_allowed(mce_dbopt("MDB")));

    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("A"  ), mce_dbopt("A"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("A"  ), mce_dbopt("B"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("A"  ), mce_dbopt("ROP")));
    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("A"  ), mce_dbopt("MDB")));
    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("B"  ), mce_dbopt("A"  )));
    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("B"  ), mce_dbopt("B"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("B"  ), mce_dbopt("ROP")));
    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("B"  ), mce_dbopt("MDB")));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("ROP"), mce_dbopt("A"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("ROP"), mce_dbopt("B"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("ROP"), mce_dbopt("ROP")));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("ROP"), mce_dbopt("MDB")));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("MDB"), mce_dbopt("A"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("MDB"), mce_dbopt("B"  )));
    LMI_TEST(!dbo_transition_is_allowed(mce_dbopt("MDB"), mce_dbopt("ROP")));
    LMI_TEST( dbo_transition_is_allowed(mce_dbopt("MDB"), mce_dbopt("MDB")));

    {
    std::vector<mce_dbopt> v = {};
    LMI_TEST_THROW
        (dbo_sequence_is_allowed(v)
        ,std::runtime_error
        ,"DBO must not be empty."
        );
    }

    {
    std::vector<mce_dbopt> v = {mce_dbopt("MDB")};
    LMI_TEST(dbo_sequence_is_allowed(v));
    }

    {
    std::vector<mce_dbopt> v = {mce_dbopt("ROP")};
    LMI_TEST_THROW
        (dbo_sequence_is_allowed(v)
        ,std::runtime_error
        ,"Forbidden initial DBO 'ROP'."
        );
    }

    {
    std::vector<mce_dbopt> v =
        {mce_dbopt("B")
        ,mce_dbopt("A")
        ,mce_dbopt("MDB")
        };
    LMI_TEST(dbo_sequence_is_allowed(v));
    }

    {
    std::vector<mce_dbopt> v =
        {mce_dbopt("A")
        ,mce_dbopt("B")
        };
    LMI_TEST_THROW
        (dbo_sequence_is_allowed(v)
        ,std::runtime_error
        ,"Forbidden DBO change from 'A' to 'B' after 1 years."
        );
    }

    {
    std::vector<mce_dbopt> v =
        {mce_dbopt("B")
        ,mce_dbopt("B")
        ,mce_dbopt("B")
        ,mce_dbopt("A")
        ,mce_dbopt("A")
        ,mce_dbopt("MDB")
        ,mce_dbopt("MDB")
        ,mce_dbopt("ROP")
        ,mce_dbopt("MDB")
        };
    LMI_TEST_THROW
        (dbo_sequence_is_allowed(v)
        ,std::runtime_error
        ,"Forbidden DBO change from 'MDB' to 'ROP' after 7 years."
        );
    }

    {
    std::vector<mce_dbopt> v =
        {mce_dbopt("B")
        ,mce_dbopt("B")
        ,mce_dbopt("B")
        ,mce_dbopt("A")
        ,mce_dbopt("A")
        ,mce_dbopt("MDB")
        };
    v.resize(100, mce_dbopt("MDB"));
    auto f = [&] {dbo_sequence_is_allowed(v);};
    std::cout
        << "\n  Speed test..."
        << "\n  DBO changes: " << TimeAnAliquot(f)
        << std::endl
        ;
    (dbo_sequence_is_allowed(v));
    }

    return 0;
}
