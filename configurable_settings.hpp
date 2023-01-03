// Configurable settings.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef configurable_settings_hpp
#define configurable_settings_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

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

class LMI_SO configurable_settings final
    :public xml_serializable  <configurable_settings>
    ,public MemberSymbolTable <configurable_settings>
{
  public:
    static configurable_settings& instance();

    void save() const;

    std::string const& calculation_summary_columns        () const;
    bool               census_paste_palimpsestically      () const;
    std::string const& cgi_bin_log_filename               () const;
    std::string const& custom_input_0_filename            () const;
    std::string const& custom_input_1_filename            () const;
    std::string const& custom_output_0_filename           () const;
    std::string const& custom_output_1_filename           () const;
    std::string const& default_input_filename             () const;
    std::string const& libraries_to_preload               () const;
    bool               offer_hobsons_choice               () const;
    std::string const& print_directory                    () const;
    int                seconds_to_pause_between_printouts () const;
    std::string const& skin_filename                      () const;
    std::string const& spreadsheet_file_extension         () const;
    bool               use_builtin_calculation_summary    () const;

  private:
    configurable_settings();
    ~configurable_settings() override = default;
    configurable_settings(configurable_settings const&) = delete;
    configurable_settings& operator=(configurable_settings const&) = delete;

    void ascribe_members();
    void load();

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    void handle_missing_version_attribute() const override;
    bool is_detritus(std::string const&) const override;
    void redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,std::string      & value
        ) const override;
    void redintegrate_ex_post
        (int                                       file_version
        ,std::map<std::string, std::string> const& detritus_map
        ,std::list<std::string>             const& residuary_names
        ) override;

    std::string calculation_summary_columns_;
    bool        census_paste_palimpsestically_;
    std::string cgi_bin_log_filename_;
    std::string custom_input_0_filename_;
    std::string custom_input_1_filename_;
    std::string custom_output_0_filename_;
    std::string custom_output_1_filename_;
    std::string default_input_filename_;
    std::string libraries_to_preload_;
    bool        offer_hobsons_choice_;
    std::string print_directory_;
    int         seconds_to_pause_between_printouts_;
    std::string skin_filename_;
    std::string spreadsheet_file_extension_;
    bool        use_builtin_calculation_summary_;
};

LMI_SO std::string const&       configuration_filepath();

std::vector<std::string>        input_calculation_summary_columns();
// This function must be visible to 'wx_test_calculation_summary.cpp'.
LMI_SO std::vector<std::string> effective_calculation_summary_columns();

#endif // configurable_settings_hpp
