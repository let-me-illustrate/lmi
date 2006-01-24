// Global settings.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: global_settings.cpp,v 1.12 2006-01-24 07:11:47 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#include "alert.hpp"
#include "path_utility.hpp"

/// Initialize directory paths to ".", not an empty string. Reason:
/// objects of the boost filesystem library's path class are created
/// from these strings, which, if the strings were empty, would trigger
/// exceptions when passed to that library's directory_iterator ctor.
///
/// 6.7/4 might seem to permit instance() to call this ctor before the
/// first statement of main(); however, that is actually not permitted
/// because it wouldn't meet the conditions of 3.6.2/2; cf.
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/f322587e93ce83fb
/// Even if that were permitted, or if instance() is negligently
/// called too early, then setting a default policy later with
/// default_name_check() will throw an exception, so there's no need
/// to guard against such things here in any event.

global_settings::global_settings()
    :mellon_                    (false)
    ,ash_nazg_                  (false)
    ,custom_io_0_               (false)
    ,regression_testing_        (false)
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

void global_settings::set_regression_testing(bool b)
{
    regression_testing_ = b;
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

bool global_settings::regression_testing() const
{
    return regression_testing_;
}

fs::path const& global_settings::data_directory() const
{
    return data_directory_;
}

fs::path const& global_settings::regression_test_directory() const
{
    return regression_test_directory_;
}

