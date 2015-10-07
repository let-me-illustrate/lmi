// Path utilities.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// $Id$

#ifndef path_utility_hpp
#define path_utility_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <ostream>
#include <string>

void LMI_SO initialize_filesystem(); // Listed first because of its importance.

std::string LMI_SO orthodox_filename
    (std::string const& original_filename
    );

fs::path LMI_SO serial_file_path
    (fs::path    const& exemplar
    ,std::string const& personal_name
    ,int                serial_number
    ,std::string const& extension
    );

fs::path LMI_SO unique_filepath
    (fs::path    const& original_filepath
    ,std::string const& supplied_extension
    );

void LMI_SO validate_directory
    (std::string const& directory
    ,std::string const& context
    );

void LMI_SO validate_filepath
    (std::string const& filepath
    ,std::string const& context
    );

inline std::ostream& operator<<(std::ostream& os, fs::path const& z)
{
    return os << z.string();
}

#endif // path_utility_hpp

