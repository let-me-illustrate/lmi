// Text to be dropped into report templates.
//
// Copyright (C) 2020 Gregory W. Chicares.
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

#include "lingo.hpp"

#include "alert.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "map_lookup.hpp"
#include "my_proem.hpp"                 // ::write_proem()
#include "sample.hpp"                   // superior::lingo
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

/// Construct from filename.

lingo::lingo(std::string const& filename)
{
    xml_lmi::dom_parser parser(filename);
    xml::element const& root = parser.root_node(xml_root_name());
    int file_version = 0;
    if(!xml_lmi::get_attr(root, "version", file_version))
        {
        alarum()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    xml_serialize::from_xml(root, map_);
}

std::string const& lingo::lookup(int index) const
{
    return map_lookup(map_, index);
}

void lingo::write_lingo_files()
{
    // superior::lingo enumerators are used for clarity in specifying
    // this map. They decay to integers in the resulting file, which
    // can therefore be read without the enumerators being visible.
    static std::unordered_map<superior::lingo,std::string> enumerative_map
        {{superior::policy_form      , "UL32768-NY"}
        ,{superior::policy_form_KS_KY, "UL32768-X"}
        };

    fs::path const path(AddDataDir("sample.lingo"));
    xml_lmi::xml_document document(xml_root_name());
    write_proem(document, fs::basename(path));
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());
    xml_serialize::to_xml(root, enumerative_map);
    document.save(path.string());
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20201109T1600Z

int lingo::class_version()
{
    return 0;
}

std::string const& lingo::xml_root_name()
{
    static std::string const s("lingo");
    return s;
}

void lingo::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_leaf_name
    )
{
    ::write_proem(document, file_leaf_name);
}
