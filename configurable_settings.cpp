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

// $Id: configurable_settings.cpp,v 1.14.2.18 2006-11-08 23:10:56 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"

#include "alert.hpp"
#include "data_directory.hpp"     // AddDataDir()
#include "handle_exceptions.hpp"
#include "platform_dependent.hpp" // access()
#include "xml_lmi.hpp"

#include <boost/filesystem/operations.hpp>
#include <libxml++/libxml++.h>

#include <fstream>
#include <stdexcept>
#include <vector>

// TODO ?? Need unit tests.

namespace
{
    std::string const& xml_root_name()
    {
        static std::string s("configurable_settings");
        return s;
    }

    // the defualt value for configurable_settings::calculation_summary_colums
    std::string calculation_summary_colums_default_value
        = "Outlay AcctVal_Guaranteed CSVNet_Guaranteed EOYDeathBft_Guaranteed "
          "AcctVal_Current CSVNet_Current EOYDeathBft_Current";
} // Unnamed namespace.

configurable_settings::configurable_settings(bool load_values_from_file)
    :calculation_summary_colums_       (calculation_summary_colums_default_value)
    ,cgi_bin_log_filename_             ("cgi_bin.log"        )
    ,custom_input_filename_            ("custom.ini"         )
    ,custom_output_filename_           ("custom.out"         )
    ,default_input_filename_           ("etc/opt/default.ini")
    ,libraries_to_preload_             (""                   )
    ,offer_hobsons_choice_             (false                )
    ,skin_filename_                    ("xml_notebook.xrc"   )
    ,spreadsheet_file_extension_       (".gnumeric"          )
    ,xml_schema_filename_              ("schema.xsd"         )
    ,xsl_fo_command_                   ("fo"                 )
    ,xsl_fo_directory_                 ("/usr/bin/fo"        )
    ,xslt_directory_                   (""                   )
    ,xslt_format_xml_filename_         ("format.xml"         )
    ,xslt_html_filename_               ("html.xsl"           )
    ,xslt_light_tab_delimited_filename_("cs_tsv.xsl"         )
    ,xslt_tab_delimited_filename_      ("tab_delimited.xsl"  )
{
    ascribe_members();
    if(load_values_from_file)
        {
        load_from_file();
        }
}

void configurable_settings::load_from_file()
{
    // Look for the configuration file first where FHS would put it.
    // To support non-FHS platforms, if it's not found there, then
    // look in the data directory.
    std::string filename = "/etc/opt/lmi/" + configuration_filename();
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

    // first of all reset all members to default values
    configurable_settings const defaults(false);
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const default_value = defaults[*i].str();
        operator[](*i) = default_value;
        }

    // save the filename to be used in the save_to_file
    xml_filename_ = filename;

    xml_lmi::dom_parser parser(xml_filename_);
    xml_lmi::Element const& root = parser.root_node(xml_root_name());
    xml_lmi::ElementContainer const elements(xml_lmi::child_elements(root));
    typedef xml_lmi::ElementContainer::const_iterator eci;
    for(eci i = elements.begin(); i != elements.end(); ++i)
        {
        operator[]((*i)->get_name()) = xml_lmi::get_content(**i);
        }

    // Store the absolute file path so that if $(pwd) is changed,
    // the xml_filename_ is not affected.
    try
        {
        xml_filename_
            = boost::filesystem::system_complete(xml_filename_).string();
        }
    catch(std::exception const& e)
        {
        warning()
            << "Configurable settings file name seems to be invalid '"
            << xml_filename_
            << "'. "
            << e.what()
            << LMI_FLUSH
            ;
        }
}

