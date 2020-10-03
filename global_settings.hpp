// Global settings.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef global_settings_hpp
#define global_settings_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "path.hpp"
#include "so_attributes.hpp"

#include <string>

/// Design notes for class global_settings.
///
/// This is a simple Meyers singleton, with the expected threading and
/// dead-reference issues.
///
/// Data members, in logical rather than alphabetical order:
///
/// mellon_: 'Home-office' password given--enable some fancy features.
/// (LOTR: 'pedo mellon a minno'.)
///
/// ash_nazg_: 'One password to rule them all' given--enable everything
/// 'mellon' does (by forcing the latter member's value), along with
/// other features--some of which may be experimental or perilous.
/// (LOTR: 'ash nazg durbatulûk'.)
///
/// pyx_: Enable experimental behaviors that vary from time to time.
///
/// custom_io_0_: Special input and output facility for one customer.
///
/// regression_testing_: Enable special behaviors needed for regression
/// testing. For instance, allow test cases to run even in states that
/// haven't approved a product, because it is important to test new
/// products before approval.
///
/// data_directory_: Path to data files, initialized to ".", not an
/// empty string. Reason: objects of the boost filesystem library's
/// path class are created from these strings, which, if the strings
/// were empty, would trigger exceptions when passed to that library's
/// directory_iterator ctor.
///
/// Directory members, whose names end in 'directory_', are stored as
/// filesystem path objects because that is their nature. They are
/// accessed as such in order to make misuse more difficult. But they
/// are set from std::string objects, because that is a natural way to
/// store them e.g. in xml files, and because that enables 'set_'
/// functions to validate their arguments.

class LMI_SO global_settings final
{
  public:
    static global_settings& instance();

    void set_mellon                   (bool);
    void set_ash_nazg                 (bool);
    void set_pyx                      (std::string const&);
    void set_custom_io_0              (bool);
    void set_regression_testing       (bool);
    void set_data_directory           (std::string const&);
    void set_prospicience_date        (calendar_date const&);

    bool                 mellon                   () const;
    bool                 ash_nazg                 () const;
    std::string const&   pyx                      () const;
    bool                 custom_io_0              () const;
    bool                 regression_testing       () const;
    fs::path const&      data_directory           () const;
    calendar_date const& prospicience_date        () const;

  private:
    global_settings() = default;
    ~global_settings() = default;
    global_settings(global_settings const&) = delete;
    global_settings& operator=(global_settings const&) = delete;

    bool mellon_                     {false};
    bool ash_nazg_                   {false};
    std::string pyx_                 {};
    bool custom_io_0_                {false};
    bool regression_testing_         {false};
    fs::path data_directory_         {fs::system_complete(".")};
    calendar_date prospicience_date_ {last_yyyy_date()};
};

#endif // global_settings_hpp
