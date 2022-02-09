// Fund names and investment-management fees.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "fund_data.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "my_proem.hpp"                 // ::write_proem()
#include "path.hpp"
#include "platform_dependent.hpp"       // access()
#include "ssize_lmi.hpp"
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

//============================================================================
FundInfo::FundInfo
    (double             ScalarIMF
    ,std::string const& ShortName
    ,std::string const& LongName
    ,std::string const& gloss
    )
    :ScalarIMF_ {ScalarIMF}
    ,ShortName_ {ShortName}
    ,LongName_  {LongName}
    ,gloss_     {gloss}
{
}

namespace xml_serialize
{
template<> struct xml_io<FundInfo>
{
    static void to_xml(xml::element& e, FundInfo const& t)
    {
        set_element(e, "scalar_imf", t.ScalarIMF());
        set_element(e, "short_name", t.ShortName());
        set_element(e, "long_name" , t.LongName ());
        set_element(e, "gloss"     , t.gloss    ());
    }

    static void from_xml(xml::element const& e, FundInfo& t)
    {
        double      scalar_imf;
        std::string short_name;
        std::string long_name;
        std::string gloss;
        get_element(e, "scalar_imf", scalar_imf);
        get_element(e, "short_name", short_name);
        get_element(e, "long_name" , long_name );
        get_element(e, "gloss"     , gloss     );
        t = FundInfo(scalar_imf, short_name, long_name, gloss);
    }
};
} // namespace xml_serialize

//============================================================================
FundData::FundData(fs::path const& a_Filename)
{
    Read(a_Filename.string());
}

int FundData::GetNumberOfFunds() const
{
    return lmi::ssize(FundInfo_);
}

namespace
{
std::string const& xml_root_name()
{
    static std::string const s("funds");
    return s;
}
} // Unnamed namespace.

//============================================================================
void FundData::Read(std::string const& a_Filename)
{
    if(access(a_Filename.c_str(), R_OK))
        {
        alarum()
            << "File '"
            << a_Filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(a_Filename);
    xml::element const& root = parser.root_node(xml_root_name());

    xml_serialize::from_xml(root, FundInfo_);
}

//============================================================================
void FundData::Write(std::string const& a_Filename) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    ::write_proem(document, fs::path{a_Filename}.stem().string());

    xml_lmi::set_attr(root, "version", "0");
    xml_serialize::to_xml(root, FundInfo_);

    document.save(a_Filename);
}

//============================================================================
void FundData::write_funds_files()
{
    FundData foo;
    foo.FundInfo_.push_back
        (FundInfo
            (50
            ,"Money Market"
            ,"Money Market Fund"
            ,"Specimen gloss."
            )
        );
    foo.Write(AddDataDir("sample.funds"));
}
