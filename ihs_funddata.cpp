// Fund data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// This class describes funds: their names and investment mgmt fees.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_funddata.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>

#include <fstream>

//============================================================================
FundInfo::FundInfo()
    :ScalarIMF_(0.0)
    ,ShortName_("")
    ,LongName_ ("")
{
}

//============================================================================
FundInfo::FundInfo
    (double      ScalarIMF
    ,std::string ShortName
    ,std::string LongName
    )
    :ScalarIMF_(ScalarIMF)
    ,ShortName_(ShortName)
    ,LongName_ (LongName)
{
}

//============================================================================
FundInfo::~FundInfo()
{
}

//============================================================================
namespace xml_serialize
{
    template<>
    struct type_io<FundInfo>
    {
        static void to_xml(xml::node& out, FundInfo const& in)
        {
            add_property(out, "scalar_imf", in.ScalarIMF());
            add_property(out, "short_name", in.ShortName());
            add_property(out, "long_name",  in.LongName());
        }

        static void from_xml(FundInfo& out, xml::node const& in)
        {
            double simf;
            std::string sname, lname;

            get_property(in, "scalar_imf", simf);
            get_property(in, "short_name", sname);
            get_property(in, "long_name",  lname);

            out = FundInfo(simf, sname, lname);
        }
    };
} // namespace xml_serialize

//============================================================================
FundData::FundData()
{
}

//============================================================================
FundData::FundData(std::string const& a_Filename)
{
    Read(a_Filename);
}

//============================================================================
FundData::~FundData()
{
}

//============================================================================
#ifndef LMI_NO_LEGACY_FORMATS
void FundData::ReadLegacy(std::string const& a_Filename)
{
    std::ifstream is(a_Filename.c_str());

    LMI_ASSERT(0 == FundInfo_.size());
    for(;;)
        {
        if(EOF == is.peek())
            {
            break;
            }

        FundInfo f;
        is >> f.ScalarIMF_;
        // First, a dummy call to eat the tab after the double.
        std::string sink;
        std::getline(is, sink, '\t');
        std::getline(is, f.ShortName_, '\t');
        std::getline(is, f.LongName_, '\n');
        if(!is.good())
            {
            fatal_error()
                << "Error reading fund file '"
                << a_Filename
                << "'. Try reinstalling."
                << LMI_FLUSH
                ;
            }
        FundInfo_.push_back(f);
        }
}
#endif // !LMI_NO_LEGACY_FORMATS

void FundData::Read(std::string const& a_Filename)
{
    if(access(a_Filename.c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << a_Filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

#ifndef LMI_NO_LEGACY_FORMATS
    // We temporarily support reading both XML and the old file formats.
    if(".fnd" == fs::extension(a_Filename))
        {
        ReadLegacy(a_Filename);
        return;
        }
#endif // !LMI_NO_LEGACY_FORMATS

    xml_lmi::dom_parser doc(a_Filename);
    xml_serialize::from_xml(FundInfo_, doc.root_node("fund_data"));
}

//============================================================================
void FundData::Write(std::string const& a_Filename)
{
    xml::document doc("fund_data");
    xml_serialize::to_xml(doc.get_root_node(), FundInfo_);

    if(!doc.save_to_file(a_Filename.c_str()))
        {
        fatal_error()
            << "Unable to write fund file '"
            << a_Filename
            << "'."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
void FundData::WriteFundFiles()
{
    FundData foo;
    foo.FundInfo_.push_back
        (FundInfo
            (50
            ,"Money Market"
            ,"Money Market Fund"
            )
        );
    foo.Write(AddDataDir("sample.xfnd"));
}

