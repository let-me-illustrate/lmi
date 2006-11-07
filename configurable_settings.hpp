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
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: configurable_settings.hpp,v 1.13.2.4 2006-11-07 01:47:59 etarassov Exp $

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
/// skin_filename_: Name of '.xrc' interface skin.
///
/// spreadsheet_file_extension_: File extension (beginning with a dot)
/// typical for the user's preferred spreadsheet program, used to
/// determine mimetype or msw 'file association'.
///
/// xml_schema_filename_: xsd for ledger xml data and 'format.xml',
/// used in calculation summary to validate xml resources.
///
/// xsl_fo_command_: Command to execute xsl 'formatting objects'
/// processor. Making this an external command permits using a program
/// with a free but not GPL-compatible license, such as apache fop,
/// which cannot be linked with a GPL version 2 program.
///
/// xsl_fo_directory_: Directory where xsl 'formatting objects'
/// processor resides.
///
/// xsl_directory_: Directory where xsl templates, 'schema.xsd'
/// and 'format.xml' reside.
///
/// xslt_format_xml_filename_: xml file containing column titles and
/// value formats used in calculation summary.
///
/// xslt_html_filename_: xsl template that produces calculation-
/// summary html output from ledger xml data.
///
/// xslt_tab_delimited_filename_: xsl template that produces
/// calculation-summary tab-delimited output suitable for pasting into
/// a spreadsheet.

class LMI_SO configurable_settings
    :public MemberSymbolTable<configurable_settings>
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<configurable_settings>
{
  public:
    static configurable_settings& instance();

    std::string const& calculation_summary_colums () const;
    std::string const& cgi_bin_log_filename       () const;
    std::string const& custom_input_filename      () const;
    std::string const& custom_output_filename     () const;
    std::string const& default_input_filename     () const;
    std::string const& libraries_to_preload       () const;
    bool               offer_hobsons_choice       () const;
    std::string const& skin_filename              () const;
    std::string const& spreadsheet_file_extension () const;
    std::string const& xml_schema_filename        () const;
    std::string const& xsl_fo_command             () const;
    std::string const& xsl_fo_directory           () const;
    std::string const& xslt_directory             () const;
    std::string const& xslt_format_xml_filename   () const;
    std::string const& xslt_html_filename         () const;
    std::string const& xslt_tab_delimited_filename() const;

    void set_calculation_summary_colums (std::string const&);

    void load_from_file();
    void save_to_file() const;

  private:
    // bool parameters are evil, but in this case ctor is part of the private
    // interface, so its ok
    configurable_settings(bool load_values_from_file = true);

    void ascribe_members();

    std::string const& configuration_filename() const;

    std::string calculation_summary_colums_;
    std::string cgi_bin_log_filename_;
    std::string custom_input_filename_;
    std::string custom_output_filename_;
    std::string default_input_filename_;
    std::string libraries_to_preload_;
    bool        offer_hobsons_choice_;
    std::string skin_filename_;
    std::string spreadsheet_file_extension_;
    std::string xml_schema_filename_;
    std::string xsl_fo_command_;
    std::string xsl_fo_directory_;
    std::string xslt_directory_;
    std::string xslt_format_xml_filename_;
    std::string xslt_html_filename_;
    std::string xslt_tab_delimited_filename_;

#ifdef __BORLANDC__
// COMPILER !! Borland compilers defectively [11/5] require a public dtor; see:
// http://groups.google.com/groups?selm=7ei6fi%244me%241%40nnrp1.dejanews.com
// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&selm=m3k9fc25dj.fsf%40gabi-soft.fr
  public:
#endif // __BORLANDC__
    ~configurable_settings();
};

#endif // configurable_settings_hpp

