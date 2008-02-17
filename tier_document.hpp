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

// $Id: tier_document.hpp,v 1.8 2008-02-17 15:17:15 chicares Exp $

#ifndef tier_document_hpp
#define tier_document_hpp

#include "config.hpp"

#include "product_editor.hpp"
#include "stratified_charges.hpp"

class TierDocument
    :public ProductEditorDocument
{
  public:
    TierDocument();
    virtual ~TierDocument();

    stratified_entity& get_stratified_entity(e_stratified index);

  private:
    // wxDocument overrides.
    virtual void ReadDocument (std::string const& filename);
    virtual void WriteDocument(std::string const& filename);

    void initialize_charges();

    stratified_charges charges_;

    DECLARE_DYNAMIC_CLASS(TierDocument)
};

#endif // tier_document_hpp

