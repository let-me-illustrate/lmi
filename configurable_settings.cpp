// Configurable settings.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "configurable_settings.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "contains.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "handle_exceptions.hpp"        // report_exception()
#include "map_lookup.hpp"
#include "mc_enum.hpp"                  // all_strings<>()
#include "mc_enum_type_enums.hpp"       // mcenum_report_column
#include "path_utility.hpp"             // validate_directory(), validate_filepath()
#include "platform_dependent.hpp"       // access()

#include <iterator>                     // istream_iterator
#include <sstream>
#include <stdexcept>

// This symbol is defined by configure when it is used. If we don't use
// configure, fall back to the default installation location which is "/"
#ifndef LMI_INSTALL_PREFIX
    #define LMI_INSTALL_PREFIX ""
#endif

template class xml_serializable<configurable_settings>;

namespace
{
std::string const& default_calculation_summary_columns()
{
    static std::string s
        ("Outlay"
        " AcctVal_Current"
        " CSVNet_Current"
        " EOYDeathBft_Current"
        );
    return s;
}
} // Unnamed namespace.

/// Store the complete configuration-file path at startup, in case
/// it's non-complete--as is typical msw usage.
///
/// Look for the configuration file first where FHS would have it.
/// To support non-FHS platforms, if it's not readable there, then
/// look in the data directory. Care should be taken to parse the
/// command line early in main(), particularly because it may use
/// '--data_path' to specify the data directory.
///
/// Throws if the file is not readable.
///
/// A warning is given at initialization if the file is readable but
/// not writable. It could conceivably be readable in both locations,
/// but writable only in the second:
///   -r--r--r-- ... /etc/opt/lmi/configurable_settings.xml
///   -rw-rw-rw- ... /opt/lmi/data/configurable_settings.xml
/// In that particular case, it might at first seem better to choose
/// the second file. However, in the most plausible case--an archival
/// copy of the system stored on a read-only medium, including coeval
/// data files--it would be better to mount that medium as the data
/// directory, e.g.:
///   -rw-rw-rw- ... /etc/opt/lmi/configurable_settings.xml
///   -r--r--r-- ... /dev/cdrom/configurable_settings.xml
/// and the file in /etc/opt/lmi/ would be chosen by default, as seems
/// most appropriate. (A knowledgeable user could of course move it
/// aside if it is desired to use the file on the read-only medium.)

std::string const& configuration_filepath()
{
    static std::string complete_path;
    if(!complete_path.empty())
        {
        return complete_path;
        }

    static std::string const basename {"configurable_settings.xml"};

    std::string filename = LMI_INSTALL_PREFIX "/etc/opt/lmi/" + basename;
    if(0 != access(filename.c_str(), R_OK))
        {
        filename = AddDataDir(basename);
        if(0 != access(filename.c_str(), R_OK))
            {
            alarum()
                << "No readable file '"
                << basename
                << "' exists."
                << LMI_FLUSH
                ;
            }
        }

    if(0 != access(filename.c_str(), W_OK))
        {
        warning()
            << "Configurable-settings file '"
            << filename
            << "' can be read but not written."
            << " No configuration changes can be saved."
            << LMI_FLUSH
            ;
        }

    validate_filepath(filename, "Configurable-settings file");
    complete_path = fs::absolute(filename).string();
    return complete_path;
}

configurable_settings::configurable_settings()
    :calculation_summary_columns_        {default_calculation_summary_columns()}
    ,census_paste_palimpsestically_      {true                                 }
    ,cgi_bin_log_filename_               {"cgi_bin.log"                        }
    ,custom_input_0_filename_            {"custom.ini"                         }
    ,custom_input_1_filename_            {"custom.inix"                        }
    ,custom_output_0_filename_           {"custom.out0"                        }
    ,custom_output_1_filename_           {"custom.out1"                        }
    ,default_input_filename_             {"/etc/opt/lmi/default.ill"           }
    ,libraries_to_preload_               {""                                   }
    ,offer_hobsons_choice_               {false                                }
    ,print_directory_                    {"/opt/lmi/print"                     }
    ,seconds_to_pause_between_printouts_ {10                                   }
    ,skin_filename_                      {"skin.xrc"                           }
    ,spreadsheet_file_extension_         {".gnumeric"                          }
    ,use_builtin_calculation_summary_    {false                                }
{
    ascribe_members();
    load();

    default_input_filename_ = fs::absolute(default_input_filename_  ).string();
    print_directory_        = remove_alien_msw_root(print_directory_).string();
    print_directory_        = fs::absolute(print_directory_         ).string();

    try
        {
        validate_directory(print_directory_, "Print directory");
        }
    catch(...)
        {
        report_exception();
        print_directory_ = fs::absolute(AddDataDir(".")).string();
        warning()
            << "If possible, data directory '"
            << print_directory_
            << "' will be used for print files instead."
            << LMI_FLUSH
            ;
        validate_directory(print_directory_, "Fallback print directory");
        save();
        }
}

