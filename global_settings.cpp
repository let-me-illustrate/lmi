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

// $Id: global_settings.cpp,v 1.3 2005-06-21 05:26:39 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#if 0
// Code copied from elsewhere that belongs here.

    // TODO ?? Path validation belongs in class global_settings.
    if(0 == data_directory.size())
        {
        data_directory = ".";
        }
    fs::path path(data_directory);
    if(!fs::exists(path) || !fs::is_directory(path))
        {
        hobsons_choice()
            << "Data directory '"
            << path.string()
            << "' not found."
            << LMI_FLUSH
            ;
        }
[Not yet done]

Elsewhere was formerly found code like
                global_settings::instance().set_ash_nazg(true);
                global_settings::instance().set_mellon(true);
but ash_nazg should imply mellon, and the place to do that is here.
[Done]
#endif // 0

// Initialize directory strings to ".", not an empty string. Reason:
// objects of the boost filesystem library's path class are created
// from these strings, and that class deliberately rejects empty
// strings.

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
    data_directory_ = s;
}

void global_settings::set_regression_test_directory(std::string const& s)
{
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

