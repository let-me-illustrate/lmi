// GPT server exception type.
//
// Copyright (C) 1998, 2001, 2005 Gregory W. Chicares.
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

// $Id: ihs_x_type.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef x_type_hpp
#define x_type_hpp

#include "config.hpp"

#include <stdexcept>
#include <string>

class x_product_rule_violated
    :public std::runtime_error
{
public:
    x_product_rule_violated(std::string const& what_arg)
    :std::runtime_error("Product rule violated: " + what_arg)
    {}
};

#endif // x_type_hpp

