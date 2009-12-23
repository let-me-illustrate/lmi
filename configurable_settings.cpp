// Configurable settings.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: configurable_settings.cpp,v 1.51 2009-04-19 20:33:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"

#include "alert.hpp"
#include "data_directory.hpp"     // AddDataDir()
#include "handle_exceptions.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "platform_dependent.hpp" // access()
#include "xml_lmi.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <xmlwrapp/nodes_view.h>

#include <algorithm> // std::find()
#include <iterator>
#include <sstream>

// This symbol is defined by configure when it is used. If we don't use
// configure, fall back to the default installation location which is "/"
#ifndef LMI_INSTALL_PREFIX
    #define LMI_INSTALL_PREFIX ""
#endif

// TODO ?? Need unit tests.

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
/// To support non-FHS platforms, if it's not found there, then
/// look in the data directory.
///
/// TODO ?? CALCULATION_SUMMARY Should write access be checked
/// here? What if the first file found is read-only, but the
/// second is read-write?

fs::path const& configuration_filepath()
{
    static fs::path complete_path;
    if(!complete_path.empty())
        {
        return complete_path;
        }

    std::string filename = LMI_INSTALL_PREFIX "/etc/opt/lmi/"
                            + configuration_filename();
    if(access(filename.c_str(), R_OK))
        {
        filename = AddDataDir(configuration_filename());
        if(access(filename.c_str(), R_OK))
            {
            fatal_error()
                << "No readable file '"
                << configuration_filename()
                << "' exists."
                << LMI_FLUSH
                ;
            }
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

/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool is_detritus(std::string const& s)
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
    return v.end() != std::find(v.begin(), v.end(), s);
}

std::string const& xml_root_name()
{
    static std::string s("configurable_settings");
    return s;
}
} // Unnamed namespace.

configurable_settings::configurable_settings()
    :calculation_summary_columns_     (default_calculation_summary_columns())
    ,cgi_bin_log_filename_            ("cgi_bin.log"                        )
    ,custom_input_filename_           ("custom.ini"                         )
    ,custom_output_filename_          ("custom.out"                         )
    ,default_input_filename_          ("/etc/opt/lmi/default.ill"           )
    ,libraries_to_preload_            (""                                   )
    ,offer_hobsons_choice_            (false                                )
    ,print_directory_                 ("/var/opt/lmi/spool"                 )
    ,skin_filename_                   ("skin.xrc"                           )
    ,spreadsheet_file_extension_      (".gnumeric"                          )
    ,use_builtin_calculation_summary_ (false                                )
    ,xsl_fo_command_                  ("fo"                                 )
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

// TODO ?? CALCULATION_SUMMARY Class template any_member should expose
// a has_element() function.

void configurable_settings::load()
{
    std::ostringstream oss;
    xml_lmi::dom_parser parser(configuration_filepath().string());
    xml::element const& root = parser.root_node(xml_root_name());
    xml::const_nodes_view const elements(root.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi i = elements.begin(); i != elements.end(); ++i)
        {
        std::string name = i->get_name();
        if
            (   member_names().end()
            !=  std::find(member_names().begin(), member_names().end(), name)
            )
            {
            operator[](i->get_name()) = xml_lmi::get_content(*i);
            }
        else if(is_detritus(name))
            {
            // Hold certain obsolete entities that must be translated.
            // For now, there are none.
            }
        else
            {
            oss << "  '" << name << "'\n";
            }
        }
    if(!oss.str().empty())
        {
        warning()
            << "Configurable-settings file '"
            << configuration_filepath()
            << "':\n"
            << oss.str()
            << "not recognized."
            << LMI_FLUSH
            ;
        }
}

void configurable_settings::save() const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        xml_lmi::add_node(root, *i, operator[](*i).str());
        }

    fs::ofstream ofs(configuration_filepath(), ios_out_trunc_binary());
    ofs << document;
    if(!ofs)
        {
        fatal_error()
            << "Configurable-settings file '"
            << configuration_filepath()
            << "' is not writeable."
            << LMI_FLUSH
            ;
        }
}

// TODO ?? CALCULATION_SUMMARY Address the validation issue:
/// Precondition: Argument is semantically valid; ultimately this will
/// be validated elsewhere.

void configurable_settings::calculation_summary_columns(std::string const& s)
{
    calculation_summary_columns_ = s;
}

void configurable_settings::use_builtin_calculation_summary(bool b)
{
    use_builtin_calculation_summary_ = b;
}

std::string const& configurable_settings::calculation_summary_columns() const
{
    return calculation_summary_columns_;
}

std::string const& configurable_settings::cgi_bin_log_filename() const
{
    return cgi_bin_log_filename_;
}

std::string const& configurable_settings::custom_input_filename() const
{
    return custom_input_filename_;
}

std::string const& configurable_settings::custom_output_filename() const
{
    return custom_output_filename_;
}

std::string const& configurable_settings::default_input_filename() const
{
    return default_input_filename_;
}

std::string const& configurable_settings::libraries_to_preload() const
{
    return libraries_to_preload_;
}

bool configurable_settings::offer_hobsons_choice() const
{
    return offer_hobsons_choice_;
}

std::string const& configurable_settings::print_directory() const
{
    return print_directory_;
}

std::string const& configurable_settings::skin_filename() const
{
    return skin_filename_;
}

std::string const& configurable_settings::spreadsheet_file_extension() const
{
    return spreadsheet_file_extension_;
}

bool configurable_settings::use_builtin_calculation_summary() const
{
    return use_builtin_calculation_summary_;
}

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

