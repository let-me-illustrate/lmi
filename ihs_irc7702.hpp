// Internal Revenue Code section 7702 (definition of life insurance).
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_irc7702.hpp,v 1.13 2008-07-14 11:22:24 chicares Exp $

#ifndef ihs_irc7702_hpp
#define ihs_irc7702_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "round_to.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <vector>

class BasicValues;
class ULCommFns;

// Specified amount (specamt) is carefully distinguished from benefit
// amount (bftamt). The former is directly chosen by the owner, and
// typically determines the target premium and underwriting charge.
// The latter is called 'death benefit' in the statute, and is variously
// interpreted as either specamt or the actual benefit payable on death;
// we choose a neutral term, using more specific terms only where they
// are always exactly applicable. To preserve the differentiation, we
// sometimes need extra variables or arguments: for instance, the function
// CalculatePremium() needs to know both the specamt (e.g. for target
// premium) and the bftamt (for death benefit), even though those two
// arguments may be equal.

// TODO ?? Is this still true?
// No rounding is performed in this class. Round the values it calculates
// as needed, being sure to round conservatively if at all. Unrounded
// values are especially needed for the iterative specamt calculation.

class Irc7702
    :private boost::noncopyable
    ,virtual private obstruct_slicing<Irc7702>
{
    friend class FindSpecAmt;

  public:
    Irc7702
        (BasicValues         const& a_Values
        ,mcenum_defn_life_ins       a_Test7702
        ,int                        a_IssueAge
        ,int                        a_EndtAge
        ,std::vector<double> const& a_Qc
        ,std::vector<double> const& a_GLPic
        ,std::vector<double> const& a_GSPic
        ,std::vector<double> const& a_Ig
        ,std::vector<double> const& a_IntDed
        ,double                     a_PresentBftAmt
        ,double                     a_PresentSpecAmt
        ,mcenum_dbopt_7702          a_PresentDBOpt
        ,std::vector<double> const& a_AnnChgPol
        ,std::vector<double> const& a_MlyChgPol
        ,std::vector<double> const& a_MlyChgSpecAmt
        ,double                     a_SpecAmtLoadLimit
        ,std::vector<double> const& a_MlyChgADD
        ,double                     a_ADDLimit
        ,std::vector<double> const& a_LoadTgt
        ,std::vector<double> const& a_LoadExc
        ,double                     a_TargetPremium
        ,round_to<double>    const& a_round_min_premium
        ,round_to<double>    const& a_round_max_premium
        ,round_to<double>    const& a_round_min_specamt
        ,round_to<double>    const& a_round_max_specamt
// TODO ?? Should we have default arguments at all?
        ,int                        a_InforceDuration = 0
        ,double                     a_InforceCumGLP   = 0.0
        ,double                     a_InforceGSP      = 0.0
        ,double                     a_PriorBftAmt     = 0.0
        ,double                     a_PriorSpecAmt    = 0.0
        ,double                     a_LeastBftAmtEver = 0.0
        ,mcenum_dbopt_7702          a_PriorDBOpt      = mce_option1_for_7702
        // TODO ?? Perhaps other arguments are needed for inforce.
        );
    ~Irc7702();

    // TODO ?? Not sure why we need this non-const variant.
    void Initialize7702
        (double                     a_BftAmt
        ,double                     a_SpecAmt
        ,mcenum_dbopt_7702          a_DBOpt
        );
    void UpdateBOY7702();
    void ProcessGptPmt
        (int                        a_Duration
        ,double&                    a_Pmt
        ,double&                    a_CumPmt
        );
    // Returns forceout if any, else 0.0 .
    void ProcessAdjustableEvent
        (int                        a_Duration
        ,double                     a_NewBftAmt
        ,double                     a_PriorBftAmt
        ,double                     a_NewSpecAmt
        ,double                     a_PriorSpecAmt
        ,mcenum_dbopt_7702          a_NewDBOpt
        ,mcenum_dbopt_7702          a_PriorDBOpt
        );
    double Forceout();

    std::vector<double> const& Corridor() const;
    double Calculate7PP() const;
    double CalculateGLP
        (int                        a_Duration
        ,double                     a_BftAmt
        ,double                     a_SpecAmt
        ,double                     a_LeastBftAmtEver
        ,mcenum_dbopt_7702          a_DBOpt
        ) const;
    double CalculateGSP
        (int                        a_Duration
        ,double                     a_BftAmt
        ,double                     a_SpecAmt
        ,double                     a_LeastBftAmtEver
        ) const;
    double CalculateGLPSpecAmt
        (int                        a_Duration
        ,double                     a_Premium
        ,mcenum_dbopt_7702          a_DBOpt
        ) const;
    double CalculateGSPSpecAmt
        (int                        a_Duration
        ,double                     a_Premium
        ) const;
    void Initialize7702
        (double                     a_SpecAmt
        ) const;
    double GetLeastBftAmtEver() const;
    double RoundedGLP() const;
    double RoundedGSP() const;

  private:
    // Interest and DB Option basis
    enum EIOBasis
        {Opt1Int4Pct
        ,Opt2Int4Pct
        ,Opt1Int6Pct
        ,NumIOBases
        };

    void Init();
    void InitCommFns();
    void InitCorridor();
    void InitPvVectors(EIOBasis const& a_EIOBasis);

    double CalculatePremium
        (EIOBasis const&            a_EIOBasis
        ,int                        a_Duration
        ,double                     a_BftAmt
        ,double                     a_SpecAmt
        ,double                     a_LeastBftAmtEver
        ,double                     a_NetPmtFactorTgt
        ,double                     a_NetPmtFactorExc
        ) const;
    // Return value is both specamt and bftamt; we name it 'specamt'
    // because it is typically used to set an input parameter, and
    // specamt is such a parameter whereas DB is not.
    double CalculateSpecAmt
        (EIOBasis const&            a_EIOBasis
        ,int                        a_Duration
        ,double                     a_Premium
        ,double                     a_NetPmtFactorTgt
        ,double                     a_NetPmtFactorExc
        ) const;
    EIOBasis Get4PctBasis
        (mcenum_dbopt_7702          a_DBOpt
        ) const;

    BasicValues const&         Values;     // needed for target premium callback
    mcenum_defn_life_ins const Test7702;   // 7702 test: CVAT or GPT
    int const                  IssueAge;   // Issue age
    int const                  EndtAge;    // Endowment age

    std::vector<double> const& Qc;         // 7702 mortality rate
    std::vector<double> const& GLPic;      // 7702 GLP interest rate
    std::vector<double> const& GSPic;      // 7702 GSP interest rate
    std::vector<double> const& Ig;         // Death benefit discount rate
    std::vector<double> const& IntDed;     // Deduction from interest rate

    double                     PresentBftAmt;
    double                     PriorBftAmt;
    mutable double             PresentSpecAmt;
    mutable double             PriorSpecAmt;
    mutable double             LeastBftAmtEver;// Lowest bft amt since issue date // TODO ?? NOT!
    mcenum_dbopt_7702          PresentDBOpt;   // Present death benefit option
    mcenum_dbopt_7702          PriorDBOpt;     // Prior death benefit option

    std::vector<double> const& AnnChgPol;  // Annual charge per policy
    std::vector<double> const& MlyChgPol;  // Monthly charge per policy
    std::vector<double> const& MlyChgSpecAmt;  // Monthly charge per $1 spec amt
    double const               SpecAmtLoadLimit;   // Max spec amt charge base
    std::vector<double> const& MlyChgADD;  // Monthly charge for ADD
    double const               ADDLimit;   // Max spec amt for ADD charge

    std::vector<double> const& LoadTgt;    // Premium load up to target
    std::vector<double> const& LoadExc;    // Premium load on excess over target
    double const               TargetPremium;

    round_to<double>           round_min_premium;
    round_to<double>           round_max_premium;
    round_to<double>           round_min_specamt;
    round_to<double>           round_max_specamt;

    int const                  InforceDuration;
    // TODO ?? Is inforce cum GLP needed?
    double const               InforceCumGLP;
    double const               InforceGSP;

    int                        Length;

    mutable double             PresentGLP;
    mutable double             PriorGLP;
    mutable double             PresentGSP;
    mutable double             PriorGSP;
    mutable double             CumGLP;     // Cumulative GLP
    mutable double             GptLimit;   // Guideline limit: max(cum GLP, GSP)
    mutable double             CumPmts;    // Cumulative payments

    // Commutation functions
// TODO ?? Apparently the original reason for using smart pointers
// was to minimize stack usage in a 16-bit environment; clearly that
// doesn't matter anymore.
//
// TODO ?? Consider using std::vector instead of array members.
    boost::scoped_ptr<ULCommFns> CommFns       [NumIOBases];
    // After the Init- functions have executed, we can delete the
    // rather sizeable ULCommFns objects, as long as we keep the
    // endowment-year value of D for each basis.
    double                     DEndt           [NumIOBases];

    // GPT corridor factors for attained ages [IssueAge, 100]
    std::vector<double>        GptCorridor;

    // CVAT corridor factors for attained ages [IssueAge, 100]
    std::vector<double>        CvatCorridor;

    // Present values of charges and load factors
    std::vector<double>        PvChgPol        [NumIOBases];
    std::vector<double>        PvChgSpecAmt    [NumIOBases];
    std::vector<double>        PvChgADD        [NumIOBases];
    std::vector<double>        PvChgMort       [NumIOBases];
    // TODO ?? Perhaps -Sgl/Lvl and -Tgt/Exc should be dimensions.
    std::vector<double>        PvNpfSglTgt     [NumIOBases];
    std::vector<double>        PvNpfLvlTgt     [NumIOBases];
    std::vector<double>        PvNpfSglExc     [NumIOBases];
    std::vector<double>        PvNpfLvlExc     [NumIOBases];
// TODO ?? Not necessary?
    std::vector<double>        PvLoadDiffSgl   [NumIOBases];
    std::vector<double>        PvLoadDiffLvl   [NumIOBases];
};

