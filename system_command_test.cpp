// Instruct the operating system to execute a command--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "system_command.hpp"

#include "miscellany.hpp"
#include "test_tools.hpp"

#include <cstdio>                       // remove()
#include <fstream>

int test_main(int, char*[])
{
    std::ofstream os0("eraseme", ios_out_trunc_binary());
    os0 << "0123456789abcdef0123456789abcdef  eraseme\n";
    os0.close();

    std::ofstream os1("eraseme.md5", ios_out_trunc_binary());
    os1 << "e87dfb7b7c7f87985d3eff4782c172b8  eraseme\n";
    os1.close();

    system_command("lmi_md5sum --check --status eraseme.md5");

    LMI_TEST_THROW
        (system_command("lmi_md5sum --check --status eraseme")
        ,std::runtime_error
        ,lmi_test::what_regex
            ("Exit code [0-9]* from command"
             " 'lmi_md5sum --check --status eraseme'."
            )
        );

#if !defined LMI_MSW
    lmi_test::what_regex bad_cmd("Exit code [0-9]* from command 'xyzzy'.");
#else  // defined LMI_MSW
    lmi_test::what_regex bad_cmd("Exit code 12345 from command 'xyzzy'.");
#endif // defined LMI_MSW
    LMI_TEST_THROW
        (system_command("xyzzy")
        ,std::runtime_error
        ,bad_cmd
        );

    std::remove("eraseme");
    std::remove("eraseme.md5");

    return 0;
}
