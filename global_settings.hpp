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

// $Id: global_settings.hpp,v 1.2 2005-03-11 13:40:41 chicares Exp $

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

