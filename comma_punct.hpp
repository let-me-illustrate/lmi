// Punctuate numbers with commas.
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

// $Id: comma_punct.hpp,v 1.2 2005-05-10 11:17:22 chicares Exp $

#ifndef comma_punct_hpp
#define comma_punct_hpp

#include "config.hpp"

#include <locale>
#include <string>

// INELEGANT !! It might be better to use std::setlocale() with a
// user-configurable locale, e.g.
//   setlocale(LC_ALL, "En_US");
// as a basis for customization, though perhaps any national locale
// would require further customization. However, this would appear not
// to work as of 2005-05-09 with gcc on the msw platform:
//   http://sf.net/mailarchive/message.php?msg_id=11704377

class comma_punct
    :public std::numpunct<char>
{
  protected:
    char do_thousands_sep()   const {return ',';}
    std::string do_grouping() const {return "\003";}
  public:
    comma_punct()
        :std::numpunct<char>()
        {}
};

#endif // comma_punct_hpp

