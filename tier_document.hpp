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

// $Id: tier_document.hpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

#ifndef tier_document_hpp
#define tier_document_hpp

#include "config.hpp"

#include "stratified_charges.hpp"
#include "product_editor.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h>

class TierDocument
    :public ProductEditorDocument
{
  public:
    TierDocument();
    virtual ~TierDocument();

    stratified_entity* get_stratified_entity(e_stratified index);

  private:
    // wxDocument overrides.
    virtual void ReadDocument(wxString const& filename);
    virtual void WriteDocument(wxString const& filename);

    stratified_charges charges_;

    DECLARE_DYNAMIC_CLASS(TierDocument)
};


#endif // tier_document_hpp

