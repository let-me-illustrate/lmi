// 7702 (and 7702A) interest rates.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "i7702.hpp"

#include "assert_lmi.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "miscellany.hpp"               // each_equal()
#include "ssize_lmi.hpp"

#include <ios>                          // fixed, ios_base::precision() 7702 !! pyx
#include <iostream>                     // 7702 !! pyx
#include <sstream>                      // 7702 !! pyx

/// Here's how lmi determines §7702 and §7702A interest rates.
///
/// All these variables are vectors that may vary by year, except
/// that statutory rates are always scalar.
///
/// Several rates must be considered:
///
/// A: statutory rates (concrete values A0 and A1 described below)
/// B: contractual guarantees
/// C: initial short-term guarantees
/// D: asset-based charges
/// E: NAAR discount
///
/// and several account-paths for each of {B,C,D}:
///
/// gen: general account  (unloaned)
/// sep: separate account (unloaned)
/// flr: fixed loan rate
/// vlr: variable loan rate
///
/// Let "max" mean the year-by-year maximum of vectors: e.g.,
///   max({1 2 3}, {0 2 4}) is {1 2 4}
///
/// Then the general formula is the maximum of
///   max(A, B, C) - D
/// along each account-path, i.e.
///   max
///     (max(A, Bgen, Cgen) - Dgen
///     ,max(A, Bsep, Csep) - Dsep
///     ,max(A, Bflr, Cflr) - Dflr
///     ,max(A, Bvlr, Cvlr) - Dvlr
///     )
/// where A is chosen from {A0, A1} as appropriate,
/// and the particular formulas are
///   ic_usual  max(A0, B, C)
///   ic_glp    max(A0, B   ) - D
///   ic_gsp    max(A1, B, C) - D
/// where C and D are taken as zero if omitted.
///
/// All ig are zero iff E uniformly equals zero; otherwise each is
/// the greater of its ic counterpart and E:
///   ig_usual  max(ic_usual, E)
///   ig_glp    max(ic_glp,   E)
///   ig_gsp    max(ic_gsp,   E)
///
/// Exhaustive list of use cases:
///   {GLP; GSP; CVAT NSP and corridor; §7702A NSP; 7PP; DCV}
/// All but {GLP; GSP} use "usual" rates.
///
/// Notes on input rates
///
/// A: statutory rates (always scalar)
///   A0: all but GSP (e.g., 4% as of 1984)
///   A1: GSP only (A0 + 2% in current statute)
///
/// B: contractual guarantees
///   for loans: charged - spread
///   usually no guarantee for separate account, making Bsep zero
///
/// C: initial short-term guarantees--ignored for GLP only
///   "extending no more than one year" (DEFRA Blue Book, page 649)
///   a vector for convenience, uniformly zero after the issue year
///   expressed as the total floor rate, not as an increment
///   usually altogether avoided by careful product design
///
/// D: asset-based charges
///   lowest value each year, if dependent on assets, premiums, etc.
///   separate-account-only charges affect only Dsep
///
/// E: NAAR discount (given here as i, the annual rate of interest)
///   often specified in contract as Bgen upper 12 / 12
///     if monthly contract factor rounded down, Bgen governs instead
///     (slightly better 7702 outcome in that case)
///   but some policies do not discount NAAR
///     in which case E uniformly equals zero
///   an assertion checks that either E=0 or E materially equals Bgen
///
/// Monthly and annual rates
///
/// For consistency, the formulas above are expressed in terms of
/// annual rates only. Most of the calculations use only annual rates,
/// transforming them (i --> i upper 12 / 12) to monthly as a final
/// step for actual use with monthly UL commutation functions.
///
/// However, to preserve accuracy, the ig* calculations are performed
/// on a monthly basis. The rate E above is normally expressed on a
/// monthly basis, often with rounding; it makes no sense to convert
/// it to annual in a formula like this:
///   ig_usual  max(ic_usual, E)
/// and then convert the result back to monthly, especially since the
/// corresponding ic* rate must be converted to monthly anyway.
///
/// Therefore, lmi actually retrieves a monthly E (call it Em) from
/// the product database, and returns only monthly ig* and ic* rates.
///
/// Discussion
///
/// 7702 interest rates should be rounded up, if at all; lmi doesn't
/// round them at all.
///
/// ig may actually exceed ic; for example:
///   ic = 4% guaranteed rate
///   ig = 4.00000001% implied by contractual NAAR discount
/// The contractual NAAR discount affects only ig. Effect:
///   lower  ig --> higher guideline
///   higher ig --> lower guideline
/// so this could be considered "conservative", but really it just
/// follows from §7702(b)(2)(A)'s "greater of" prescription.
///
/// Asset-based charges (D above) affect the interest rate only for
/// calculation of guideline premiums. They're expense charges, which
/// must be ignored for 7PP and for the §7702 as well as §7702A NSPs
/// because those quantities are net premiums. DCV calculations deduct
/// actual charges during monthly processing, and credit interest at
/// the ic_usual rate, which ignores D because those charges must not
/// be double-counted; thus, DCV correctly reflects any dependence of
/// such charges on asset or premium tiers, which D cannot do.
///
/// For contracts that require an irrevocable choice, before delivery,
/// between a fixed and a variable loan rate, the loan path not chosen
/// need not be considered (but lmi doesn't yet implement VLR).
///
/// Idea not implemented: optionally set all ig* equal to E. The SOA
/// textbook (page 52) supports treating it as "a contractual element
/// that is not an interest rate guaranteed on issue of the contract"
/// for §7702 purposes. Yet it's simple to follow lmi's more careful
/// interpretation, which most often produces materially the same
/// result. If a contract specifies E as the monthly equivalent of
/// any rate other than Bgen, that's presumably just a mistake.

