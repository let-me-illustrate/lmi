// Document class for Stratified charges.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "tier_document.hpp"

#include "ieee754.hpp" // infinity<>()

// EVGENIY !! Doesn't it seem strange that this wx header appears
// to be needed here? I don't see it included in similar files.
// I tried omitting it, but wasn't able to figure out what the
// diagnostics really meant.

#include <wx/defs.h>

#include <vector>

IMPLEMENT_DYNAMIC_CLASS(TierDocument, ProductEditorDocument)

TierDocument::TierDocument()
    :ProductEditorDocument()
    ,charges_()
{
    initialize_charges();
}

TierDocument::~TierDocument()
{
}

void TierDocument::initialize_charges()
{
    static stratified_entity const dummy_entity
        (std::vector<double>(1, infinity<double>()) // limits
        ,std::vector<double>(1,                0.0) // values
        );
    std::vector<std::string> const& v = charges_.member_names();
    typedef std::vector<std::string>::const_iterator svci;
    for(svci i = v.begin(); i != v.end(); ++i)
        {
        charges_.datum(*i) = dummy_entity;
        }
}

void TierDocument::ReadDocument(std::string const& filename)
{
    load(charges_, filename);
}

void TierDocument::WriteDocument(std::string const& filename)
{
    save(charges_, filename);
}

stratified_entity& TierDocument::get_stratified_entity(e_stratified index)
{
    return charges_.raw_entity(index);
}

