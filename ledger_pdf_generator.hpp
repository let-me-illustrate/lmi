// Generate PDF files with ledger data.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#ifndef pdf_generator_hpp
#define pdf_generator_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <memory>                       // std::shared_ptr

class Ledger;

/// Abstract base class for generating PDFs with ledger data.
///
/// Although there is currently only a single concrete implementation of this
/// abstract base class and no other implementations are planned, splitting the
/// PDF generation functionality into an abstract base and the concrete derived
/// class is still needed because the former is part of liblmi while the latter
/// uses wxPdfDocument and other wx facilities and is only part of libskeleton.

class LMI_SO ledger_pdf_generator
{
  public:
    typedef std::shared_ptr<ledger_pdf_generator> (*creator_type)();

    static bool set_creator(creator_type);
    static std::shared_ptr<ledger_pdf_generator> create();

    virtual ~ledger_pdf_generator() = default;

    virtual void write(Ledger const& ledger, fs::path const& output) = 0;

  protected:
    ledger_pdf_generator() = default;

  private:
    ledger_pdf_generator(ledger_pdf_generator const&) = delete;
    ledger_pdf_generator& operator=(ledger_pdf_generator const&) = delete;
};

#endif // pdf_generator_hpp
