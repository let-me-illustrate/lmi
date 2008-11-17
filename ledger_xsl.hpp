// Ledger xsl operations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger_xsl.hpp,v 1.12 2008-11-17 00:37:12 chicares Exp $

#ifndef ledger_xsl_hpp
#define ledger_xsl_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

class Ledger;

/// Write ledger as pdf.
///
/// Ideally, this would be called only by emit_ledger(), and would not
/// be exported from a shared library.

std::string LMI_SO write_ledger_as_pdf(Ledger const&, fs::path const&);

fs::path xsl_filepath(Ledger const&);

#endif // ledger_xsl_hpp

