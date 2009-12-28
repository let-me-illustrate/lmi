// Stubs for the antediluvian branch.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: antediluvian_stubs.cpp,v 1.11 2009-07-31 02:59:27 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "authenticity.hpp"
#include "mec_server.hpp"

void authenticate_system()
{
}

std::string const& timestamp_of_production_release()
{
    static std::string const s("");
    return s;
}

mec_server::mec_server(mcenum_emission)
{
}

mec_server::~mec_server()
{
}

bool mec_server::operator()(fs::path const&)
{
    return false;
}

mec_state::mec_state()
{
}

mec_state::mec_state(mec_state const&)
    :obstruct_slicing<mec_state>()
    ,streamable()
    ,MemberSymbolTable<mec_state>()
{
}

mec_state::~mec_state()
{
}

void mec_state::read(xml::element const&)
{
}

void mec_state::write(xml::element&) const
{
}

int mec_state::class_version() const
{
    return 0;
}

std::string mec_state::xml_root_name() const
{
    static std::string const s("");
    return s;
}

