// Configurable settings.
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

// $Id: configurable_settings.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef configurable_settings_hpp
#define configurable_settings_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <string>

// A simple Meyers singleton, with the expected dead-reference and
// threading issues.
class LMI_EXPIMP configurable_settings
{
  public:
    static configurable_settings& instance();

    std::string const& cgi_bin_log_filename();
    std::string const& custom_input_filename();
    std::string const& custom_output_filename();
    std::string const& default_product();

  private:
    configurable_settings();
    configurable_settings(configurable_settings const&);
    configurable_settings& operator=(configurable_settings const&);

    std::string cgi_bin_log_filename_;
    std::string custom_input_filename_;
    std::string custom_output_filename_;
    std::string default_product_;

#ifdef __BORLANDC__
// COMPILER!! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~configurable_settings();
};

#endif // configurable_settings_hpp

