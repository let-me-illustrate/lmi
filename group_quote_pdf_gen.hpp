// Generate group premium quote PDF file.
//
// Copyright (C) 2015, 2016 Gregory W. Chicares.
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

#ifndef group_quote_pdf_gen_hpp
#define group_quote_pdf_gen_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/shared_ptr.hpp>

#include <string>

class Ledger;

/// Abstract base class for generating group premium quote PDFs.
///
/// Although there is currently only a single concrete implementation of this
/// abstract base class and no other implementations are planned, splitting the
/// PDF generation functionality into an abstract base and the concrete derived
/// class is still needed because the former is part of liblmi while the latter
/// uses wxPdfDocument and other wx facilities and is only part of libskeleton.

class LMI_SO group_quote_pdf_generator
    :private lmi::uncopyable<group_quote_pdf_generator>
{
  public:
    typedef boost::shared_ptr<group_quote_pdf_generator> (*creator_type)();

    static bool set_creator(creator_type);
    static boost::shared_ptr<group_quote_pdf_generator> create();

    virtual ~group_quote_pdf_generator();

    virtual void add_ledger(Ledger const& ledger) = 0;
    virtual void save(std::string const& output_filename) = 0;

  protected:
    group_quote_pdf_generator();
};

#endif // group_quote_pdf_gen_hpp

