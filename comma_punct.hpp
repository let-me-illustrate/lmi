// Punctuate numbers with commas.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef comma_punct_hpp
#define comma_punct_hpp

#include "config.hpp"

#include <locale>
#include <string>

// INELEGANT !! It might be better to use std::setlocale() with a
// user-configurable locale, e.g.
//   setlocale(LC_ALL, "En_US");
// as a basis for customization, though perhaps any national locale
// would require further customization. However, this would appear not
// to work as of 2005-05-09 with gcc on the msw platform:
//   http://article.gmane.org/gmane.comp.gnu.mingw.user/16048
//     [2005-05-10T03:14:53Z from Tor Lillqvist]

class comma_punct
    :public std::numpunct<char>
{
  public:
    comma_punct()
        :std::numpunct<char>()
        {}

  protected:
    char do_thousands_sep()   const override {return ',';}
    std::string do_grouping() const override {return "\003";}
};

#endif // comma_punct_hpp
