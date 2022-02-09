// The GPL and the notices it requires.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

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
    "Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,\n" /**/ "2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015,\n" /**/ "2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.\n"
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
    std::string t(s);
    static std::string const double_newline("\n\n");
    static std::string const paragraph_separator("\n</p>\n<p>\n");
    std::string::size_type t_pos = t.find(double_newline);
    while(t_pos != std::string::npos)
        {
        t.replace(t_pos, double_newline.length(), paragraph_separator);
        t_pos = t.find(double_newline, 1 + t_pos);
        }

    std::string r
        ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">"
        "<html>"
        "<head>"
        "<meta http-equiv=\"Content-Type\" "
        "content=\"text/html; charset=iso-8859-1\">"
        "<title>About 'Let me illustrate...'</title>"
        "</head>"
        "<body>"
        "<p>"
        + t +
        "</p>"
        "</body>"
        "</html>"
        );

    // The copyright symbol (which is included in iso-8859-1) is
    // strongly preferred to its ASCII representation, but it's not
    // worth the trouble to replace the ellipsis.
    static std::string const ascii_copyright("(C)");
    static std::string const html_copyright("&copy;");
    std::string::size_type r_pos = r.find(ascii_copyright);
    while(r_pos != std::string::npos)
        {
        r.replace(r_pos, ascii_copyright.length(), html_copyright);
        r_pos = r.find(ascii_copyright, 1 + r_pos);
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
