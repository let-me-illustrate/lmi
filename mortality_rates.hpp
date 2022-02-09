// Mortality rates.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef mortality_rates_hpp
#define mortality_rates_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "round_to.hpp"

#include <vector>

class BasicValues;

/// Design notes--class MortalityRates
///
/// This class encapsulates mortality rates.
///
/// TODO ?? It also includes certain other types of rates that happen
/// to be stored using the same technology as true mortality rates.
/// It is not yet clear whether the class should be renamed or the
/// other rates should be moved elsewhere. The set of other rates
/// might someday be expanded to include
///   1983 GAM for experience rating
///   experience q for pricing
///   q for basic reserves
///   q for deficiency reserves
///   q for tax reserves
///   q for nonforfeiture calculations
///   New York minimum COI rate
///   80 CSO with select factors for NY Reg 112 section 47.2(a)(3) ["XXX"]
/// When that's done, it may make sense to reduce initialization
/// overhead by calculating each private member's value only when
/// it's first needed.

class MortalityRates
{
    friend class mortality_rates_test;

  public:
    MortalityRates(BasicValues const&);

    std::vector<double> const& MonthlyCoiRates(mcenum_gen_basis) const; // Antediluvian.

    std::vector<double> const& MonthlyCoiRatesBand0(mcenum_gen_basis) const;
    std::vector<double> const& MonthlyCoiRatesBand1(mcenum_gen_basis) const;
    std::vector<double> const& MonthlyCoiRatesBand2(mcenum_gen_basis) const;

    std::vector<double> const& MonthlyTermCoiRates (mcenum_gen_basis) const;
    std::vector<double> const& AdbRates            () const;
    std::vector<double> const& WpRates             () const;
    std::vector<double> const& ChildRiderRates     () const;
    std::vector<double> const& SpouseRiderRates    (mcenum_gen_basis) const;

    std::vector<double> const& MinimumPremiumRates () const;
    std::vector<double> const& TargetPremiumRates  () const;

    std::vector<double> const& Irc7702Q            () const;
    std::vector<double> const& GroupProxyRates     () const;
    std::vector<double> const& PartialMortalityQ   () const;
    std::vector<double> const& CvatCorridorFactors () const;
    std::vector<double> const& SevenPayRates       () const;
    std::vector<double> const& CvatNspRates        () const;

  private:
    MortalityRates(); // For unit testing only.

    void Init(BasicValues const&); // Antediluvian.

    void reserve_vectors();
    void fetch_parameters(BasicValues const&);
    void initialize();

    void SetCoiRates
        (std::vector<double>      & coi_rates
        ,std::vector<double> const& coi_multiplier
        ,std::vector<double> const& maximum
        ,bool                       table_is_annual
        );
    void SetGuaranteedRates();
    void SetNonguaranteedRates();
    void SetOneNonguaranteedRateBand
        (std::vector<double>      & coi_rates
        ,std::vector<double> const& coi_multiplier
        );
    void SetOtherRates();

    void MakeCoiRateSubstandard(std::vector<double>& coi_rates);

    int Length_;

    bool AllowAdb_        ;
    bool AllowChild_      ;
    bool AllowFlatExtras_ ;
    bool AllowSpouse_     ;
    bool AllowSubstdTable_;
    bool AllowTerm_       ;
    bool AllowWp_         ;
    bool CCoiIsAnnual_    ;
    bool GCoiIsAnnual_    ;
    bool IsTgtPremTabular_;

    double MaxMonthlyCoiRate_;

    double CountryCoiMultiplier_;
    bool IsPolicyRated_;
    mcenum_table_rating SubstandardTable_;

    std::vector<double> CurrentCoiMultiplier_;
    std::vector<double> AnnualFlatExtra_;
    std::vector<double> PartialMortalityMultiplier_;

    round_to<double> round_coi_rate_;

    std::vector<double> GCoiMultiplier_;
    std::vector<double> CCoiMultiplier_;
    std::vector<double> SubstdTblMult_;

    std::vector<double> MonthlyGuaranteedCoiRates_;
    std::vector<double> MonthlyCurrentCoiRatesBand0_;
    std::vector<double> MonthlyCurrentCoiRatesBand1_;
    std::vector<double> MonthlyCurrentCoiRatesBand2_;
    std::vector<double> MonthlyMidpointCoiRatesBand0_;
    std::vector<double> MonthlyMidpointCoiRatesBand1_;
    std::vector<double> MonthlyMidpointCoiRatesBand2_;

    std::vector<double> MonthlyGuaranteedTermCoiRates_;
    std::vector<double> MonthlyCurrentTermCoiRates_;
    std::vector<double> MonthlyMidpointTermCoiRates_;
    std::vector<double> AdbRates_;
    std::vector<double> WpRates_;
    std::vector<double> ChildRiderRates_;
    std::vector<double> GuaranteedSpouseRiderRates_;
    std::vector<double> CurrentSpouseRiderRates_;
    std::vector<double> MidpointSpouseRiderRates_;

    std::vector<double> MinimumPremiumRates_;
    std::vector<double> TargetPremiumRates_;

    std::vector<double> Irc7702Q_;
    std::vector<double> GroupProxyRates_;
    std::vector<double> PartialMortalityQ_;
    std::vector<double> CvatCorridorFactors_;
    std::vector<double> SevenPayRates_;
    std::vector<double> CvatNspRates_;
};

inline std::vector<double> const& MortalityRates::AdbRates() const
{
    return AdbRates_;
}

inline std::vector<double> const& MortalityRates::WpRates() const
{
    return WpRates_;
}

inline std::vector<double> const& MortalityRates::ChildRiderRates() const
{
    return ChildRiderRates_;
}

inline std::vector<double> const& MortalityRates::MinimumPremiumRates() const
{
    return MinimumPremiumRates_;
}

inline std::vector<double> const& MortalityRates::TargetPremiumRates() const
{
    return TargetPremiumRates_;
}

inline std::vector<double> const& MortalityRates::Irc7702Q() const
{
    return Irc7702Q_;
}

inline std::vector<double> const& MortalityRates::GroupProxyRates() const
{
    return GroupProxyRates_;
}

inline std::vector<double> const& MortalityRates::PartialMortalityQ() const
{
    return PartialMortalityQ_;
}

inline std::vector<double> const& MortalityRates::CvatCorridorFactors() const
{
    return CvatCorridorFactors_;
}

inline std::vector<double> const& MortalityRates::SevenPayRates() const
{
    return SevenPayRates_;
}

inline std::vector<double> const& MortalityRates::CvatNspRates() const
{
    return CvatNspRates_;
}

#endif // mortality_rates_hpp
