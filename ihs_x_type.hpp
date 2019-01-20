// GPT server exception type.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef ihs_x_type_hpp
#define ihs_x_type_hpp

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

#endif // ihs_x_type_hpp

