// Global settings.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "global_settings.hpp"

#include "alert.hpp"
#include "handle_exceptions.hpp"
#include "path_utility.hpp"

#include <boost/filesystem/operations.hpp>

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
    ,pyx_                       ("")
    ,custom_io_0_               (false)
    ,regression_testing_        (false)
    ,data_directory_            (fs::system_complete("."))
    ,regression_test_directory_ (fs::system_complete("."))
{}

global_settings::~global_settings()
{}

global_settings& global_settings::instance()
{
    try
        {
        static global_settings z;
        return z;
        }
    catch(...)
        {
        report_exception();
        fatal_error() << "Instantiation failed." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
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

void global_settings::set_pyx(std::string const& s)
{
    pyx_ = s;
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
    data_directory_ = fs::system_complete(s);
}

void global_settings::set_regression_test_directory(std::string const& s)
{
    validate_directory(s, "Regression-test directory");
    regression_test_directory_ = fs::system_complete(s);
}

bool global_settings::mellon() const
{
    return mellon_;
}

bool global_settings::ash_nazg() const
{
    return ash_nazg_;
}

std::string const& global_settings::pyx() const
{
    return pyx_;
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

