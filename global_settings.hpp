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

// $Id: global_settings.hpp,v 1.3 2005-04-06 23:11:11 chicares Exp $

// TODO ?? This behaviorless aggregate is no longer appropriate.
// File 'ce_product_name.cpp' does this:
//
//    std::string data_directory = global_settings::instance().data_directory;
//    if(0 == data_directory.size())
//        {
//        data_directory = ".";
//        }
//
// because an empty string can't be used as an initializer for class
// boost::filesystem::path. It would be nice to do this too:
//
//    fs::path path(data_directory);
//    if(!fs::exists(path) || !fs::is_directory(path))
//        {
//        hobsons_choice()
//            << "Data directory '"
//            << path.string()
//            << "' not found."
//            << LMI_FLUSH
//            ;
//        }
//
// (or perhaps to split the tests and report failures separately; note
// that the test as written depends on short-circuit evaluation to
// avoid throwing an exception--see the boost documentation), but that
// other file isn't the right place. Such a condition should be tested
// at startup, and this is the right place to do that. (Testing it in
// the 'main' function isn't appropriate because there are several
// 'main' functions.)
//
// What's wanted is something like:
//
//    void set_data_directory(std::string const&);
// // implementation in '.cpp' file: data_directory_ = [argument name];
//
//    std::string const data_directory() const;
// // implementation in '.cpp' file: return data_directory_;
//
//  private:
//    std::string data_directory_;
//
// for all member variables, as a first step. Then the 'set_*'
// functions can have validation code added to them.

#ifndef global_settings_hpp
#define global_settings_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <string>

// A simple Meyers singleton, with the expected dead-reference and
// threading issues.
class LMI_EXPIMP global_settings
{
  public:
    static global_settings& instance();

    // Run regression test when regression_test_directory is specified.
    bool regression_testing();

    // 'Home-office' password given--enable some fancy features.
    bool mellon; // 'pedo mellon a minno'

    // 'One password to rule them all' given--enable even perilous features.
    bool ash_nazg; // 'ash nazg durbatulûk'

    // Special text output in a format desired by one customer.
    bool special_output;

    std::string data_directory;

    // Generate tab-delimited text output, as well as CRC, when
    // running a regression test.
    bool regression_test_full;

    // Regression test all .cns files in specified directory.
    std::string regression_test_directory;

  private:
    global_settings();
    global_settings(global_settings const&);
    global_settings& operator=(global_settings const&);

#ifdef __BORLANDC__
// COMPILER !! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~global_settings();
};

#endif // global_settings_hpp

