// GPT server kludges.
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

// $Id: ihs_standalone.hpp,v 1.2 2005-02-14 04:37:51 chicares Exp $

#ifndef standalone_hpp
#define standalone_hpp

#include "config.hpp"

#include "calendar_date.hpp"

#ifdef STANDALONE

#include "xenumtypes.hpp"

// Special version of class BasicValues for standalone server.
// The illustration system's class is much more elaborate.

class Irc7702;

// TODO ?? Is this used?
enum EProduct
    {IsProduct0
    ,IsProduct1
    ,IsProduct2
    };
typedef x_enum<e_base<EProduct> > E_Product;

class GPT_BasicValues
{
public:
    GPT_BasicValues
        (E_Product const&   a_ProductName
        ,e_gender const&    a_Gender
        ,e_class const&     a_UnderwritingClass
        ,e_smoking const&   a_Smoker
        ,int                a_IssueAge
        ,e_uw_basis const&  a_UnderwritingBasis
        ,e_state const&     a_StateOfJurisdiction
        ,double             a_FaceAmount
        ,e_dbopt const&     a_DBOpt
        ,bool               a_ADDInForce
        );
    GPT_BasicValues(GPT_BasicValues const&);
    GPT_BasicValues& operator=(GPT_BasicValues const&);
    virtual ~GPT_BasicValues();

    void Init();
    int GetLength() const {return Length;}
    double GetTgtPrem
        (int            Year
        ,double         SpecAmt
        ,e_dbopt const& DBOpt
        ,e_mode const&  Mode
        ) const;
    double GPT_BasicValues::GetModalTgtPrem
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GPT_BasicValues::GetModalPrem
        (int           Year
        ,e_mode const& Mode
        ,double SpecAmt
        ,e_modal_prem_type const& PremType
        ) const;
    double GPT_BasicValues::GetModalPrem_MaxNonMec
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;
    double GPT_BasicValues::GetModalPrem_MlyDed
        (int           Year
        ,e_mode const& Mode
        ,double        SpecAmt
        ) const;

    Irc7702*            Irc7702_;

private:
    E_Product const&    ProductName;
    e_gender const&     Gender;
    e_class const&      UnderwritingClass;
    e_smoking const&    Smoker;
    int const           IssueAge;
    e_uw_basis const&   UnderwritingBasis;
    e_state const&      StateOfJurisdiction;
    double              FaceAmount;
    e_dbopt const&      DBOpt;
    bool                ADDInForce;

    e_modal_prem_type const& TargetPremiumType;

    int                 Length;
    vector<double>      a7pp7702A;
    vector<double>      mAddRates;
};

#else
#   include "basic_values.hpp"
#endif // STANDALONE

#endif // standalone_hpp

