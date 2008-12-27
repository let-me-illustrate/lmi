// Custom interface number zero.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: custom_io_0.hpp,v 1.12 2008-12-27 02:56:39 chicares Exp $

#ifndef custom_io_0_hpp
#define custom_io_0_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

class Input;
class Ledger;

bool LMI_SO custom_io_0_file_exists();

// These two functions take std::string arguments that either contain
// filenames or are blank. If they're blank, as they normally are,
// then the actual filenames are read from configurable settings. For
// system testing, actual filenames are supplied.
//
// Usually it would be preferable to pass actual filenames in every
// case. However, the general facility for emitting output assumes
// that input and output files share a common stem; in this custom
// situation, that need not be the case.

bool LMI_SO custom_io_0_read (Input&       , std::string const& filename);
void LMI_SO custom_io_0_write(Ledger const&, std::string const& filename);

#endif // custom_io_0_hpp

