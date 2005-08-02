// Configurable settings.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: configurable_settings.cpp,v 1.7 2005-08-02 21:23:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"

#include "data_directory.hpp"
#include "miscellany.hpp"
#include "platform_dependent.hpp" // access()

#ifdef USING_CURRENT_XMLWRAPP
#   include <xmlwrapp/document.h>
#endif // USING_CURRENT_XMLWRAPP defined.
#include <xmlwrapp/init.h>
#include <xmlwrapp/node.h>
#include <xmlwrapp/tree_parser.h>

#include <algorithm>
#include <iterator>
#include <sstream>
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
} // Unnamed namespace.

configurable_settings::configurable_settings()
    :cgi_bin_log_filename_       ("cgi_bin.log")
    ,custom_input_filename_      ("custom.ini" )
    ,custom_output_filename_     ("custom.out" )
    ,default_product_            ("sample"     )
    ,offer_hobsons_choice_       (false)
    ,spreadsheet_file_extension_ (".gnumeric"  )
    ,xsl_fo_command_             ("fo"         )
    ,xsl_fo_directory_           ("/usr/bin/fo")
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
            throw std::runtime_error
                ("No readable file '" + configuration_filename() + "' exists."
                );
            }
        }

    xml::init init;
    xml::tree_parser parser(filename.c_str());
    if(!parser)
        {
        throw std::runtime_error
            ("Error parsing '" + configuration_filename() + "'."
            );
        }
#ifdef USING_CURRENT_XMLWRAPP
    xml::node& root = parser.get_document().get_root_node();
#else // USING_CURRENT_XMLWRAPP not defined.
    xml::node& root = parser.get_root_node();
#endif // USING_CURRENT_XMLWRAPP not defined.
    if(xml_root_name() != root.get_name())
        {
        std::ostringstream msg;
        msg
            << "File '"
            << configuration_filename()
            << "': xml node name is '"
            << root.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected. Try reinstalling."
            ;
        throw std::runtime_error(msg.str());
        }

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
    using namespace xml;
#endif // __BORLANDC__

    xml::node::const_iterator child = root.begin();
    for(; child != root.end(); ++child)
        {
        if(child->is_text())
            {
            continue;
            }
        operator[](child->get_name()) = child->get_content();
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
    ascribe("default_product"            ,&configurable_settings::default_product_           );
    ascribe("offer_hobsons_choice"       ,&configurable_settings::offer_hobsons_choice_      );
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

std::string const& configurable_settings::default_product() const
{
    return default_product_;
}

bool configurable_settings::offer_hobsons_choice() const
{
    return offer_hobsons_choice_;
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

