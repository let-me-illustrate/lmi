// Miscellaneous functions.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: miscellany_lmi.hpp,v 1.1 2005-03-11 03:19:31 chicares Exp $

#ifndef miscellany_lmi_hpp
#define miscellany_lmi_hpp

#include "config.hpp"

// TODO ?? Put non-template functions in a  '.cpp' file.

#include <cstddef>
#include <ctime>
#include <string>
#include <stdexcept>

template <typename T, std::size_t n>
inline std::size_t array_size(T(&)[n])
{return n;}

// http://groups-beta.google.com/group/borland.public.cpp.borlandcpp/msg/638d1f25e66472d9
// 18 Jul 2001 18:25:15 -0400
inline std::string iso_8601_datestamp_verbose()
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
// TODO ?? expunge?
//   return &s[0];
   return s;
}

// Omitting colons yields a valid posix path.
inline std::string iso_8601_datestamp_terse()
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

#endif // miscellany_lmi_hpp

