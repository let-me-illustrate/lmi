// Stream extraction operator for xmlwrapp.
//
// Copyright (C) 2002, 2005 Gregory W. Chicares.
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

// $Id: xmlwrapp_ex.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

// xmlwrapp-0.2.0 provides operator>>() but not operator<<(). Perhaps
// a future version will provide symmetric operators; until then, use
// this to read an istream into a string and then feed that string to
// xmlwrapp's tree parser.

#ifndef xmlwrapp_ex_hpp
#define xmlwrapp_ex_hpp

#include "config.hpp"

#include <istream>
#include <string>

std::string istream_to_string(std::istream& is);

#endif // xmlwrapp_ex_hpp

