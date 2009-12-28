// Fund data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_funddata.cpp,v 1.11 2008-12-27 02:56:43 chicares Exp $

// This class describes funds: their names and investment mgmt fees.
// TODO ?? An extension other than .fnd would be preferable: msw uses
// .fnd for "find"

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_funddata.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "platform_dependent.hpp" // access()

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

//============================================================================
void FundData::Write(std::string const& a_Filename)
{
    std::ofstream os(a_Filename.c_str());

    std::vector<FundInfo>::const_iterator i = FundInfo_.begin();
    for(;i != FundInfo_.end(); i++)
        {
        os
            << i->ScalarIMF_
            << '\t'
            << i->ShortName_
            << '\t'
            << i->LongName_
            << '\n'
            ;
        }

    if(!os.good())
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
    foo.Write(AddDataDir("sample.fnd"));
}

