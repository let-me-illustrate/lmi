// Rounding rules, with document storage.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_rnddata.hpp,v 1.7 2008-12-27 02:56:44 chicares Exp $

#ifndef ihs_rnddata_hpp
#define ihs_rnddata_hpp

#include "config.hpp"

#include "rounding_rules.hpp"

#include "so_attributes.hpp"

#include <string>

class LMI_SO StreamableRoundingRules
    :public rounding_rules
{
    friend class RoundingDocument;

  public:
    explicit StreamableRoundingRules(std::string const& a_Filename);

    static void WriteRndFiles();
    static void WriteProprietaryRndFiles();

    rounding_rules const& get_rounding_rules();

  private:
    StreamableRoundingRules();

    void Init(std::string const& a_Filename);
    void Read(std::string const& a_Filename);
    void ReadLegacy(std::string const& a_Filename);
    void Write(std::string const& a_Filename);
};

#endif // ihs_rnddata_hpp

