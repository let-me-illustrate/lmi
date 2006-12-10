// Miscellaneous functions.
//
// Copyright (C) 2001, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: miscellany.cpp,v 1.4 2006-12-10 16:19:22 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "miscellany.hpp"

#include <ctime>
#include <fstream>
#include <istream>
#include <iterator>
#include <stdexcept>

namespace
{
/// This function is a derived work adapted from usenet article
/// <1eo2sct.ggkc9z84ko0eN%cbarron3@ix.netcom.com>. GWC rewrote it in
/// 2005. Any defect should not reflect on Carl Barron's reputation.

bool streams_are_identical(std::istream& is0, std::istream& is1)
{
    std::istreambuf_iterator<char> i(is0);
    std::istreambuf_iterator<char> j(is1);
    std::istreambuf_iterator<char> end;

    for(; end != i && end != j; ++i, ++j)
        {
        if(*i != *j)
            {
            return false;
            }
        }
    return end == i && end == j;
}
} // Unnamed namespace.

// TODO ?? Add unit tests.

bool files_are_identical(std::string const& file0, std::string const& file1)
{
    std::ifstream is0(file0.c_str());
    std::ifstream is1(file1.c_str());
    return streams_are_identical(is0, is1);
}

// http://groups-beta.google.com/group/borland.public.cpp.borlandcpp/msg/638d1f25e66472d9
// 18 Jul 2001 18:25:15 -0400
std::string iso_8601_datestamp_verbose()
{
   std::size_t const len = sizeof "CCYY-MM-DDTHH:MM:SSZ";
   std::time_t t = std::time(0);
   std::tm* gmt = std::gmtime(&t);
   char s[len];
   std::size_t rc = std::strftime(s, len, "%Y-%m-%dT%H:%M:%SZ", gmt);
   if(0 == rc)
       {
       throw std::logic_error("std::strftime() failed.");
       }
   return s;
}

std::string iso_8601_datestamp_terse()
{
   std::size_t const len = sizeof "CCYYMMDDTHHMMSSZ";
   std::time_t t = std::time(0);
   std::tm* gmt = std::gmtime(&t);
   char s[len];
   std::size_t rc = std::strftime(s, len, "%Y%m%dT%H%M%SZ", gmt);
   if(0 == rc)
       {
       throw std::logic_error("std::strftime() failed.");
       }
   return s;
}