configurable_settings& configurable_settings::instance()
{
    try
        {
        static configurable_settings z;
        return z;
        }
    catch(...)
        {
        report_exception();
        alarum() << "Instantiation failed." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void configurable_settings::ascribe_members()
{
    ascribe("calculation_summary_columns"        ,&configurable_settings::calculation_summary_columns_        );
    ascribe("census_paste_palimpsestically"      ,&configurable_settings::census_paste_palimpsestically_      );
    ascribe("cgi_bin_log_filename"               ,&configurable_settings::cgi_bin_log_filename_               );
    ascribe("custom_input_0_filename"            ,&configurable_settings::custom_input_0_filename_            );
    ascribe("custom_input_1_filename"            ,&configurable_settings::custom_input_1_filename_            );
    ascribe("custom_output_0_filename"           ,&configurable_settings::custom_output_0_filename_           );
    ascribe("custom_output_1_filename"           ,&configurable_settings::custom_output_1_filename_           );
    ascribe("default_input_filename"             ,&configurable_settings::default_input_filename_             );
    ascribe("libraries_to_preload"               ,&configurable_settings::libraries_to_preload_               );
    ascribe("offer_hobsons_choice"               ,&configurable_settings::offer_hobsons_choice_               );
    ascribe("print_directory"                    ,&configurable_settings::print_directory_                    );
    ascribe("seconds_to_pause_between_printouts" ,&configurable_settings::seconds_to_pause_between_printouts_ );
    ascribe("skin_filename"                      ,&configurable_settings::skin_filename_                      );
    ascribe("spreadsheet_file_extension"         ,&configurable_settings::spreadsheet_file_extension_         );
    ascribe("use_builtin_calculation_summary"    ,&configurable_settings::use_builtin_calculation_summary_    );
}

void configurable_settings::load()
{
    xml_serializable<configurable_settings>::load(configuration_filepath());
}

void configurable_settings::save() const
{
    xml_serializable<configurable_settings>::save(configuration_filepath());
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: [prior to the lmi epoch]
/// version 1: 20100612T0139Z
/// version 2: 20140915T1943Z

int configurable_settings::class_version() const
{
    return 2;
}

std::string const& configurable_settings::xml_root_name() const
{
    static std::string const s("configurable_settings");
    return s;
}

void configurable_settings::handle_missing_version_attribute() const
{
}

/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool configurable_settings::is_detritus(std::string const& s) const
{
    static std::vector<std::string> const v
        {"custom_input_filename"             // Renamed to 'custom_input_0_filename'.
        ,"custom_output_filename"            // Renamed to 'custom_output_0_filename'.
        ,"xml_schema_filename"               // Withdrawn.
        ,"xsl_directory"                     // Withdrawn.
        ,"xslt_format_xml_filename"          // Withdrawn.
        ,"xslt_html_filename"                // Withdrawn.
        ,"xslt_light_tab_delimited_filename" // Withdrawn.
        ,"xslt_tab_delimited_filename"       // Withdrawn.
        ,"xsl_fo_command"                    // Withdrawn.
        };
    return contains(v, s);
}

void configurable_settings::redintegrate_ex_ante
    (int                file_version
    ,std::string const& name
    ,std::string      & value
    ) const
{
    if(class_version() == file_version)
        {
        return;
        }

    if(0 == file_version)
        {
        // Skin names differed prior to the 20080218T1743Z change,
        // which predated the 'version' attribute.
        if("skin_filename" == name && contains(value, "xml_notebook"))
            {
            value =
                  "xml_notebook.xrc"                   == value ? "skin.xrc"
                : "xml_notebook_coli_boli.xrc"         == value ? "skin_coli_boli.xrc"
                : "xml_notebook_group_carveout.xrc"    == value ? "skin_group_carveout.xrc"
                : "xml_notebook_single_premium.xrc"    == value ? "skin_single_premium.xrc"
                : throw std::runtime_error(value + ": unexpected skin filename.")
                ;
            }
        }
}

void configurable_settings::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& detritus_map
    ,std::list<std::string>             const& residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    if(file_version < 2)
        {
        // Version 2 renamed these elements.
        LMI_ASSERT(contains(residuary_names, "custom_input_0_filename"));
        LMI_ASSERT(contains(residuary_names, "custom_output_0_filename"));
        custom_input_0_filename_  = map_lookup(detritus_map, "custom_input_filename");
        custom_output_0_filename_ = map_lookup(detritus_map, "custom_output_filename");
        }
}

/// A whitespace-delimited list of columns to be shown on the
/// calculation summary, unless overridden by
/// use_builtin_calculation_summary(true).

std::string const& configurable_settings::calculation_summary_columns() const
{
    return calculation_summary_columns_;
}

/// When pasting a census, replace old contents instead of appending.

bool configurable_settings::census_paste_palimpsestically() const
{
    return census_paste_palimpsestically_;
}

/// Name of log file used for cgicc's debugging facility.

std::string const& configurable_settings::cgi_bin_log_filename() const
{
    return cgi_bin_log_filename_;
}

/// Static name of custom "0" input file.

std::string const& configurable_settings::custom_input_0_filename() const
{
    return custom_input_0_filename_;
}

/// Static name of custom "1" input file.

std::string const& configurable_settings::custom_input_1_filename() const
{
    return custom_input_1_filename_;
}

/// Static name of custom "0" output file.

std::string const& configurable_settings::custom_output_0_filename() const
{
    return custom_output_0_filename_;
}

/// Static name of custom "1" output file.

std::string const& configurable_settings::custom_output_1_filename() const
{
    return custom_output_1_filename_;
}

/// Name of '.ill' file containing default input values for new '.ill'
/// and '.cns' files.

std::string const& configurable_settings::default_input_filename() const
{
    return default_input_filename_;
}

/// Names of any libraries to be preloaded. Used to work around a
/// defect of msw.

std::string const& configurable_settings::libraries_to_preload() const
{
    return libraries_to_preload_;
}

/// Unsafely allow users the option to bypass error conditions if
/// 'true'. Setting this to 'false' prevents the system from asking
/// whether to bypass problems; that is the default, and changing it
/// may have no effect with non-GUI interfaces. Eventually this option
/// may be removed altogether.

bool configurable_settings::offer_hobsons_choice() const
{
    return offer_hobsons_choice_;
}

/// Directory to which PDF files are written.

std::string const& configurable_settings::print_directory() const
{
    return print_directory_;
}

/// Number of seconds to pause between batched printouts.

int configurable_settings::seconds_to_pause_between_printouts() const
{
    return seconds_to_pause_between_printouts_;
}

/// Name of '.xrc' interface skin.

std::string const& configurable_settings::skin_filename() const
{
    return skin_filename_;
}

/// File extension (beginning with a dot) typical for the user's
/// preferred spreadsheet program. Used to determine mimetype or msw
/// 'file association'.

std::string const& configurable_settings::spreadsheet_file_extension() const
{
    return spreadsheet_file_extension_;
}

/// If true, then use built-in default calculation-summary columns;
/// otherwise, use calculation_summary_columns().

bool configurable_settings::use_builtin_calculation_summary() const
{
    return use_builtin_calculation_summary_;
}

namespace
{
std::vector<std::string> parse_calculation_summary_columns
    (std::string const& s
    ,bool               use_builtin_calculation_summary
    )
{
    std::istringstream iss(s);
    std::vector<std::string> const& allowable = all_strings<mcenum_report_column>();
    std::vector<std::string> columns;
    std::istream_iterator<std::string> i(iss);
    std::istream_iterator<std::string> const eos;
    for(; i != eos; ++i)
        {
        if(contains(allowable, *i))
            {
            columns.push_back(*i);
            }
        else
            {
            warning()
                << "Disregarding unrecognized calculation-summary column '"
                << *i
                << "'. Use 'Preferences' to remove it permanently."
                << std::flush
                ;
            }
        }

    if(columns.empty() && !use_builtin_calculation_summary)
        {
        warning()
            << "Calculation summary will be empty: no columns chosen."
            << LMI_FLUSH
            ;
        }

    return columns;
}
} // Unnamed namespace.

std::vector<std::string> input_calculation_summary_columns()
{
    configurable_settings const& z = configurable_settings::instance();
    return parse_calculation_summary_columns
        (z.calculation_summary_columns()
        ,z.use_builtin_calculation_summary()
        );
}

std::vector<std::string> effective_calculation_summary_columns()
{
    configurable_settings const& z = configurable_settings::instance();
    return parse_calculation_summary_columns
        (z.use_builtin_calculation_summary()
            ? default_calculation_summary_columns()
            : z.calculation_summary_columns()
        ,z.use_builtin_calculation_summary()
        );
}
