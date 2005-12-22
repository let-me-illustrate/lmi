// Path utilities.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: path_utility.hpp,v 1.3 2005-12-22 13:59:49 chicares Exp $

#ifndef path_utility_hpp
#define path_utility_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

std::string LMI_SO serialize_extension
    (int                serial_number
    ,std::string const& extension
    );

fs::path LMI_SO serialized_file_path
    (fs::path const&    exemplar
    ,int                serial_number
    ,std::string const& extension
    );

fs::path LMI_SO unique_filepath
    (fs::path const&    original_filepath
    ,std::string const& extension
    );

#endif // path_utility_hpp

