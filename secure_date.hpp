// Moderately secure system date validation--tells whether system has expired.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: secure_date.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef secure_date_hpp
#define secure_date_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "expimp.hpp"

#ifndef BC_BEFORE_5_5
#   include <boost/static_assert.hpp>
#endif // not old borland compiler

#include <climits> // CHAR_BIT
#include <string>
#include <vector>

#ifndef BC_BEFORE_5_5
    // The gnu libc md5 implementation seems to assume this:
    BOOST_STATIC_ASSERT(8 == CHAR_BIT || 16 == CHAR_BIT);
#endif // not old borland compiler
    // so md5 output is 128 bits == 16 8-bit bytes or 8 16-bit bytes:
enum {md5len = 128 / CHAR_BIT};

class LMI_EXPIMP secure_date
    :public calendar_date
{
  public:

    enum
        {ill_formed_passkey = 1
        ,date_out_of_range
        ,md5sum_error
        ,incorrect_passkey
        };

    static secure_date* instance();
    static int validate
        (calendar_date const& candidate
        ,std::string const& path = ""
        );

  private:
    secure_date();
    secure_date(secure_date const&);
    secure_date& operator=(secure_date const&);

    static secure_date* instance_;
};

// Return hex representation of an md5 sum as a string.
std::string md5_hex_string(std::vector<unsigned char> const&);

#endif // secure_date_hpp