// Implementation thoughts
//
// This class was designed to be instantiated by a C++ illustration
// program that normally processes one life at a time, then deleted
// after that life has been processed.
//
// Calling it from a non-C++ illustration program can be relatively
// straightforward. For instance, this code could be provided as a
// shared library, which could process requests to instantiate and delete
// an object for each life. It might assign each life an identifier
// that would be unique while the shared library is loaded. The identifier
// would be passed in any call through the shared library's API. Upon shutdown,
// the shared library could confirm that all instances have been deleted.
//
// Calling it from an administration system on a different platform
// raises more questions, though none is intractable--rather, there
// are a variety of design decisions that depend on specific goals.
// In large part this boils down to caching decisions.
//
// Brute force is the simplest and most robust approach: cache nothing,
// and regenerate everything on each use. Instantiating the object from
// scratch for a 20-year-old takes about 2.6 msec for this implementation
// as of 27 Mar 2000 on a 300 MHz PII with a fast hard disk and 128M RAM;
// figure that might grow to 10 msec (one one-hundredth of a second) for
// a typical life on the more powerful hardware to be used for production.
// If we have to process 100,000 lives per nightly cycle, that's one
// thousand seconds, or about 20 minutes. Does that even matter?
//
// A different approach would involve caching. For instance, present value
// factors could be calculated and stored in a table for all ages etc. and
// retrieved as needed. The resulting implementation would be limited by
// disk or RAM speed, whereas the brute-force implementation would be
// FPU bound. The table's size might be about
//   6      six present-value vectors
//   100    one hundred durations per vector
//   8      eight bytes per value
//   3      genders including unisex
//   3      smoker statuses including unismoke
//   3      underwriting types: GI, SI, FU
//   100    issue ages
// = 13MB   for *nonrated* lives
// *Rated* lives probably make this infeasible. We use about a dozen
// different table ratings, and 150 MB isn't yet quite prohibitive; but
// the potential number of different flat extra amounts and durations
// is enormous.
//
// Caching is less attractive from the standpoint of uniformity. We want
// all systems to use exactly the same path through the same code. A PC
// illustration system isn't going to store 13MB of data, never mind the
// much larger amount needed for substandard; it's going to calculate
// everything from scratch. If we precalculate vast arrays of numbers
// and store them, we create a new source of potential discrepancy.
//
// For a second-to-die product, the number of different tables needs
// to be squared. In the above example:
//   4800 bytes per table entry
//   2700 table entries for each policy form
// = 13MB per single-life policy form
// * 2700 for each combination of *nonrated* lives
// = 35GB per second-to-die policy form, for *nonrated* lives only
// But by the nature of the second-to-die market, one or both lives is
// very commonly substandard. Ignoring flat extras (which we cannot do),
// a dozen different table ratings on one or both lives gives rise to
// 144 times as many possibilities, requiring
//   35GB * 144 = 5TB
// We should not seriously contemplate storing five terabytes of numbers.
//
// The brute-force approach lets us handle changes in product
// parameters: for instance, a current monthly policy fee of $6
// reduced to $5 for all contracts as of 1 January 2005. It seems
// likely that we will make such changes.
//
// Another argument against a table-driven approach is that tables
// have to be maintained whenever any parameter changes, while code
// does not. That doesn't mean that code doesn't ever have to be
// maintained--it certainly does--but rather that code that does
// everything from first principles requires less maintenance.
//
// It is also possible to cache the above 4800 or so bytes per
// contract in each admin system record. For 100,000 policies, that's
// 480MB of storage, which is probably feasible. This approach means
// saving the actual numbers needed for each policy, rather than
// generating a table that can handle any conceivable policy. It
// involves a greater chance of error than the brute force technique.
// It would work for second-to-die policies. This approach does not
// readily accommodate parameter changes as described above: large
// amounts of data stored on the remote admin system would need to
// be changed, and great pains would need to be taken to ensure the
// correctness of the changes. This approach can achieve the ideal
// of uniformity of code across all platforms: the data stored for
// each life is exactly what would be generated by a PC illustration
// system--the difference is only that the illustration system keeps
// it in RAM for a few seconds, while the admin system stores in on
// disk for a large number of years.

#endif // ihs_irc7702_hpp

