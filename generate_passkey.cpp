// Moderately secure system date validation--passkey generator.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "authenticity.hpp"             // md5sum_file()
#include "main_common.hpp"
#include "md5.hpp"                      // md5_buffer(), md5_stream()
#include "md5sum.hpp"                   // md5_hex_string()

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>

/// Generate passkey as the md5 sum of the md5 sum of a file
/// containing md5 sums of secured files. Iterating the md5
/// operation twice provides modest security: it's easy to forge
/// if you know the trick (although this comment just might be
/// deliberately misleading); otherwise it's infeasibly hard,
/// unless you use this program (but then you might discover
/// other obstacles that are undocumented).

int try_main(int, char*[])
{
    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::FILE* md5sums_file = std::fopen(md5sum_file(), "rb");
    if(nullptr == md5sums_file)
        {
        std::cerr << "File '" << md5sum_file() << "' not found.\n";
        return EXIT_FAILURE;
        }
    md5_stream(md5sums_file, u_passkey);
    std::fclose(md5sums_file);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::vector<unsigned char> v(u_passkey, u_passkey + md5len);
    std::cout << md5_hex_string(v) << std::flush;
    return EXIT_SUCCESS;
}
