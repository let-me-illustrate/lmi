// Manage files of name-value pairs.
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

// $Id: name_value_pairs.hpp,v 1.1 2005-06-08 16:03:04 chicares Exp $

#ifndef name_value_pairs_hpp
#define name_value_pairs_hpp

#include "config.hpp"

// TODO ?? expunge
#include "alert.hpp"

#include <fstream>
#include <map>
#include <ostream> // std::flush() TODO ?? expunge eventually
#include <sstream>
#include <string>

// TODO ??
// Document.
//   'ini' resemblance
//   reason for skipping lines without '='
// Deliberately don't open file in binary mode.
// Don't check file existence--just return empty map if no such file?
//   then what if the filename is invalidly formed?
// Eventually write out of line, or make this function into a class.
// Duplicate names --> overwritten values.
// Don't strip trailing blanks--take care though with numeric conversions.
//   numeric conversions--eventually default to zero if value is blank?

inline std::map<std::string, std::string> read_name_value_pairs
    (std::string const& filename
    )
{
    std::ifstream is(filename.c_str());
    std::map<std::string, std::string> m;
    std::string line;
    while(std::getline(is, line))
        {
        std::istringstream iss_line(line, std::ios_base::in);
        if(std::string::npos == line.find_first_of('='))
            {
// TODO ?? expunge eventually
warning()
  << "line: " << "'" << line << "' skipped\n"
  << std::flush
  ;
            continue;
            }

        std::string name;
        std::getline(iss_line, name, '=');
        std::string value;
        std::getline(iss_line, value);
        m[name] = value;

// TODO ?? expunge eventually
warning()
  << "line: " << "'" << line << "'\n"
  << "  name: " << "'" << name << "'; "
  << "value: " << "'" << value << "'\n"
  << std::flush
  ;
        }
    return m;
}

#endif // name_value_pairs_hpp

