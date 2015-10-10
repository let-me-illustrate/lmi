// Emit a ledger or a group of ledgers in various guises.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifndef emit_ledger_hpp
#define emit_ledger_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"       // enum mcenum_emission
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

class group_quote_pdf_generator;
class Ledger;

/// Emit a group of ledgers in various guises.
///
/// Each member function (except the lightweight ctor and dtor)
/// returns time spent, which is almost always wanted.

class LMI_SO ledger_emitter
    :        private lmi::uncopyable <ledger_emitter>
    ,virtual private obstruct_slicing<ledger_emitter>
{
  public:
    ledger_emitter(fs::path const& case_filepath, mcenum_emission emission);
    ~ledger_emitter();

    double initiate ();
    double emit_cell(fs::path const& cell_filepath, Ledger const& ledger);
    double finish   ();

  private:
    fs::path const& case_filepath_;
    mcenum_emission emission_;

    // Initialized only if required by emission_; empty otherwise.
    fs::path case_filepath_spreadsheet_;
    fs::path case_filepath_group_roster_;
    fs::path case_filepath_group_quote_;

    // Used only if emission_ includes mce_emit_group_quote; empty otherwise.
    boost::shared_ptr<group_quote_pdf_generator> group_quote_gen_;
};

double LMI_SO emit_ledger
    (fs::path const& cell_filepath
    ,Ledger const&   ledger
    ,mcenum_emission emission
    );

#endif // emit_ledger_hpp

