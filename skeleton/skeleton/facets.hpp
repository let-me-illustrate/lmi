// Custom facets.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: facets.hpp,v 1.1.1.1 2004-05-15 19:58:34 chicares Exp $

#ifndef facets_hpp
#define facets_hpp

#include "config.hpp"

#include <locale>

// Class blank_is_not_whitespace_ctype is intended for extracting
// std::strings from a stream without stopping at blanks. It is a
// derived work based on Dietmar Kuehl's article
//   http://groups.google.com/groups?selm=7vpld2%24jj%241%40nnrp1.deja.com
// which bears no copyright notice, as is usual in usenet.
//
// Ross Smith points out
//   http://groups.google.com/groups?selm=3820A76F.3952E808%40ihug.co.nz
// that ' ' can be used as an array index even if type 'char' is
// signed, because 2.2/3 requires it to have a positive value.
//
// GWC modified this code in 2004 only in trivial details, and in any
// later year shown above; any defect in it should not reflect on
// Dietmar Kuehl's or Ross Smith's reputation.

class blank_is_not_whitespace_ctype: public std::ctype<char>
{
  public:
    blank_is_not_whitespace_ctype()
        :std::ctype<char>(get_table())
        {}

  private:
    static std::ctype_base::mask const* get_table()
        {
        static std::ctype_base::mask m_[table_size];
        std::copy(classic_table(), classic_table() + table_size, m_);
        m_[' '] = static_cast<std::ctype_base::mask>
            (m_[' '] & ~std::ctype_base::space
            );
        return m_;
        }
};

#endif // facets_hpp

