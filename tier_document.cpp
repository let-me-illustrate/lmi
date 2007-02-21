// Document class for Stratified charges.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: tier_document.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "tier_document.hpp"

#include "alert.hpp"
#include "tier_view.hpp"
#include "tier_view_editor.hpp"

IMPLEMENT_DYNAMIC_CLASS(TierDocument, ProductEditorDocument)

TierDocument::TierDocument()
    :ProductEditorDocument()
    ,charges_()
{}

TierDocument::~TierDocument()
{
}

void TierDocument::ReadDocument(wxString const& filename)
{
    charges_.read(filename);
}

void TierDocument::WriteDocument(wxString const& filename)
{
    charges_.write(filename);
}

stratified_entity* TierDocument::get_stratified_entity(e_stratified index)
{
    return &charges_.raw_entity(index);
}

