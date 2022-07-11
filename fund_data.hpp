// Fund names and investment-management fees.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef fund_data_hpp
#define fund_data_hpp

#include "config.hpp"

#include "cache_file_reads.hpp"
#include "path.hpp"
#include "so_attributes.hpp"

#include <string>
#include <vector>

// Separate account funds: their names and investment mgmt fees

// Implicitly-declared special member functions do the right thing.

class LMI_SO FundInfo final
{
    friend class FundData;

  public:
    FundInfo() = default;
    FundInfo
        (double             ScalarIMF
        ,std::string const& ShortName
        ,std::string const& LongName
        ,std::string const& gloss = std::string()
        );

    FundInfo(FundInfo const&) = default;
    FundInfo(FundInfo&&) = default;
    FundInfo& operator=(FundInfo const&) = default;
    FundInfo& operator=(FundInfo&&) = default;
    ~FundInfo() = default;

    double ScalarIMF() const;
    std::string const& ShortName() const;
    std::string const& LongName() const;
    std::string const& gloss() const;

  private:
    double      ScalarIMF_ {0.0};
    std::string ShortName_ {};
    std::string LongName_  {};
    std::string gloss_     {};
};

class LMI_SO FundData final
    :public cache_file_reads<FundData>
{
  public:
    explicit FundData(fs::path const& a_Filename);
    ~FundData() = default;

    static void write_funds_files();
    static void write_proprietary_funds_files();

    FundInfo const& GetFundInfo(int j) const;
    int GetNumberOfFunds() const;

  private:
    FundData() = default; // Used by write_funds_files().
    FundData(FundData const&) = delete;
    FundData& operator=(FundData const&) = delete;

    void Read (std::string const& a_Filename);
    void Write(std::string const& a_Filename) const;

    std::vector<FundInfo> FundInfo_ {};
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

#endif // fund_data_hpp
