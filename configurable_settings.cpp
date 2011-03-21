// Configurable settings.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "contains.hpp"
#include "data_directory.hpp"     // AddDataDir()
#include "handle_exceptions.hpp"
#include "miscellany.hpp"         // lmi_array_size()
#include "path_utility.hpp"       // validate_directory(), validate_filepath()
#include "platform_dependent.hpp" // access()

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm> // std::copy()
#include <iterator>
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
std::string const& configuration_filename()
{
    static std::string s("configurable_settings.xml");
    return s;
}

/// Store the complete configuration-file path at startup, in case
/// it's non-complete--as is typical msw usage.
///
/// Look for the configuration file first where FHS would have it.
/// To support non-FHS platforms, if it's not readable there, then
/// look in the data directory.
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

fs::path const& configuration_filepath()
{
    static fs::path complete_path;
    if(!complete_path.empty())
        {
        return complete_path;
        }

    std::string filename = LMI_INSTALL_PREFIX "/etc/opt/lmi/"
                            + configuration_filename();
    if(0 != access(filename.c_str(), R_OK))
        {
        filename = AddDataDir(configuration_filename());
        if(0 != access(filename.c_str(), R_OK))
            {
            fatal_error()
                << "No readable file '"
                << configuration_filename()
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
    complete_path = fs::system_complete(filename);
    return complete_path;
}

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

configurable_settings::configurable_settings()
    :calculation_summary_columns_      (default_calculation_summary_columns())
    ,cgi_bin_log_filename_             ("cgi_bin.log"                 )
    ,custom_input_filename_            ("custom.ini"                  )
    ,custom_output_filename_           ("custom.out"                  )
    ,default_input_filename_           ("etc/opt/default.ill"         )
    ,libraries_to_preload_             (""                            )
    ,offer_hobsons_choice_             (false                         )
    ,print_directory_                  ("/var/opt/lmi/spool"          )
    ,skin_filename_                    ("skin.xrc"                    )
    ,spreadsheet_file_extension_       (".gnumeric"                   )
    ,use_builtin_calculation_summary_  (false                         )
    ,xsl_fo_command_                   ("fo"                          )
{
    ascribe_members();
    load();

    default_input_filename_ = fs::system_complete(default_input_filename_).string();
    print_directory_        = fs::system_complete(print_directory_       ).string();

    try
        {
        validate_directory(print_directory_, "Print directory");
        }
    catch(...)
        {
        report_exception();
        print_directory_ = fs::system_complete(".").string();
        warning()
            << "If possible, current directory '"
            << print_directory_
            << "' will be used for print files instead."
            << LMI_FLUSH
            ;
        validate_directory(print_directory_, "Fallback print directory");
        }
}

configurable_settings::~configurable_settings()
{}

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
        fatal_error() << "Instantiation failed." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void configurable_settings::ascribe_members()
{
    ascribe("calculation_summary_columns"      ,&configurable_settings::calculation_summary_columns_      );
    ascribe("cgi_bin_log_filename"             ,&configurable_settings::cgi_bin_log_filename_             );
    ascribe("custom_input_filename"            ,&configurable_settings::custom_input_filename_            );
    ascribe("custom_output_filename"           ,&configurable_settings::custom_output_filename_           );
    ascribe("default_input_filename"           ,&configurable_settings::default_input_filename_           );
    ascribe("libraries_to_preload"             ,&configurable_settings::libraries_to_preload_             );
    ascribe("offer_hobsons_choice"             ,&configurable_settings::offer_hobsons_choice_             );
    ascribe("print_directory"                  ,&configurable_settings::print_directory_                  );
    ascribe("skin_filename"                    ,&configurable_settings::skin_filename_                    );
    ascribe("spreadsheet_file_extension"       ,&configurable_settings::spreadsheet_file_extension_       );
    ascribe("use_builtin_calculation_summary"  ,&configurable_settings::use_builtin_calculation_summary_  );
    ascribe("xsl_fo_command"                   ,&configurable_settings::xsl_fo_command_                   );
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

int configurable_settings::class_version() const
{
    return 1;
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
    static std::string const a[] =
        {"xml_schema_filename"               // Withdrawn.
        ,"xsl_directory"                     // Withdrawn.
        ,"xslt_format_xml_filename"          // Withdrawn.
        ,"xslt_html_filename"                // Withdrawn.
        ,"xslt_light_tab_delimited_filename" // Withdrawn.
        ,"xslt_tab_delimited_filename"       // Withdrawn.
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
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
                : "xml_notebook_group_carveout2.xrc"   == value ? "skin_group_carveout2.xrc"
                : "xml_notebook_private_placement.xrc" == value ? "skin_reg_d.xrc"
                : "xml_notebook_single_premium.xrc"    == value ? "skin_single_premium.xrc"
                : "xml_notebook_variable_annuity.xrc"  == value ? "skin_variable_annuity.xrc"
                : throw std::runtime_error(value + ": unexpected skin filename.")
                ;
            }
        }
}

void configurable_settings::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& // detritus_map
    ,std::list<std::string>             const& // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    // Nothing to do for now.
}

// TODO ?? CALCULATION_SUMMARY Address the validation issue:

/// A whitespace-delimited list of columns to be shown on the
/// calculation summary, unless overridden by
/// use_builtin_calculation_summary(true).
///
/// Precondition: Argument is semantically valid; ultimately this will
/// be validated elsewhere.

void configurable_settings::calculation_summary_columns(std::string const& s)
{
    calculation_summary_columns_ = s;
}

/// If true, then use built-in default calculation-summary columns;
/// otherwise, use calculation_summary_columns().

void configurable_settings::use_builtin_calculation_summary(bool b)
{
    use_builtin_calculation_summary_ = b;
}

/// A whitespace-delimited list of columns to be shown on the
/// calculation summary, unless overridden by
/// use_builtin_calculation_summary(true).

std::string const& configurable_settings::calculation_summary_columns() const
{
    return calculation_summary_columns_;
}

/// Name of log file used for cgicc's debugging facility.

std::string const& configurable_settings::cgi_bin_log_filename() const
{
    return cgi_bin_log_filename_;
}

/// Static name of custom input file.

std::string const& configurable_settings::custom_input_filename() const
{
    return custom_input_filename_;
}

/// Static name of custom output file.

std::string const& configurable_settings::custom_output_filename() const
{
    return custom_output_filename_;
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

/// Directory to which xsl-fo input and output are written.

std::string const& configurable_settings::print_directory() const
{
    return print_directory_;
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

/// Command to execute xsl-fo processor. Making this an external
/// command permits using a program with a free but not GPL-compatible
/// license, such as apache fop, which cannot be linked with a GPL
/// version 2 program.

std::string const& configurable_settings::xsl_fo_command() const
{
    return xsl_fo_command_;
}

std::vector<std::string> effective_calculation_summary_columns()
{
    configurable_settings const& z = configurable_settings::instance();
    std::istringstream iss
        (z.use_builtin_calculation_summary()
        ? default_calculation_summary_columns()
        : z.calculation_summary_columns()
        );
    std::vector<std::string> columns;
    std::copy
        (std::istream_iterator<std::string>(iss)
        ,std::istream_iterator<std::string>()
        ,std::back_inserter(columns)
        );
    return columns;
}

