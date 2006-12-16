// Permit running the system iff data files and date are valid.
//
// Copyright (C) 2003, 2005, 2006 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: secure_date.hpp,v 1.11 2006-12-16 17:08:15 chicares Exp $

#ifndef secure_date_hpp
#define secure_date_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "obstruct_slicing.hpp"

#include <boost/filesystem/path.hpp>

#if !defined __BORLANDC__
#include <boost/static_assert.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) /##/
#endif // Defined __BORLANDC__ .

#include <boost/utility.hpp>

#include <climits> // CHAR_BIT
#include <string>
#include <vector>

// The gnu libc md5 implementation seems to assume this:
BOOST_STATIC_ASSERT(8 == CHAR_BIT || 16 == CHAR_BIT);
// so md5 output is 128 bits == 16 8-bit bytes or 8 16-bit bytes:
enum {md5len = 128 / CHAR_BIT};

/// Permit running the system iff data files and date are valid.
///
/// Implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.

class SecurityValidator
    :public calendar_date
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<SecurityValidator>
{
    friend class PasskeyTest;

  public:
    static SecurityValidator& Instance();
    static std::string Validate
        (calendar_date const& candidate
        ,fs::path const&      data_path
        );

  private:
    SecurityValidator();
    ~SecurityValidator();

    static void PurgeCache();
};

/// Hex representation of an md5 sum as a string.

std::string md5_hex_string(std::vector<unsigned char> const&);

/// Name of file containing md5sums of secured files.

inline char const* md5sum_file() {return "validated.md5";}

#endif // secure_date_hpp

