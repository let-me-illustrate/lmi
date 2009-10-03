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

// $Id: ihs_commfns.hpp,v 1.16 2009-10-03 17:32:15 chicares Exp $

#ifndef ihs_commfns_hpp
#define ihs_commfns_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <vector>

// Commutation functions
// C, D, M, and N from q and i
// Can be reused for quick solving or initial solve guess
// Can be reused for reserves
// Can be reused for nonforfeiture

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

class LMI_SO ULCommFns
    :private boost::noncopyable
    ,virtual private obstruct_slicing<ULCommFns>
{
  public:
    ULCommFns
        (std::vector<double> const& a_q
        ,std::vector<double> const& a_ic
        ,std::vector<double> const& a_ig
        ,mcenum_dbopt               a_db_option
        ,mcenum_mode                a_commfn_mode
        ,mcenum_mode                a_process_mode
        );

    // ctor arguments:
    // a_q              mortality rates
    // a_ic             interest rates
    // a_ig             guaranteed interest rate
    // a_db_option      death benefit option
    // a_commfn_mode    mode of desired commutation functions
    // a_process_mode   contract processing mode
    //
    // Numeric arguments--mortality and interest rates--must be on
    // the mode for which commutation functions are wanted. If monthly
    // functions are to be obtained from annual rates, convert the
    // rates to monthly before passing them as arguments. There's more
    // than one way to perform a modal conversion, and it's not this
    // class's responsibility to choose.
    //
    // TODO ?? The primary (?) interest rate is a_i.
    //
    // UL commutation functions require two interest rates:
    // a_ic corresponds to Eckley's ic;
    // a_ig corresponds to Eckley's ig.

    // UL commutation functions vary by death benefit option.
    // TODO ?? It would be nice to let db option vary by year.
    //
    // UL commutation functions D and N are calculated on the mode
    // specified by a_commfn_mode. For instance, apply monthly
    // D and N to monthly premiums and monthly policy fees.
    // TODO ?? Functions C and M do not depend on a_commfn_mode because...why?
// input mode for C -> freq of clm pmt?
    //
    // The contract processing mode, a_process_mode, specifies
    // how often UL n-iversary processing is done. This is most
    // commonly performed monthly, but need not be.

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

//  std::vector<double>        q;
//  std::vector<double>        i;

    mcenum_dbopt DBOption;
    // TODO ?? Should we keep this?
    mcenum_mode CommfnMode;
    // Processing mode--usually monthly--governs how frequently
    // COIs and expense charges are deducted.
    mcenum_mode ProcessMode;

    int Length;

    std::vector<double> ad;
    std::vector<double> kd;
    std::vector<double> kc;
    std::vector<double> an;
    std::vector<double> km;
};

#endif // ihs_commfns_hpp

