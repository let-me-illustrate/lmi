// Global settings.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: global_settings.cpp,v 1.6 2005-06-21 23:48:35 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>
#include <stdexcept>

namespace
{
    /// validate_directory() throws an informative exception if its
    /// 'd' argument does not name a valid directory.
    ///
    /// 'd': directory-name to be validated.
    ///
    /// 'context': semantic description of the directory to be named;
    /// used in the exception report.
    ///
    /// Although a std::invalid_argument exception would seem more
    /// fitting in the context of this function, in the global context
    /// 'd' may be specified by users, so std::runtime_error is
    /// preferable.
    ///
    /// Exceptions thrown from the boost filesystem library on path
    /// assignment are caught in order to rethrow with 'context'
    /// prepended.

    void validate_directory(std::string const& d, std::string const& context)
        {
        fs::path path;
        try
            {
            path = d;
            }
        catch(fs::filesystem_error const& e)
            {
            std::ostringstream oss;
            oss
                << context
                << ": "
                << e.what()
                ;
            throw std::runtime_error(oss.str());
            }

        if(path.empty())
            {
            std::ostringstream oss;
            oss
                << context
                << " must not be empty."
                ;
            throw std::runtime_error(oss.str());
            }
        if(!fs::exists(path))
            {
            std::ostringstream oss;
            oss
                << context
                << " '"
                << path.string()
                << "' not found."
                ;
            throw std::runtime_error(oss.str());
            }
        if(!fs::is_directory(path))
            {
            std::ostringstream oss;
            oss
                << context
                << " '"
                << path.string()
                << "' is not a directory."
                ;
            throw std::runtime_error(oss.str());
            }
        }
} // Unnamed namespace.

/// Initialize directory strings to ".", not an empty string. Reason:
/// objects of the boost filesystem library's path class are created
/// from these strings, which, if the strings were empty, would trigger
/// exceptions when passed to that library's directory_iterator ctor.

global_settings::global_settings()
    :mellon_                    (false)
    ,ash_nazg_                  (false)
    ,custom_io_0_               (false)
    ,data_directory_            (".")
    ,regression_test_directory_ (".")
{}

global_settings::~global_settings()
{}

global_settings& global_settings::instance()
{
    static global_settings z;
    return z;
}

bool global_settings::regression_testing() const
{
    return regression_test_directory_.empty();
}

void global_settings::set_mellon(bool b)
{
    mellon_ = b;
}

void global_settings::set_ash_nazg(bool b)
{
    ash_nazg_ = b;
    if(b)
        {
        // 'ash_nazg' implies 'mellon'.
        mellon_ = b;
        }
}

void global_settings::set_custom_io_0(bool b)
{
    custom_io_0_ = b;
}

void global_settings::set_data_directory(std::string const& s)
{
    validate_directory(s, "Data directory");
    data_directory_ = s;
}

void global_settings::set_regression_test_directory(std::string const& s)
{
    validate_directory(s, "Regression-test directory");
    regression_test_directory_ = s;
}

bool global_settings::mellon() const
{
    return mellon_;
}

bool global_settings::ash_nazg() const
{
    return ash_nazg_;
}

bool global_settings::custom_io_0() const
{
    return custom_io_0_;
}

std::string const& global_settings::data_directory() const
{
    return data_directory_;
}

std::string const& global_settings::regression_test_directory() const
{
    return regression_test_directory_;
}

