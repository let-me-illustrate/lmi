// Symbolic member names: base class.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: any_entity.hpp,v 1.2 2006-08-12 17:16:33 chicares Exp $

#ifndef any_entity_hpp
#define any_entity_hpp

#include "config.hpp"

#include <ostream>
#include <string>
#include <typeinfo>

/// Abstract class any_entity specifies the interface required for
/// entities in the MVC Model. Class any_member is derived from this
/// class, but other implementations may be substituted.
///
/// Implicitly-declared special member functions do the right thing.

class any_entity
{
  public:
    virtual ~any_entity() {}

    any_entity& operator=(std::string const& s) {return assign(s);}

    virtual std::string str() const = 0;
    virtual std::type_info const& type() const = 0;

  private:
    virtual any_entity& assign(std::string const&) = 0;
};

#endif // any_entity_hpp

