// Sample input class for wx data-transfer demonstration.
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

// $Id: input_datum.hpp,v 1.1.1.1 2004-05-15 19:58:43 chicares Exp $

#ifndef input_datum_hpp
#define input_datum_hpp

#include "config.hpp"

#include <istream>
#include <ostream>

// TODO ?? Should this be a base class? Else rename it.
// TODO ?? Public data members obviously should be rethought.
class input_datum
{
  public:
    input_datum();
    explicit input_datum(bool datum);

    bool datum_;
    bool enabled_;
};

inline std::istream& operator>>(std::istream& is, input_datum& z)
{
    is >> z.datum_;
    return is;
}

inline std::ostream& operator<<(std::ostream& os, input_datum const& z)
{
    os << z.datum_;
    return os;
}

#endif // input_datum_hpp

