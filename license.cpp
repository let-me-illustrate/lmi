// The GPL and the notices it requires.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "license.hpp"

namespace
{
/// Make the GPL available as a char array representing flat text.
///
/// The provided 'GNUmakefile' runs script 'text_to_strings.sed' on the
/// GPL text to produce the file included here. Simply including the
/// GPL text file wouldn't work because it contains quote characters.

char const license_text[] =
#include "quoted_gpl"
    ;

/// Make the GPL available as a char array representing html.
///
/// The provided 'GNUmakefile' runs script 'text_to_html_strings.sed'
/// on the GPL text to produce the file included here.

char const license_html[] =
#include "quoted_gpl_html"
    ;

// The boilerplate language here could be extracted from the GPL
// text file with appropriate 'sed' commands, but that would be
// fragile because the words surrounding them might change. The
// required notices are short, and it is simple enough to write
// them out in full here.

char const notices_text[] =
    "'Let me illustrate...' creates life insurance illustrations.\n"
    "\n"
    "Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,\n"
    "2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.\n"
    "\n"
    "This program is free software; you can redistribute it and/or\n"
    "modify it under the terms of the GNU General Public License\n"
    "version 2 as published by the Free Software Foundation.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program; if not, write to the Free Software Foundation,\n"
    "Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA\n"
    ;

std::string htmlize(std::string const& s)
{
    std::string r
        ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">"
        "<html>"
        "<head>"
        "<meta http-equiv=\"Content-Type\" "
        "content=\"text/html; charset=iso-8859-1\">"
        "<title>About 'Let me illustrate...'</title>"
        "</head>"
        "<body>"
        "<pre>"
        + s +
        "</pre>"
        "</body>"
        "</html>"
        );

    static std::string const ascii("(C)");
    static std::string const html("&copy;");
    std::string::size_type position = r.find(ascii);
    while(position != std::string::npos)
        {
        r.replace(position, ascii.length(), html);
        position = r.find(ascii, 1 + position);
        }
    return r;
}
} // Unnamed namespace.

std::string const& license_as_html()
{
    static std::string s(license_html);
    return s;
}

std::string const& license_as_text()
{
    static std::string s(license_text);
    return s;
}

std::string const& license_notices_as_html()
{
    static std::string s(htmlize(license_notices_as_text()));
    return s;
}

std::string const& license_notices_as_text()
{
    static std::string s(notices_text);
    return s;
}

