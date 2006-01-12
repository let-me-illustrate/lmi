// Configurable settings.
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

// $Id: configurable_settings.hpp,v 1.8 2006-01-12 09:53:44 chicares Exp $

#ifndef configurable_settings_hpp
#define configurable_settings_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <string>

/// Design notes for class configurable_settings.
///
/// This is a simple Meyers singleton, with the expected threading and
/// dead-reference issues.
///
/// This class reads user-configurable settings from an xml file and
/// makes them available to other modules.
///
/// It is implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.
///
/// Data members, in logical rather than alphabetical order:
///
/// cgi_bin_log_filename_: Name of log file used for cgicc's debugging
/// facility.
///
/// custom_input_filename_: Static name of custom input file.
///
/// custom_output_filename_: Static name of custom output file.
///
/// default_product_: Name of the product that is selected by default:
/// the extensionless 'basename' of the product data file.
///
/// offer_hobsons_choice_: Unsafely allow users the option to bypass
/// error conditions. Setting this to 'false' prevents the system from
/// asking whether to bypass problems; that is the default, and
/// changing it may have no effect with non-GUI interfaces. Eventually
/// this option may be removed altogether.
///
/// spreadsheet_file_extension_: File extension (beginning with a dot)
/// typical for the user's preferred spreadsheet program, used to
/// determine mimetype or msw 'file association'.
///
/// xsl_fo_command_: Command to execute xsl 'formatting objects'
/// processor. Making this an external command permits using a program
/// with a free but not GPL-compatible license, such as apache fop,
/// which cannot be linked with a GPL version 2 program.
///
/// xsl_fo_directory_: Directory where xsl 'formatting objects'
/// processor resides.

class LMI_SO configurable_settings
    :public MemberSymbolTable<configurable_settings>
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<configurable_settings>
{
  public:
    static configurable_settings& instance();

    std::string const& cgi_bin_log_filename      () const;
    std::string const& custom_input_filename     () const;
    std::string const& custom_output_filename    () const;
    std::string const& default_product           () const;
    bool               offer_hobsons_choice      () const;
    std::string const& spreadsheet_file_extension() const;
    std::string const& xsl_fo_command            () const;
    std::string const& xsl_fo_directory          () const;

  private:
    configurable_settings();

    void ascribe_members();

    std::string const& configuration_filename();

    std::string cgi_bin_log_filename_;
    std::string custom_input_filename_;
    std::string custom_output_filename_;
    std::string default_product_;
    bool offer_hobsons_choice_;
    std::string spreadsheet_file_extension_;
    std::string xsl_fo_command_;
    std::string xsl_fo_directory_;

#ifdef __BORLANDC__
// COMPILER !! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~configurable_settings();
};

#endif // configurable_settings_hpp

