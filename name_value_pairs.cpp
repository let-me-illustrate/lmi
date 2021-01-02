// Manage files of name-value pairs.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "name_value_pairs.hpp"

#include "numeric_io_cast.hpp"
#include "value_cast.hpp"

#include <fstream>
#include <sstream>

name_value_pairs::name_value_pairs(std::string const& filename)
{
    std::ifstream is(filename.c_str());
    std::string line;
    while(std::getline(is, line))
        {
        std::istringstream iss_line(line);
        if(std::string::npos == line.find_first_of('='))
            {
            continue;
            }

        std::string name;
        std::getline(iss_line, name, '=');
        std::string value;
        std::getline(iss_line, value);
        map_[name] = value;
        }
}

std::string const& name_value_pairs::string_value(std::string const& key) const
{
    string_map::const_iterator i = map_.find(key);
    if(i == map_.end())
        {
        static std::string const empty_string = std::string();
        return empty_string;
        }
    else
        {
        return (*i).second;
        }
}

double name_value_pairs::numeric_value(std::string const& key) const
{
    double z = 0.0;
    std::string s(string_value(key));

    std::string::size_type last_nonblank = s.find_last_not_of(' ');
    s.resize(++last_nonblank);

    if(!s.empty())
        {
        z = numeric_io_cast<double>(s);
        }

    return z;
}

std::string name_value_pairs::string_numeric_value(std::string const& key) const
{
    return value_cast<std::string>(numeric_value(key));
}

std::map<std::string, std::string> const& name_value_pairs::map() const
{
    return map_;
}
