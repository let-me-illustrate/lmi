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

// $Id: global_settings.cpp,v 1.4 2005-06-21 05:59:56 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#include "alert.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{
    void validate_path(std::string const& path, std::string const& name)
        {
        fs::path p(path);
        if(p.empty())
            {
            hobsons_choice()
                << name
                << " must not be empty."
                << LMI_FLUSH
                ;
            }
        if(!fs::exists(p))
            {
            hobsons_choice()
                << name
                << " '"
                << p.string()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        if(!fs::is_directory(p))
            {
            hobsons_choice()
                << name
                << " '"
                << p.string()
                << "' is not a directory."
                << LMI_FLUSH
                ;
            }
        }
} // Unnamed namespace.

// Initialize directory strings to ".", not an empty string. Reason:
// objects of the boost filesystem library's path class are created
// from these strings, which are not allowed to be passed to that
// library's directory_iterator ctor.

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
    validate_path(s, "Data directory");
    data_directory_ = s;
}

void global_settings::set_regression_test_directory(std::string const& s)
{
    validate_path(s, "Regression-test directory");
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

