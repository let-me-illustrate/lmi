// Interface to libxslt.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: xslt_lmi.hpp,v 1.2 2007-01-27 00:00:52 wboutin Exp $

#ifndef xslt_lmi_hpp
#define xslt_lmi_hpp

#include "config.hpp"

#include "xml_lmi_fwd.hpp" // xml_lmi::Document

#include <boost/utility.hpp>

#include <iosfwd>
#include <map>
#include <string>

/// Forward declaration of a libxslt struct. The name is reserved,
/// but that's libxslt's fault.

struct _xsltStylesheet;

/// Interface to libxslt.

namespace xslt_lmi
{
    // TODO ?? CALCULATION_SUMMARY Reimplement this in terms of class
    // xml_lmi::xml_document.
    typedef xml_lmi::Document Document;

    class Stylesheet
        :private boost::noncopyable
    {
        typedef _xsltStylesheet* stylesheet_ptr_t;

      public:
        Stylesheet(std::string const& filename);
        Stylesheet(Document const&);
        ~Stylesheet();

        enum enum_output_type
            {e_output_xml
            ,e_output_html
            ,e_output_text
            };

        void transform
            (Document const&
            ,std::ostream&
            ,enum_output_type
            ) const;

        void transform
            (Document const&
            ,std::ostream&
            ,enum_output_type
            ,std::map<std::string,std::string> const& parameters
            ) const;

      private:
        std::string error_context_;
        stylesheet_ptr_t stylesheet_;

        void set_stylesheet(stylesheet_ptr_t stylesheet);
    };

} // namespace xslt_lmi

#endif //xslt_lmi_hpp

