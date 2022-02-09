// Create a PDF file from a ledger.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef pdf_command_hpp
#define pdf_command_hpp

#include "config.hpp"

#include "path.hpp"
#include "so_attributes.hpp"

class Ledger;

typedef void (*pdf_command_fp_type)(Ledger const&, fs::path const&);

LMI_SO bool pdf_command_initialize(pdf_command_fp_type);

LMI_SO void pdf_command(Ledger const&, fs::path const&);

#endif // pdf_command_hpp
