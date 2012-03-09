// Proem for xml product files.
//
// Copyright (C) 2012 Gregory W. Chicares.
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

#include "my_proem.hpp"

#include "calendar_date.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

/// Write proemial information such as a license notice to a product file.
///
/// Copy this file to '../products/src' and edit the copy to specify
/// a proprietary proem, such as a specific copyright notice.
///
/// Argument 'file_leaf_name', unused in this specimen implementation,
/// permits the proem to vary by file name.

void write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     // file_leaf_name
    )
{
    std::string const y(value_cast<std::string>(today().year()));
    document.add_comment("Copyright (C) " + y + " Gregory W. Chicares.");
    document.add_comment("http://savannah.nongnu.org/projects/lmi");
    // The apparently-needless '""' prevents 'test_coding_rules.cpp'
    // from complaining that this file has more than one RCS Id.
    document.add_comment("$""Id: $");
}

