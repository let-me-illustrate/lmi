// Stubs for the antediluvian branch.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "commutation_functions.hpp"
#include "fund_data.hpp"
#include "gpt_server.hpp"
#include "ihs_irc7702.hpp"
#include "lingo.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "mec_server.hpp"
#include "product_data.hpp"
#include "stratified_charges.hpp"
#include "verify_products.hpp"
#include "xml_lmi.hpp"
#include "xml_serializable.tpp"

namespace
{
    std::string const empty_string = std::string();
} // Unnamed namespace.

// Free-function stubs alphabetized in ASCII order.

void authenticate_system()
{}

double iglp()
{
    return 0.04;
}

double igsp()
{
    return 0.02 + iglp();
}

bool is_antediluvian_fork()
{
    return true;
}

void verify_products()
{}

// Member-function stubs alphabetized in ASCII order.

int FundData::GetNumberOfFunds() const
{
    return 0;
}

gpt_server::gpt_server(mcenum_emission)
{}

bool gpt_server::operator()(fs::path const&)
{
    return false;
}

template class xml_serializable<gpt_state>;

gpt_state::gpt_state() = default;

gpt_state::gpt_state(gpt_state const&)
    :xml_serializable  <gpt_state>()
    ,MemberSymbolTable <gpt_state>()
{}

int gpt_state::class_version() const
{
    return 0;
}

std::string const& gpt_state::xml_root_name() const
{
    return empty_string;
}

bool gpt_state::is_detritus(std::string const&) const
{
    return false;
}

std::string const& lingo::lookup(int) const
{
    return empty_string;
}

mec_server::mec_server(mcenum_emission)
{}

bool mec_server::operator()(fs::path const&)
{
    return false;
}

template class xml_serializable<mec_state>;

mec_state::mec_state() = default;

mec_state::mec_state(mec_state const&)
    :xml_serializable  <mec_state>()
    ,MemberSymbolTable <mec_state>()
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

product_data::product_data()
{}

product_data::product_data(std::string const&)
{}

product_data::product_data(fs::path const&)
{}

product_data::~product_data() = default;

product_data::product_data(product_data const&)
    :MemberSymbolTable <product_data> {}
{}

std::string filename_from_product_name(std::string const&)
{
    return empty_string;
}

int product_data::class_version() const
{
    return 0;
}

std::string const& product_data::xml_root_name() const
{
    return empty_string;
}

product_data::value_type product_data::fetch_element
    (xml::element const&
    ) const
{
    return value_type();
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

void product_data::write_proem
    (xml_lmi::xml_document&
    ,std::string const&
    ) const
{}

bool product_data::is_detritus(std::string const&) const
{
    return false;
}

void product_data::redintegrate_ex_ante
    (int
    ,std::string const&
    ,value_type       &
    ) const
{}

void product_data::redintegrate_ex_post
    (int
    ,std::map<std::string,value_type> const&
    ,std::list<std::string>           const&
    )
{}

std::string const& product_data::datum(std::string const&) const
{
    return empty_string;
}

double stratified_charges::maximum_tiered_premium_tax_rate(mcenum_state) const
{
    return 0.0;
}

double stratified_charges::minimum_tiered_premium_tax_rate(mcenum_state) const
{
    return 0.0;
}

bool stratified_charges::premium_tax_is_tiered(mcenum_state) const
{
    return false;
}

double stratified_charges::tiered_premium_tax(mcenum_state, double, double) const
{
    return 0.0;
}