void configurable_settings::save_to_file() const
{
    if(xml_filename_.empty())
        {
        fatal_error()
            << "Configuration settings file was not loaded. "
            << "Can not write back settings."
            << LMI_FLUSH
            ;
        }

    if(access(xml_filename_.c_str(), W_OK))
        {
        // User could choose to ignore this error and the only thing he risks
        // in that case is configurable settings not saved.
        fatal_error()
            << "Configuration settings file '"
            << xml_filename_
            << "' is not writeable."
            << LMI_FLUSH
            ;
        return; // unreachable code
        }

    // we dont want to write default values back into xml file
    // create an empty default instance of the class and compare current values
    // against the default ones, and write only modified values
    configurable_settings const defaults(false);

    xml_lmi::dom_parser parser(xml_filename_);
    xml_lmi::Document& document = parser.document();
    xml_lmi::Element& root = parser.root_node(xml_root_name());

    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const& name = *i;
        std::string const content = operator[](name).str();
        xml_lmi::Element* element = NULL;

        xml_lmi::ElementContainer elements
            (xml_lmi::child_elements(root, name)
            );

        // Now write the new (or the same old) value to the last node with
        // the name. The motivation is to allow multiple nodes with the same
        // name in the file, while only the last one determines the value.
        // If no node exists we will need to create a new fresh node.
        if(elements.empty())
            {
            // but if the value is the default one, then...
            if(content == defaults[name].str())
                {
                // ..., because the original xml file does not contain
                // the corresponding node, we should not create it.
                continue;
                }
            element = root.add_child(name);
            }
        else
            {
            // the element with the 'name' exist, we have to clear it up
            element = elements.back();
            xml_lmi::NodeContainer nodes = element->get_children();
            xml_lmi::NodeContainer::iterator it;
            for(it = nodes.begin(); it != nodes.end(); ++it)
                {
                element->remove_child(*it);
                }
            }

        if(!element)
            {
            fatal_error()
                << "Cannot create a new node '"
                << name
                << "' and write its value '"
                << content
                << "' into '"
                << xml_filename_
                << "'."
                << LMI_FLUSH
                ;
            }
        // finally put the new value into the element
        element->add_child_text(content);
        }

    std::ofstream ofs
        (xml_filename_.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    ofs << document;
}

configurable_settings::~configurable_settings()
{}

configurable_settings& configurable_settings::instance()
{
    try
        {
        // default parameter to constructor is true, which means to autoload
        // values from the xml file
        static configurable_settings z;
        return z;
        }
    catch(...)
        {
        report_exception();
        fatal_error() << "Instantiation failed." << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

void configurable_settings::ascribe_members()
{
    ascribe("calculation_summary_colums"       ,&configurable_settings::calculation_summary_colums_       );
    ascribe("cgi_bin_log_filename"             ,&configurable_settings::cgi_bin_log_filename_             );
    ascribe("custom_input_filename"            ,&configurable_settings::custom_input_filename_            );
    ascribe("custom_output_filename"           ,&configurable_settings::custom_output_filename_           );
    ascribe("default_input_filename"           ,&configurable_settings::default_input_filename_           );
    ascribe("libraries_to_preload"             ,&configurable_settings::libraries_to_preload_             );
    ascribe("offer_hobsons_choice"             ,&configurable_settings::offer_hobsons_choice_             );
    ascribe("skin_filename"                    ,&configurable_settings::skin_filename_                    );
    ascribe("spreadsheet_file_extension"       ,&configurable_settings::spreadsheet_file_extension_       );
    ascribe("xml_schema_filename"              ,&configurable_settings::xml_schema_filename_              );
    ascribe("xsl_fo_command"                   ,&configurable_settings::xsl_fo_command_                   );
    ascribe("xsl_fo_directory"                 ,&configurable_settings::xsl_fo_directory_                 );
    ascribe("xslt_directory"                   ,&configurable_settings::xslt_directory_                   );
    ascribe("xslt_format_xml_filename"         ,&configurable_settings::xslt_format_xml_filename_         );
    ascribe("xslt_html_filename"               ,&configurable_settings::xslt_html_filename_               );
    ascribe("xslt_light_tab_delimited_filename",&configurable_settings::xslt_light_tab_delimited_filename_);
    ascribe("xslt_tab_delimited_filename"      ,&configurable_settings::xslt_tab_delimited_filename_      );
}

std::string const& configurable_settings::calculation_summary_colums() const
{
    return calculation_summary_colums_;
}

std::string const& configurable_settings::configuration_filename() const
{
    static std::string s("configurable_settings.xml");
    return s;
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

std::string const& configurable_settings::skin_filename() const
{
    return skin_filename_;
}

std::string const& configurable_settings::spreadsheet_file_extension() const
{
    return spreadsheet_file_extension_;
}

std::string const& configurable_settings::xml_schema_filename() const
{
    return xml_schema_filename_;
}

std::string const& configurable_settings::xsl_fo_command() const
{
    return xsl_fo_command_;
}

std::string const& configurable_settings::xsl_fo_directory() const
{
    return xsl_fo_directory_;
}

std::string const& configurable_settings::xslt_directory() const
{
    return xslt_directory_;
}

std::string const& configurable_settings::xslt_format_xml_filename() const
{
    return xslt_format_xml_filename_;
}

std::string const& configurable_settings::xslt_html_filename() const
{
    return xslt_html_filename_;
}

std::string const& configurable_settings::xslt_light_tab_delimited_filename() const
{
    return xslt_light_tab_delimited_filename_;
}

std::string const& configurable_settings::xslt_tab_delimited_filename() const
{
    return xslt_tab_delimited_filename_;
}

void configurable_settings::set_calculation_summary_colums
    (std::string const& value
    )
{
    calculation_summary_colums_ = value;
}

