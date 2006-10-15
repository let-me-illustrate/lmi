// Libxml++ inclusion header.
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

// $Id: xml_lmi.hpp,v 1.1.2.1 2006-10-15 17:15:44 chicares Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include <libxml++/libxml++.h>

#include <string>
#include <istream>

// injecting into xmlpp namespace
namespace xmlpp
{
    // Helper class containing some frequently used operations
    class LmiHelper
    {
    public:
        // retrieves the full textual content of the xml node
        static std::string get_content(const Element & node);

        // gets first non-textual node element of the parent and its const version
        static Element * get_first_element(Element & node);
        static const Element * get_first_element(const Element & node);
    };

    // prefer os << document; over document.write_to_stream(os);
    std::ostream & operator << ( std::ostream & os, Document & document );
}

#endif //xml_lmi_hpp

