// Document class for Stratified charges.
//
// Copyright (C) 2007, 2008 Gregory W. Chicares.
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

// $Id: tier_document.cpp,v 1.7 2008-02-17 15:17:14 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "tier_document.hpp"

// EVGENIY !! Doesn't it seem strange that this wx header appears
// to be needed here? I don't see it included in similar files.
// I tried omitting it, but wasn't able to figure out what the
// diagnostics really meant.

#include <wx/defs.h>

#include <cfloat> // DBL_MAX
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
    typedef std::map<e_stratified, stratified_entity> dictionary_t;

    charges_.initialize_dictionary();

    stratified_entity dummy_entity
        (std::vector<double>(1, DBL_MAX) // limits
        ,std::vector<double>(1, 0)       // values
        );

    for
        (dictionary_t::iterator it = charges_.dictionary.begin()
        ,                       end = charges_.dictionary.end()
        ;it != end
        ;++it
        )
        {
        it->second = dummy_entity;
        }
}

void TierDocument::ReadDocument(std::string const& filename)
{
    charges_.read(filename);
}

void TierDocument::WriteDocument(std::string const& filename)
{
    charges_.write(filename);
}

stratified_entity& TierDocument::get_stratified_entity(e_stratified index)
{
    return charges_.raw_entity(index);
}

