// Stubs for the antediluvian branch.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "authenticity.hpp"
#include "mec_server.hpp"
#include "xml_serializable.tpp"

void authenticate_system()
{}

std::string const& timestamp_of_production_release()
{
    static std::string const s("");
    return s;
}

mec_server::mec_server(mcenum_emission)
{}

mec_server::~mec_server()
{}

bool mec_server::operator()(fs::path const&)
{
    return false;
}

template class xml_serializable<mec_state>;

mec_state::mec_state()
{}

mec_state::mec_state(mec_state const&)
    :obstruct_slicing  <mec_state>()
    ,xml_serializable  <mec_state>()
    ,MemberSymbolTable <mec_state>()
{}

mec_state::~mec_state()
{}

int mec_state::class_version() const
{
    return 0;
}

std::string mec_state::xml_root_name() const
{
    static std::string const s("");
    return s;
}

bool mec_state::is_detritus(std::string const&) const
{
    return false;
}

std::string mec_state::redintegrate_ex_ante
    (int
    ,std::string const&
    ,std::string const&
    ) const
{
    static std::string const s("");
    return s;
}

void mec_state::redintegrate_ex_post
    (int
    ,std::map<std::string, std::string>
    ,std::list<std::string>
    )
{}

void mec_state::redintegrate_ad_terminum()
{}