i7702::i7702
    (int                        length
    ,double                     A0
    ,double                     A1
    ,std::vector<double> const& Bgen
    ,std::vector<double> const& Bsep
    ,std::vector<double> const& Bflr
    ,std::vector<double> const& Bvlr
    ,std::vector<double> const& Cgen
    ,std::vector<double> const& Csep
    ,std::vector<double> const& Cflr
    ,std::vector<double> const& Cvlr
    ,std::vector<double> const& Dgen
    ,std::vector<double> const& Dsep
    ,std::vector<double> const& Dflr
    ,std::vector<double> const& Dvlr
    ,std::vector<double> const& Em
    ,std::vector<double> const& use_gen
    ,std::vector<double> const& use_sep
    ,std::vector<double> const& use_flr
    ,std::vector<double> const& use_vlr
    )
    :length_   {length }
    ,trace_    {false  }
    ,A0_       {A0     }
    ,A1_       {A1     }
    ,Bgen_     {Bgen   }
    ,Bsep_     {Bsep   }
    ,Bflr_     {Bflr   }
    ,Bvlr_     {Bvlr   }
    ,Cgen_     {Cgen   }
    ,Csep_     {Csep   }
    ,Cflr_     {Cflr   }
    ,Cvlr_     {Cvlr   }
    ,Dgen_     {Dgen   }
    ,Dsep_     {Dsep   }
    ,Dflr_     {Dflr   }
    ,Dvlr_     {Dvlr   }
    ,Em_       {Em     }
    ,use_gen_  {use_gen}
    ,use_sep_  {use_sep}
    ,use_flr_  {use_flr}
    ,use_vlr_  {use_vlr}
    ,ic_usual_ (length_)
    ,ic_glp_   (length_)
    ,ic_gsp_   (length_)
    ,ig_usual_ (length_)
    ,ig_glp_   (length_)
    ,ig_gsp_   (length_)
{
    initialize();
}

void i7702::assert_preconditions()
{
    LMI_ASSERT(0 < length_);

    LMI_ASSERT(length_ == lmi::ssize(Bgen_   ));
    LMI_ASSERT(length_ == lmi::ssize(Bsep_   ));
    LMI_ASSERT(length_ == lmi::ssize(Bflr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Bvlr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Cgen_   ));
    LMI_ASSERT(length_ == lmi::ssize(Csep_   ));
    LMI_ASSERT(length_ == lmi::ssize(Cflr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Cvlr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Dgen_   ));
    LMI_ASSERT(length_ == lmi::ssize(Dsep_   ));
    LMI_ASSERT(length_ == lmi::ssize(Dflr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Dvlr_   ));
    LMI_ASSERT(length_ == lmi::ssize(Em_     ));
    LMI_ASSERT(length_ == lmi::ssize(use_gen_));
    LMI_ASSERT(length_ == lmi::ssize(use_sep_));
    LMI_ASSERT(length_ == lmi::ssize(use_flr_));
    LMI_ASSERT(length_ == lmi::ssize(use_vlr_));

    // These must all be boolean, and not all false in any year.
    LMI_ASSERT(AllOf(EqualTo(true, use_gen_) || EqualTo(false, use_gen_)));
    LMI_ASSERT(AllOf(EqualTo(true, use_sep_) || EqualTo(false, use_sep_)));
    LMI_ASSERT(AllOf(EqualTo(true, use_flr_) || EqualTo(false, use_flr_)));
    LMI_ASSERT(AllOf(EqualTo(true, use_vlr_) || EqualTo(false, use_vlr_)));
    LMI_ASSERT(AllOf(use_gen_ || use_sep_ || use_flr_ || use_vlr_));

    LMI_ASSERT(each_equal(++Cgen_.begin(), Cgen_.end(), 0.0));
    LMI_ASSERT(each_equal(++Csep_.begin(), Csep_.end(), 0.0));
    LMI_ASSERT(each_equal(++Cflr_.begin(), Cflr_.end(), 0.0));
    LMI_ASSERT(each_equal(++Cvlr_.begin(), Cvlr_.end(), 0.0));
}

