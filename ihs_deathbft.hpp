// Death benefits.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2005 Gregory W. Chicares.
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

// $Id: ihs_deathbft.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef deathbft_hpp
#define deathbft_hpp

#include "config.hpp"
#include "xenumtypes.hpp"

#include <vector>

// Death benefits

class BasicValues;

class TDeathBfts
{
public:
    TDeathBfts(BasicValues& Values);
    TDeathBfts(TDeathBfts const&);
    TDeathBfts& operator=(TDeathBfts const&);
    virtual ~TDeathBfts();

    inline std::vector<double> const&  GetSpecAmt() const {return SpecAmt;}
    inline std::vector<e_dbopt> const& GetDBOpt()   const {return DBOpt;}
    void SetSpecAmt
        (double Pmt
        ,int    SolveBegYear
        ,int    SolveEndYear
        );

protected:
    BasicValues&    Values;

private:
    TDeathBfts();
    void Alloc();
    void Init();
    void Copy(TDeathBfts const&);
    void Destroy();

    int Length;
    std::vector<double>     SpecAmt;        // Specified amount
    std::vector<e_dbopt>    DBOpt;          // Death benefit option
};

#endif

