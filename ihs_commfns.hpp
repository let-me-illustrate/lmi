// Ordinary- and universal-life commutation functions.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

#ifndef ihs_commfns_hpp
#define ihs_commfns_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <vector>

/// Ordinary-life commutation functions.

class LMI_SO OLCommFns
    :private boost::noncopyable
    ,virtual private obstruct_slicing<OLCommFns>
{
  public:
    OLCommFns
        (std::vector<double> const& a_q
        ,std::vector<double> const& a_i
        );

    ~OLCommFns();

    std::vector<double> const& C() {return c;}
    std::vector<double> const& D() {return d;}
    std::vector<double> const& M() {return m;}
    std::vector<double> const& N() {return n;}

  private:
    int Length;

    std::vector<double> const& q;
    std::vector<double> const& i;

    std::vector<double>        c;
    std::vector<double>        d;
    std::vector<double>        m;
    std::vector<double>        n;
};

/// Universal-life commutation functions: Eckley, TSA XXXIX, page 18.
///
/// Constructor arguments:
///   a_qc  Eckley's Q:  mortality rates
///   a_ic  Eckley's ic: "current"    interest rates
///   a_ig  Eckley's ig: "guaranteed" interest rates
///   dbo   death benefit option
///   mode  n-iversary mode
///
/// Numeric arguments--mortality and interest rates--must be on
/// the mode for which commutation functions are wanted. If monthly
/// functions are to be obtained from annual rates, convert the
/// rates to monthly before passing them as arguments. There's more
/// than one way to perform a modal conversion, and it's not this
/// class's responsibility to choose.
///
/// The mode argument specifies the frequency of UL n-iversary
/// processing. This is most often monthly, but need not be.
///
/// All commutation functions are calculated on the mode specified
/// by mode_. Annual D and N are always also calculated because
/// premiums are often paid annually. Use monthly D and N for
/// monthly deductions in the numerator of an actuarial function,
/// but use their annual analogs in the denominator when premiums
/// are assumed to be paid annually. C and M have no such analogs
/// because no contract pays claims only at year end.
///
/// Accessors have names like aD() for the always-annual Dx, versus
/// kD() for the modal Dx. The 'k-' prefix signifies that the mode is
/// k-ly, for mode parameter k; 'm-' might seem more clearly to stand
/// for "modal", but would too easily be taken as connoting "monthly".

class LMI_SO ULCommFns
    :private boost::noncopyable
    ,virtual private obstruct_slicing<ULCommFns>
{
  public:
    ULCommFns
        (std::vector<double> const& a_qc
        ,std::vector<double> const& a_ic
        ,std::vector<double> const& a_ig
        ,mcenum_dbopt               dbo
        ,mcenum_mode                mode
        );

    ~ULCommFns();

    std::vector<double> const& aD() const {return ad;}
    std::vector<double> const& kD() const {return kd;}
    std::vector<double> const& kC() const {return kc;}
    std::vector<double> const& aN() const {return an;}
    std::vector<double> const& kM() const {return km;}

  private:
    std::vector<double>        qc;
    std::vector<double>        ic;
    std::vector<double>        ig;

    // SOMEDAY !! It would be nice to let dbo_ vary by year.
    mcenum_dbopt dbo_;
    mcenum_mode mode_;

    int Length;

    std::vector<double> ad;
    std::vector<double> kd;
    std::vector<double> kc;
    std::vector<double> an;
    std::vector<double> km;
};

#endif // ihs_commfns_hpp

