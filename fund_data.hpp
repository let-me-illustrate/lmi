// Fund names and investment-management fees.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef fund_data_hpp
#define fund_data_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

// Separate account funds: their names and investment mgmt fees

// Implicitly-declared special member functions do the right thing.

class LMI_SO FundInfo
    :virtual private obstruct_slicing<FundInfo>
{
    friend class FundData;

  public:
    FundInfo();
    FundInfo
        (double             ScalarIMF
        ,std::string const& ShortName
        ,std::string const& LongName
        ,std::string const& gloss = std::string()
        );
    ~FundInfo();

    double ScalarIMF() const;
    std::string const& ShortName() const;
    std::string const& LongName() const;
    std::string const& gloss() const;

  private:
    double ScalarIMF_;
    std::string ShortName_;
    std::string LongName_;
    std::string gloss_;
};

class LMI_SO FundData
    :private boost::noncopyable
    ,virtual private obstruct_slicing<FundData>
{
  public:
    FundData(std::string const& a_Filename);
    ~FundData();

    static void WriteFundFiles();
    static void WriteProprietaryFundFiles();

    FundInfo const& GetFundInfo(int j) const;
    int GetNumberOfFunds() const;

  private:
    FundData(); // Private, but implemented.

    void Read (std::string const& a_Filename);
    void Write(std::string const& a_Filename) const;

    std::vector<FundInfo> FundInfo_;
};

inline double FundInfo::ScalarIMF() const
{
    return ScalarIMF_;
}

inline std::string const& FundInfo::ShortName() const
{
    return ShortName_;
}

inline std::string const& FundInfo::LongName() const
{
    return LongName_;
}

inline std::string const& FundInfo::gloss() const
{
    return gloss_;
}

inline FundInfo const& FundData::GetFundInfo(int j) const
{
    return FundInfo_[j];
}

inline int FundData::GetNumberOfFunds() const
{
    return FundInfo_.size();
}

#endif // fund_data_hpp

