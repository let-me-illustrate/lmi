// Death benefits.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: deathbenefits.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "deathbenefits.hpp"

#include "basic_values.hpp"
#include "inputs.hpp"

#include <algorithm>    // std::min()

//============================================================================
TDeathBfts::TDeathBfts()
{
    Alloc(0);
}

//============================================================================
TDeathBfts::TDeathBfts(BasicValues const& Values)
{
    Alloc(Values.GetLength());
    Init(Values);
}

//============================================================================
TDeathBfts::TDeathBfts(TDeathBfts const& obj)
{
    Alloc(obj.GetLength());
    Copy(obj);
}

//============================================================================
TDeathBfts& TDeathBfts::operator=(TDeathBfts const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Alloc(obj.Length);
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
void TDeathBfts::Alloc(int len)
{
    Length    = len;
}

//============================================================================
void TDeathBfts::Copy(TDeathBfts const& obj)
{
    SpecAmt = obj.SpecAmt;
    DBOpt   = obj.DBOpt  ;
}

//============================================================================
void TDeathBfts::Destroy()
{
}

//============================================================================
void TDeathBfts::Init()
{
    SpecAmt.assign(Length, 0.0               );
    DBOpt  .assign(Length, e_dbopt(e_option1));
}

//============================================================================
void TDeathBfts::Init(BasicValues const& Values)
{
    SpecAmt.resize(Length);
    DBOpt  .resize(Length);
    for(int j = 0; j < Length; j++)
        {
        SpecAmt[j]  = Values.Input->SpecAmt[j].value();
        DBOpt[j]    = Values.Input->DBOpt[j];
        }
}

//============================================================================
void TDeathBfts::SetSpecAmt
    (double a_Bft
    ,int    a_BegYear
    ,int    a_EndYear
    )
{
    for(int j = a_BegYear; j < std::min(Length, a_EndYear); j++)
        {
        SpecAmt[j] = a_Bft;
        }
}

