// Data directory.
//
// Copyright (C) 2003, 2005, 2006 Gregory W. Chicares.
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

// $Id: data_directory.cpp,v 1.4.2.1 2006-11-08 00:42:55 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "data_directory.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

//============================================================================
std::string AddDataDir(std::string const& a_filename)
{
    fs::path path(a_filename);
    LMI_ASSERT(a_filename == path.leaf());

    path = global_settings::instance().data_directory() / path;
    return path.string();
}

//============================================================================
std::string AddXmlDirectory(std::string const& a_filename)
{
    try
    {
        fs::path path(a_filename);
        if(!path.is_complete())
            {
            path = configurable_settings::instance().xslt_directory() / path;
            if(!path.is_complete())
                {
                path = global_settings::instance().data_directory() / path;
                }
            }

        return path.string();
    }
    catch(fs::filesystem_error const& e)
    {
        std::ostringstream oss;
        oss
            << "Invalid filename '"
            << a_filename
            << "' or directory '"
            << configurable_settings::instance().xslt_directory()
            << "', or '"
            << global_settings::instance().data_directory().string()
            << "'. "
            << e.what()
            ;
        throw std::runtime_error(oss.str());
    }
}
