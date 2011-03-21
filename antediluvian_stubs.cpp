// Stubs for the antediluvian branch.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include "lmi.hpp"          // is_antediluvian_fork()
#include "mec_server.hpp"
#include "product_data.hpp"
#include "xml_lmi.hpp"
#include "xml_serializable.tpp"

namespace
{
    std::string const empty_string("");
} // Unnamed namespace.

void authenticate_system()
{}

glossed_string::glossed_string()
{}

glossed_string::~glossed_string()
{}

bool is_antediluvian_fork()
{
    return true;
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

std::string const& mec_state::xml_root_name() const
{
    return empty_string;
}

bool mec_state::is_detritus(std::string const&) const
{
    return false;
}

product_data::product_data(std::string const&)
{}

product_data::~product_data()
{}

int product_data::class_version() const
{
    return 0;
}

std::string const& product_data::xml_root_name() const
{
    return empty_string;
}

void product_data::read_element
    (xml::element const&
    ,std::string const&
    ,int
    )
{}

void product_data::write_element
    (xml::element&
    ,std::string const&
    ) const
{}

bool product_data::is_detritus(std::string const&) const
{
    return false;
}

std::string const& product_data::datum(std::string const&) const
{
    return empty_string;
}

