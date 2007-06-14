// Ledger xsl operations.
//
// Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ledger_xsl.hpp,v 1.9 2007-06-14 16:15:08 etarassov Exp $

#ifndef ledger_xsl_hpp
#define ledger_xsl_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

class Ledger;

std::string LMI_SO write_ledger_as_pdf(Ledger const&, fs::path const&, bool experimental = false);

// EVGENIY !! EXPERIMENTAL section begins.
//
// If we keep either or both of these functions, we should consider
// making their signatures similar to that of the function above.
// Ideally, we'd make 'emit_ledger.cpp' the only place that uses them,
// and not export any of these from a shared library.

void               write_ledger_as_xml    (Ledger const&, fs::path const&);
void               write_ledger_as_xml2   (Ledger const&, fs::path const&);
void               write_ledger_as_fo_xml (Ledger const&, fs::path const&);

#endif // ledger_xsl_hpp

