// Interface to libxslt.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: xslt_lmi.cpp,v 1.1 2006-11-12 19:56:51 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xslt_lmi.hpp"

#include "alert.hpp"
#include "xml_lmi.hpp"

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <libxml/HTMLtree.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>

#include <ostream>
#include <stdexcept>

namespace xslt_lmi
{
xslt_lmi::Stylesheet::Stylesheet(std::string const& filename)
    :stylesheet_(NULL)
{
    try
        {
        error_context_ = "Unable to parse xsl stylesheet file '" + filename + "': ";
        if(filename.empty())
            {
            throw std::runtime_error("File name is empty.");
            }

        // set global options in libxml for libxslt
        int substitute_entities_copy = xmlSubstituteEntitiesDefault(1);
        int load_ext_dtd_default_copy = xmlLoadExtDtdDefaultValue;
        xmlLoadExtDtdDefaultValue = 0;

        set_stylesheet
            (xsltParseStylesheetFile(reinterpret_cast<xmlChar const*>(filename.c_str()))
            );

        // reset global options in libxml for libxslt
        xmlSubstituteEntitiesDefault(substitute_entities_copy);
        xmlLoadExtDtdDefaultValue = load_ext_dtd_default_copy;

        if(0 == stylesheet_)
            {
            throw std::runtime_error("Parser failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

xslt_lmi::Stylesheet::Stylesheet(Document const& document)
    :stylesheet_(NULL)
{
    try
        {
        error_context_ = "Unable to parse xsl stylesheet document from xml: ";
        if(0 == document.cobj())
            {
            throw std::runtime_error("Document is empty.");
            }
        set_stylesheet(xsltParseStylesheetDoc(const_cast<xmlDoc*>(document.cobj())));

        if(0 == stylesheet_)
            {
            throw std::runtime_error("Parsing failed.");
            }
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context_ << e.what() << LMI_FLUSH;
        }
}

xslt_lmi::Stylesheet::~Stylesheet()
{
    set_stylesheet(NULL);
}

void xslt_lmi::Stylesheet::transform
    (Document const& document
    ,std::ostream& os
    ,enum_output_type output_type
    ) const
{
    transform(document, os, output_type, std::map<std::string,std::string>());
}

void xslt_lmi::Stylesheet::transform
    (Document const& document
    ,std::ostream& os
    ,enum_output_type output_type
    ,std::map<std::string,std::string> const& parameters
    ) const
{
    std::string error_context = "Unable to apply xsl stylesheet to xml document: ";
    try
        {
        if(0 == stylesheet_)
            {
            throw std::runtime_error("Can't apply a NULL stylesheet.");
            }

        // parameters buffer
        static std::vector<char const*> params;
        // container for string that need to be &quote; escaped
        static std::vector<std::string> params_container;
        // parameters pointer to pass to the libxslt engine
        char const** params_ptr = NULL;

        // if there are any parameters, pass it to the transformation
        if(!parameters.empty())
            {
            params.clear();
            params.reserve(parameters.size() * 2 + 1);

            params_container.clear();
            params_container.reserve(parameters.size());

            std::map<std::string,std::string>::const_iterator ci;
            for(ci = parameters.begin(); ci != parameters.end(); ++ci)
                {
                // add parameter name
                params.push_back(ci->first.c_str());

                // parameter value has to be &quote; escaped
                std::string str(ci->second);
                std::size_t pos = 0;
                while(std::string::npos != (pos = str.find('\'', pos)))
                    {
                    str.replace(pos, 1, "&quote;");
                    }
                // add surrounding quotes
                params_container.push_back("'" + str + "'");
                // finally add parameter value
                params.push_back(params_container.back().c_str());
                }
            // parameters array passed to libxslt has to be NULL terminated
            params.push_back(NULL);
            params_ptr = &params[0];
            }

        boost::shared_ptr<xmlDoc> xml_document_ptr
            (xsltApplyStylesheet
                (stylesheet_
                ,const_cast<xmlDoc*>(document.cobj())
                ,params_ptr
                )
            );
        if(0 == xml_document_ptr.get())
            {
            throw std::runtime_error("Failed to apply stylesheet.");
            }

        char* buffer = NULL;
        int buffer_size = 0;

        // a pointer suitable for passing it to libxslt functions
        xmlChar** buffer_ptr = reinterpret_cast<xmlChar**>(&buffer);

        if(e_output_xml == output_type)
            {
            xmlDocDumpFormatMemory
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,true               // indent xml output
                );
            }
        else if(e_output_html == output_type)
            {
            htmlDocDumpMemoryFormat
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,true               // indent html output
                );
            }
        else if(e_output_text == output_type)
            {
            htmlDocDumpMemoryFormat
                (xml_document_ptr.get() // xml document to write
                ,buffer_ptr         // text buffer pointer, we own it
                ,&buffer_size       // text buffer size
                ,false              // do _not_ indent the output
                );
            }
        else
            {
            throw std::runtime_error("Invalid output type specified.");
            }

        boost::scoped_array<char> buffer_guard(buffer);

        if(buffer_size <= 0)
            {
            throw std::runtime_error("Empty output.");
            }

        os.write(buffer, buffer_size);
        }
    catch(std::exception const& e)
        {
        fatal_error() << error_context << e.what() << LMI_FLUSH;
        }
}

void xslt_lmi::Stylesheet::set_stylesheet(stylesheet_ptr_t stylesheet)
{
    if(0 != stylesheet_)
        {
        // free the current stylesheet
        xsltFreeStylesheet(stylesheet_);
        }
    stylesheet_ = stylesheet;
}
} // namespace xslt_lmi

