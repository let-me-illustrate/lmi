// Death benefits.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_deathbft.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_deathbft.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "inputs.hpp"

#include <algorithm>

//============================================================================
TDeathBfts::TDeathBfts(BasicValues& a_Values)
    :Values(a_Values)
    ,Length(a_Values.GetLength())
{
    Alloc();
    Init();
}

//============================================================================
TDeathBfts::TDeathBfts(TDeathBfts const& obj)
    :Values(obj.Values)
    ,Length(obj.Length)
{
    Alloc();
    Copy(obj);
}

//============================================================================
TDeathBfts& TDeathBfts::operator=(TDeathBfts const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Values = obj.Values;
        Length = obj.Length;
        Alloc();
        Copy(obj);
        }
    return *this;
}

//============================================================================
TDeathBfts::~TDeathBfts()
{
    Destroy();
}

//============================================================================
void TDeathBfts::Alloc()
{
    SpecAmt     .reserve(Length);
    DBOpt       .reserve(Length);
}

//============================================================================
void TDeathBfts::Copy(TDeathBfts const& obj)
{
    SpecAmt     = obj.SpecAmt       ;
    DBOpt       = obj.DBOpt         ;
}

//============================================================================
void TDeathBfts::Destroy()
{
}

//============================================================================
void TDeathBfts::Init()
{
    LMI_ASSERT(0 == SpecAmt.size());
    LMI_ASSERT(0 == DBOpt.size());
    for(int j = 0; j < Length; j++)
        {
        SpecAmt     .push_back(Values.Input->SpecAmt[j]);
        DBOpt       .push_back(Values.Input->DBOpt[j]);
        }
}

//============================================================================
void TDeathBfts::SetSpecAmt
    (double Bft
    ,int    SolveBegYear
    ,int    SolveEndYear
    )
{
    for(int j = SolveBegYear; j < std::min(Length, SolveEndYear); j++)
        {
        SpecAmt[j] = Bft;
        }
}

