// Configurable settings.
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

#ifndef configurable_settings_hpp
#define configurable_settings_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

/// Design notes for class configurable_settings.
///
/// This class reads user-configurable settings from an xml file and
/// makes them available to other modules.
///
/// It is implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.
///
/// Configurable data members:
///
/// calculation_summary_columns_: Whitespace-delimited list of columns
/// to be shown on the calculation summary, unless overridden by
/// use_builtin_calculation_summary_.
///
/// cgi_bin_log_filename_: Name of log file used for cgicc's debugging
/// facility.
///
/// custom_input_filename_: Static name of custom input file.
///
/// custom_output_filename_: Static name of custom output file.
///
/// default_input_filename_: Name of '.ill' file containing default
/// input values for new '.ill' and '.cns' files.
///
/// libraries_to_preload_: Names of any libraries to be preloaded.
/// Used to work around a defect of msw.
///
/// offer_hobsons_choice_: Unsafely allow users the option to bypass
/// error conditions. Setting this to 'false' prevents the system from
/// asking whether to bypass problems; that is the default, and
/// changing it may have no effect with non-GUI interfaces. Eventually
/// this option may be removed altogether.
///
/// print_directory_: Directory to which xsl-fo input and output are
/// written.
///
/// skin_filename_: Name of '.xrc' interface skin.
///
/// spreadsheet_file_extension_: File extension (beginning with a dot)
/// typical for the user's preferred spreadsheet program, used to
/// determine mimetype or msw 'file association'.
///
/// use_builtin_calculation_summary_: If true, then use built-in
/// defaults instead of calculation_summary_columns_.
///
/// xsl_fo_command_: Command to execute xsl 'formatting objects'
/// processor. Making this an external command permits using a program
/// with a free but not GPL-compatible license, such as apache fop,
/// which cannot be linked with a GPL version 2 program.

class LMI_SO configurable_settings
    :public MemberSymbolTable<configurable_settings>
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<configurable_settings>
{
  public:
    static configurable_settings& instance();

    void save() const;

    void calculation_summary_columns(std::string const&);
    void use_builtin_calculation_summary(bool);

    std::string const& calculation_summary_columns      () const;
    std::string const& cgi_bin_log_filename             () const;
    std::string const& custom_input_filename            () const;
    std::string const& custom_output_filename           () const;
    std::string const& default_input_filename           () const;
    std::string const& libraries_to_preload             () const;
    bool               offer_hobsons_choice             () const;
    std::string const& print_directory                  () const;
    std::string const& skin_filename                    () const;
    std::string const& spreadsheet_file_extension       () const;
    bool               use_builtin_calculation_summary  () const;
    std::string const& xsl_fo_command                   () const;

  private:
    configurable_settings();

    void ascribe_members();
    void load();

    std::string calculation_summary_columns_;
    std::string cgi_bin_log_filename_;
    std::string custom_input_filename_;
    std::string custom_output_filename_;
    std::string default_input_filename_;
    std::string libraries_to_preload_;
    bool        offer_hobsons_choice_;
    std::string print_directory_;
    std::string skin_filename_;
    std::string spreadsheet_file_extension_;
    bool        use_builtin_calculation_summary_;
    std::string xsl_fo_command_;

#ifdef __BORLANDC__
// COMPILER !! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~configurable_settings();
};

std::vector<std::string> effective_calculation_summary_columns();

#endif // configurable_settings_hpp

