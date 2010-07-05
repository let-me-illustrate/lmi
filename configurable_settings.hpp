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
#include "xml_serializable.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

/// Make user-configurable settings available to other modules.
///
/// The settings reside in 'configurable_settings.xml'; that file's
/// name is not configurable.
///
/// This is a simple Meyers singleton, with the expected
/// dead-reference and threading issues.
///
/// Data members are documented in their accessors' implementations.

class LMI_SO configurable_settings
    :        private boost::noncopyable
    ,virtual private obstruct_slicing  <configurable_settings>
    ,        public  xml_serializable  <configurable_settings>
    ,        public  MemberSymbolTable <configurable_settings>
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

    // xml_serializable required implementation.
    virtual int                class_version() const;
    virtual std::string const& xml_root_name() const;

    // xml_serializable overrides.
    virtual void handle_missing_version_attribute() const;
    virtual bool is_detritus(std::string const&) const;
    virtual void redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,std::string      & value
        ) const;
    virtual void redintegrate_ex_post
        (int                                       file_version
        ,std::map<std::string, std::string> const& detritus_map
        ,std::list<std::string>             const& residuary_names
        );

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