void i7702::initialize()
{
    assert_preconditions();
    // max(A0, B, C)
    ic_usual_ <<= Max
        (Max
            ((Max(A0_, Max(Bgen_, Cgen_))        ) * use_gen_
            ,(Max(A0_, Max(Bsep_, Csep_))        ) * use_sep_
            )
        ,Max
            ((Max(A0_, Max(Bflr_, Cflr_))        ) * use_flr_
            ,(Max(A0_, Max(Bvlr_, Cvlr_))        ) * use_vlr_
            )
        );
    // max(A0, B   ) - D
    ic_glp_   <<= Max
        (Max
            ((Max(A0_,     Bgen_        ) - Dgen_) * use_gen_
            ,(Max(A0_,     Bsep_        ) - Dsep_) * use_sep_
            )
        ,Max
            ((Max(A0_,     Bflr_        ) - Dflr_) * use_flr_
            ,(Max(A0_,     Bvlr_        ) - Dvlr_) * use_vlr_
            )
        );
    // max(A1, B, C) - D
    ic_gsp_   <<= Max
        (Max
            ((Max(A1_, Max(Bgen_, Cgen_)) - Dgen_) * use_gen_
            ,(Max(A1_, Max(Bsep_, Csep_)) - Dsep_) * use_sep_
            )
        ,Max
            ((Max(A1_, Max(Bflr_, Cflr_)) - Dflr_) * use_flr_
            ,(Max(A1_, Max(Bvlr_, Cvlr_)) - Dvlr_) * use_vlr_
            )
        );

    // 7702 !! temporary--for acceptance testing
    // Use a temporary stream to avoid changing std::cout's flags.
    if(trace_)
        {
        std::ostringstream oss;
        oss.precision(10);
        oss << std::fixed;
        oss
            << "statutory rates {GLP,GSP}\n"
            << A0_ << " A0_\n"
            << A1_ << " A1_\n"
            << "first-year {B,C,D} with row conditions\n"
            << "  "
            << Bgen_[0] << "\t"
            << Cgen_[0] << "\t"
            << Dgen_[0] << "\tif "
            << static_cast<bool>(use_gen_[0]) << "  general account\n"
            << "  "
            << Bsep_[0] << "\t"
            << Csep_[0] << "\t"
            << Dsep_[0] << "\tif "
            << static_cast<bool>(use_sep_[0]) << "  separate account\n"
            << "  "
            << Bflr_[0] << "\t"
            << Cflr_[0] << "\t"
            << Dflr_[0] << "\tif "
            << static_cast<bool>(use_flr_[0]) << "  fixed loan rate\n"
            << "  "
            << Bvlr_[0] << "\t"
            << Cvlr_[0] << "\t"
            << Dvlr_[0] << "\tif "
            << static_cast<bool>(use_vlr_[0]) << "  variable loan rate\n"
            << "annual rates\n"
            << ic_usual_[0] << " ic_usual_[0]\n"
            << ic_glp_  [0] << " ic_glp_  [0]\n"
            << ic_gsp_  [0] << " ic_gsp_  [0]\n"
            ;
        std::cout << oss.str() << std::endl;
        }

    // Convert all to monthly.
    assign(ic_usual_, apply_unary(i_upper_12_over_12_from_i<double>(), ic_usual_));
    assign(ic_glp_  , apply_unary(i_upper_12_over_12_from_i<double>(), ic_glp_  ));
    assign(ic_gsp_  , apply_unary(i_upper_12_over_12_from_i<double>(), ic_gsp_  ));

    if(!each_equal(Em_, 0.0))
        {
        ig_usual_ <<= Max(ic_usual_, Em_);
        ig_glp_   <<= Max(ic_glp_  , Em_);
        ig_gsp_   <<= Max(ic_gsp_  , Em_);
        }

    if(trace_)
        {
        std::ostringstream oss;
        oss.precision(17);
        oss << std::fixed;
        oss
            << "monthly rates\n"
            << ic_usual_[0] << " ic_usual_[0]\n"
            << ic_glp_  [0] << " ic_glp_  [0]\n"
            << ic_gsp_  [0] << " ic_gsp_  [0]\n"
            << Em_[0] << " Em_[0]\n"
            << ig_usual_[0] << " ig_usual_[0]\n"
            << ig_glp_  [0] << " ig_glp_  [0]\n"
            << ig_gsp_  [0] << " ig_gsp_  [0]\n"
            ;
        std::cout << oss.str() << std::endl;
        }
}
