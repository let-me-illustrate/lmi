// Determine specamt from GLP or GSP.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef gpt_specamt_hpp
#define gpt_specamt_hpp

#include "config.hpp"

#include "currency.hpp"
#include "ihs_irc7702.hpp"              // Irc7702::EIOBasis
#include "mc_enum_type_enums.hpp"       // mcenum_dbopt_7702

class BasicValues;

/// Determine specamt from GLP or GSP.
///
/// This class exists for the sole purpose of determining specamt from
/// a premium taken as the GLP or GSP. Target premium is the essential
/// complication: it depends on specamt, and different products set it
/// in such different ways that it's best established by calling into
/// an instance of class BasicValues. There's no other irreducible
/// dependency between that class and class Irc7702, so it's best to
/// keep them separate except for this special-purpose accouplement.
///
/// The physical rationale is to group several functions together so
/// that they can easily be made friends of class Irc7702. Although
/// it's unusual for all members of a class to be static functions,
/// the other options are worse. Befriending multiple free functions
/// would introduce their argument types (including class BasicValues)
/// into class Irc7702. Friendship could be avoided by adding numerous
/// public accessors to class Irc7702, but would increase complexity
/// and weaken encapsulation. And inheritance would just be wrong.

class gpt_specamt
{
    typedef Irc7702::EIOBasis EIOBasis;

  public:
    static currency CalculateGLPSpecAmt
        (BasicValues const& a_Values
        ,int                a_Duration
        ,double             a_Premium
        ,mcenum_dbopt_7702  a_DBOpt
        );
    static currency CalculateGSPSpecAmt
        (BasicValues const& a_Values
        ,int                a_Duration
        ,double             a_Premium
        );

  private:
    static currency CalculateSpecAmt
        (BasicValues const& a_Values
        ,Irc7702::EIOBasis  a_EIOBasis
        ,int                a_Duration
        ,double             a_Premium
        ,double             a_NetPmtFactorTgt
        ,double             a_NetPmtFactorExc
        );
};

#endif // gpt_specamt_hpp
