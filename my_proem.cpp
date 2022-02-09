// Proem for xml product files.
//
// Copyright (C) 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "my_proem.hpp"

#include "calendar_date.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

/// Write proemial information such as a license notice to a product file.
///
/// Copy this file to '../products/src' and edit the copy to specify
/// a proprietary proem, such as a specific copyright notice.
///
/// Argument 'file_basename', unused in this specimen implementation,
/// permits the proem to vary by file name.

void write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     // file_basename
    )
{
    std::string const y(value_cast<std::string>(today().year()));
    document.add_comment("Copyright (C) " + y + " Gregory W. Chicares.");
    document.add_comment("https://savannah.nongnu.org/projects/lmi");
}
