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

// $Id: secure_date.hpp,v 1.4 2005-07-06 00:48:24 chicares Exp $

#ifndef secure_date_hpp
#define secure_date_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "expimp.hpp"
#include "obstruct_slicing.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility.hpp>

#include <climits> // CHAR_BIT
#include <string>
#include <vector>

// The gnu libc md5 implementation seems to assume this:
BOOST_STATIC_ASSERT(8 == CHAR_BIT || 16 == CHAR_BIT);
// so md5 output is 128 bits == 16 8-bit bytes or 8 16-bit bytes:
enum {md5len = 128 / CHAR_BIT};

class LMI_EXPIMP secure_date
    :public calendar_date
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<secure_date>
{
  public:
    static secure_date* instance();
    static std::string validate
        (calendar_date const& candidate
        ,fs::path const&      path
        );

  private:
    secure_date();

    static secure_date* instance_;
};

// Return hex representation of an md5 sum as a string.
std::string md5_hex_string(std::vector<unsigned char> const&);

#endif // secure_date_hpp

