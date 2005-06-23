// Global settings.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: global_settings.hpp,v 1.8 2005-06-23 14:47:55 chicares Exp $

#ifndef global_settings_hpp
#define global_settings_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <string>

/// Design notes for class global_settings.
///
/// A simple Meyers singleton, with the expected dead-reference and
/// threading issues.
///
/// Data members, in logical rather than alphabetical order:
///
/// mellon: 'Home-office' password given--enable some fancy features.
/// (LOTR: 'pedo mellon a minno'.)
///
/// ash_nazg: 'One password to rule them all' given--enable everything
/// 'mellon' does (by forcing the latter member's value), along with
/// other features--some of which may be experimental or perilous.
/// (LOTR: 'ash nazg durbatulûk'.)
///
/// custom_io_0: Special input and output facility for one customer.
///
/// regression_testing: Enable special behaviors needed for regression
/// testing. For instance, allow test cases to run even in states that
/// haven't approved a product, because it is important to test new
/// products before approval.
///
/// data_directory: Path to data files.
///
/// regression_test_directory: Path for regression-testing input and
/// output.

class LMI_EXPIMP global_settings
    :private boost::noncopyable
    ,virtual private obstruct_slicing<global_settings>
{
  public:
    static global_settings& instance();

    void set_mellon                   (bool);
    void set_ash_nazg                 (bool);
    void set_custom_io_0              (bool);
    bool set_regression_testing       (bool);
    void set_data_directory           (std::string const&);
    void set_regression_test_directory(std::string const&);

    bool               mellon                   () const;
    bool               ash_nazg                 () const;
    bool               custom_io_0              () const;
    bool               regression_testing       () const;
    std::string const& data_directory           () const;
    std::string const& regression_test_directory() const;

  private:
    global_settings();

    bool mellon_;
    bool ash_nazg_;
    bool custom_io_0_;
    bool regression_testing_;
    std::string data_directory_;
    std::string regression_test_directory_;

#ifdef __BORLANDC__
// COMPILER !! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~global_settings();
};

#endif // global_settings_hpp

