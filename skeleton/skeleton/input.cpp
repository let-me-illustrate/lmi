// Sample input class for wx data-transfer demonstration.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: input.cpp,v 1.1.1.1 2004-05-15 19:58:43 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "value_cast.hpp"

TransferData const& TransferData::operator=(Input const& z)
{
    comments = z.comments;
    check0   = value_cast(z.check0, check0);
    check1   = value_cast(z.check1, check1);
    check2   = value_cast(z.check2, check2);
    option0  = z.option0.str();
    option1  = z.option1.str();
    option2  = z.option2.str();
    option3  = z.option3.str();
    return *this;
}

bool TransferData::operator==(TransferData const& z)
{
    if(z.comments != comments) return false;
    if(z.check0   != check0  ) return false;
    if(z.check1   != check1  ) return false;
    if(z.check2   != check2  ) return false;
    if(z.option0  != option0 ) return false;
    if(z.option1  != option1 ) return false;
    if(z.option2  != option2 ) return false;
    if(z.option3  != option3 ) return false;
    return true;
}

Input::Input()
    :MemberSymbolTable<Input>()
    ,comments("No digits allowed.")
    ,check0  (false)
    ,check1  (true)
    ,check2  (false)
    ,option0 (option_B)
    ,option1 (option_A)
    ,option2 (option_B)
    ,option3 (option_A)
{
// TODO ?? Can this be done more automatically?
    ascribe("comments", &Input::comments);
    ascribe("check0"  , &Input::check0  );
    ascribe("check1"  , &Input::check1  );
    ascribe("check2"  , &Input::check2  );
    ascribe("option0" , &Input::option0 );
    ascribe("option1" , &Input::option1 );
    ascribe("option2" , &Input::option2 );
    ascribe("option3" , &Input::option3 );
}

Input const& Input::operator=(TransferData const& z)
{
    comments = z.comments;
    check0 = value_cast(z.check0, check0);
    check1 = value_cast(z.check1, check1);
    check2 = value_cast(z.check2, check2);
    option0 = z.option0;
    option1 = z.option1;
    option2 = z.option2;
    option3 = z.option3;
    return *this;
}

void Input::Harmonize()
{
    // 'option1' must be lexically <= 'option0'.
    for(std::size_t j = 0; j <= option0.cardinality(); ++j)
        {
        option1.allowed_[j] = j <= option0.ordinal();
        }
    if(option0.ordinal() < option1.ordinal())
        {
        option1 = option0.value();
        }

    // 'option3' must be lexically <= 'option2'.
    for(std::size_t j = 0; j <= option2.cardinality(); ++j)
        {
        option3.allowed_[j] = j <= option2.ordinal();
        }
    if(option2.ordinal() < option3.ordinal())
        {
        option3 = option2.value();
        }

    // Enable 'check1' iff 'check0' is checked.
    check1.enabled_ = check0.datum_;

    // Enable 'check2' iff 'check0' and 'check1' are both checked.
    check2.enabled_ = check0.datum_ && check1.datum_;
}

