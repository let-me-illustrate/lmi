// Generate group premium quote PDF file.
//
// Copyright (C) 2015 Gregory W. Chicares.
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

#include "group_quote_pdf_gen.hpp"

#include "callback.hpp"

namespace
{
callback<group_quote_pdf_generator::creator_type>
    group_quote_pdf_generator_create_callback;
} // Unnnamed namespace.

typedef group_quote_pdf_generator::creator_type FunctionPointer;
template<> FunctionPointer callback<FunctionPointer>::function_pointer_ = 0;

bool group_quote_pdf_generator::set_creator(creator_type f)
{
    group_quote_pdf_generator_create_callback.initialize(f);
    return true;
}

boost::shared_ptr<group_quote_pdf_generator> group_quote_pdf_generator::create()
{
    return group_quote_pdf_generator_create_callback()();
}

group_quote_pdf_generator::group_quote_pdf_generator()
{
}

group_quote_pdf_generator::~group_quote_pdf_generator()
{
}

