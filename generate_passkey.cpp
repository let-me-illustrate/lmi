// Moderately secure system date validation--passkey generator.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: generate_passkey.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "md5.hpp"
#include "secure_date.hpp" // md5_hex_string()

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>

//============================================================================
// Generate passkey as the md5 sum of the md5 sum of file 'validated.md5'.
// Iterating the md5 operation twice provides modest security: it's easy
// to forge if you know the trick; otherwise it's infeasibly hard, unless
// you use this program.
int main()
{
    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::FILE* md5sums_file = std::fopen("validated.md5", "rb");
    if(0 == md5sums_file)
        {
        std::cerr << "File 'validated.md5' not found.";
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
}

