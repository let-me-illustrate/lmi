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

// $Id: configurable_settings.cpp,v 1.14.2.5 2006-10-17 12:13:09 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"

#include "alert.hpp"
#include "data_directory.hpp"     // AddDataDir()
#include "platform_dependent.hpp" // access()
#include "xml_lmi.hpp"

#include <exception>

// TODO ?? Need unit tests.

namespace
{
    std::string const& xml_root_name()
    {
        static std::string s("configurable_settings");
        return s;
    }
} // Unnamed namespace.

configurable_settings::configurable_settings()
    :cgi_bin_log_filename_       ("cgi_bin.log"        )
    ,custom_input_filename_      ("custom.ini"         )
    ,custom_output_filename_     ("custom.out"         )
    ,default_input_filename_     ("etc/opt/default.ini")
    ,libraries_to_preload_       (""                   )
    ,offer_hobsons_choice_       (false                )
    ,skin_filename_              ("xml_notebook.xrc"   )
    ,spreadsheet_file_extension_ (".gnumeric"          )
    ,xsl_fo_command_             ("fo"                 )
    ,xsl_fo_directory_           ("/usr/bin/fo"        )
{
    ascribe_members();

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

    xmlpp::DomParser parser;
    try {
        parser.parse_file(filename);
    }
    catch(std::exception const& ex)
    {
        fatal_error() << "Exception while reading '"
                      << configuration_filename()
                      << "'."
                      << "(" << ex.what() << ")"
                      << LMI_FLUSH;
    }

    if(!parser)
        {
        fatal_error()
            << "Error parsing '"
            << configuration_filename()
            << "'."
            << LMI_FLUSH
            ;
        }
    xmlpp::Element& root = *parser.get_document()->get_root_node();
    if(xml_root_name() != root.get_name())
        {
        fatal_error()
            << "File '"
            << configuration_filename()
            << "': xml node name is '"
            << root.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xmlpp::Node::NodeList const children = root.get_children();
    for(xmlpp::Node::NodeList::const_iterator iter = children.begin();
                                               iter != children.end();
                                               ++iter)
    {
        xmlpp::Element const* child =
            dynamic_cast<xmlpp::Element const*>(*iter);
        if(!child)
            {
            continue;
            }
        (*this)[child->get_name()] = xmlpp::LmiHelper::get_content(*child);
    }
}

configurable_settings::~configurable_settings()
{}

configurable_settings& configurable_settings::instance()
{
    static configurable_settings z;
    return z;
}

void configurable_settings::ascribe_members()
{
    ascribe("cgi_bin_log_filename"       ,&configurable_settings::cgi_bin_log_filename_      );
    ascribe("custom_input_filename"      ,&configurable_settings::custom_input_filename_     );
    ascribe("custom_output_filename"     ,&configurable_settings::custom_output_filename_    );
    ascribe("default_input_filename"     ,&configurable_settings::default_input_filename_    );
    ascribe("libraries_to_preload"       ,&configurable_settings::libraries_to_preload_      );
    ascribe("offer_hobsons_choice"       ,&configurable_settings::offer_hobsons_choice_      );
    ascribe("skin_filename"              ,&configurable_settings::skin_filename_             );
    ascribe("spreadsheet_file_extension" ,&configurable_settings::spreadsheet_file_extension_);
    ascribe("xsl_fo_command"             ,&configurable_settings::xsl_fo_command_            );
    ascribe("xsl_fo_directory"           ,&configurable_settings::xsl_fo_directory_          );
}

std::string const& configurable_settings::configuration_filename()
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

std::string const& configurable_settings::xsl_fo_command() const
{
    return xsl_fo_command_;
}

std::string const& configurable_settings::xsl_fo_directory() const
{
    return xsl_fo_directory_;
}

