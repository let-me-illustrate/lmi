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

// $Id: configurable_settings.cpp,v 1.4 2005-05-01 14:21:04 chicares Exp $

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

    std::vector<char const*> const& xml_node_names()
    {
        static char const* node_names[] =
            {"default_product"
            ,"custom_input_filename"
            ,"custom_output_filename"
            ,"cgi_bin_log_filename"
            ,"spreadsheet_file_extension"
            ,"xsl_fo_command"
            ,"xsl_fo_directory"
            };
        static std::vector<char const*> v
            (node_names
            ,node_names + lmi_array_size(node_names)
            );
        return v;
    }
} // Unnamed namespace.

configurable_settings::configurable_settings()
    :cgi_bin_log_filename_       ("cgi_bin.log")
    ,custom_input_filename_      ("custom.ini" )
    ,custom_output_filename_     ("custom.out" )
    ,default_product_            ("sample"     )
    ,spreadsheet_file_extension_ (".gnumeric"  )
    ,xsl_fo_command_             ("fo"         )
    ,xsl_fo_directory_           ("/usr/bin/fo")
{
    std::string filename = AddDataDir("configurable_settings.xml");
    if(access(filename.c_str(), R_OK))
        {
        throw std::runtime_error
            ("No readable file 'configurable_settings.xml' exists."
            );
        }

    xml::init init;
    xml::tree_parser parser(filename.c_str());
    if(!parser)
        {
        throw std::runtime_error
            ("Error parsing 'configurable_settings.xml'."
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
            << "File 'configurable_settings.xml': "
            << "xml node name is '"
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
            // TODO ?? This is hokey. Rewrite it using the
            // symbolic-member-name idiom after we remove support for
            // ancient compilers that make that difficult.
        else if(std::string(xml_node_names()[0]) == child->get_name())
            {
            default_product_ = child->get_content();
            }
        else if(std::string(xml_node_names()[1]) == child->get_name())
            {
            custom_input_filename_ = child->get_content();
            }
        else if(std::string(xml_node_names()[2]) == child->get_name())
            {
            custom_output_filename_ = child->get_content();
            }
        else if(std::string(xml_node_names()[3]) == child->get_name())
            {
            cgi_bin_log_filename_ = child->get_content();
            }
        else if(std::string(xml_node_names()[4]) == child->get_name())
            {
            spreadsheet_file_extension_ = child->get_content();
            }
        else if(std::string(xml_node_names()[5]) == child->get_name())
            {
            xsl_fo_command_ = child->get_content();
            }
        else if(std::string(xml_node_names()[6]) == child->get_name())
            {
            xsl_fo_directory_ = child->get_content();
            }
        else
            {
            std::ostringstream msg;
            msg
                << "File 'configurable_settings.xml': "
                << "xml node name '"
                << child->get_name()
                << "' is unknown. Expected one of: "
                ;
            std::copy
                (xml_node_names().begin()
                ,xml_node_names().end()
                ,std::ostream_iterator<std::string>(msg, " ")
                );
            msg
                << ". Try reinstalling."
                ;
            throw std::runtime_error(msg.str());
            }
        }
}

configurable_settings::~configurable_settings()
{}

configurable_settings& configurable_settings::instance()
{
    static configurable_settings z;
    return z;
}

std::string const& configurable_settings::cgi_bin_log_filename()
{
    return cgi_bin_log_filename_;
}

std::string const& configurable_settings::custom_input_filename()
{
    return custom_input_filename_;
}

std::string const& configurable_settings::custom_output_filename()
{
    return custom_output_filename_;
}

std::string const& configurable_settings::default_product()
{
    return default_product_;
}

std::string const& configurable_settings::spreadsheet_file_extension()
{
    return spreadsheet_file_extension_;
}

std::string const& configurable_settings::xsl_fo_command()
{
    return xsl_fo_command_;
}

std::string const& configurable_settings::xsl_fo_directory()
{
    return xsl_fo_directory_;
}

