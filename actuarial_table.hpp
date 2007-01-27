// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: actuarial_table.hpp,v 1.3 2007-01-27 00:00:51 wboutin Exp $

#ifndef actuarial_table_hpp
#define actuarial_table_hpp

#include "config.hpp"

#include <string>
#include <vector>

std::vector<double> actuarial_table
    (std::string const& table_filename
    ,int                table_number
    ,int                age
    ,int                len
    );

#endif // actuarial_table_hpp

