// Fund data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_funddata.hpp,v 1.2 2005-01-29 02:47:41 chicares Exp $

#ifndef funddata_hpp
#define funddata_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <string>
#include <vector>

// Separate account funds: their names and investment mgmt fees

struct LMI_EXPIMP TFundInfo
{
    TFundInfo()
        :ScalarIMF(0.0)
        ,ShortName("")
        ,LongName("")
        {}
    TFundInfo
        (double ScalarIMF
        ,std::string ShortName
        ,std::string LongName
        )
        :ScalarIMF(ScalarIMF)
        ,ShortName(ShortName)
        ,LongName(LongName)
        {}
    double ScalarIMF;
    std::string ShortName;
    std::string LongName;
};

class LMI_EXPIMP TFundData
{
public:
    TFundData(std::string const& a_Filename);
    TFundData(TFundData const&);
    TFundData& operator=(TFundData const&);
    virtual ~TFundData();

    static void WriteFundFiles();
    static void WriteProprietaryFundFiles();

    TFundInfo const& GetFundInfo(int j) const;
    int GetNumberOfFunds() const;

protected:
    void Init(std::string const& a_Filename);
    void Read(std::string const& a_Filename);

private:
    TFundData();
    void Write(std::string const& a_Filename);

    void Alloc();
    void Copy(TFundData const&);
    void Destroy();

    std::vector<TFundInfo> FundInfo;
};

inline TFundInfo const& TFundData::GetFundInfo(int j) const
{
    return FundInfo[j];
}

inline int TFundData::GetNumberOfFunds() const
{
    return FundInfo.size();
}

#endif // funddata_hpp

