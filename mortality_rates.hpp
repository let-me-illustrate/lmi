// Mortality rates.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: mortality_rates.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef mortality_rates_hpp
#define mortality_rates_hpp

#include "config.hpp"

#include "xenumtypes.hpp"

#include <vector>

// Someday, add other rates, such as:
//   1983 GAM for experience rating
//   experience q for pricing
//   q for basic reserves
//   q for deficiency reserves
//   q for tax reserves
//   q for nonforfeiture calculations
//   New York minimum COI rate
//   80 CSO with select factors for NY Reg 112 section 47.2(a)(3) ["XXX"]
// When that's done, it may make sense to reduce initialization
// overhead by calculating each private member's value only when
// it's first needed.

// Adding 7702A premium rates and CVAT corridor factors extended the
// scope of this class beyond mortality rates. We might either change
// its name or move such members elsewhere.

class BasicValues;

class MortalityRates
{
  public:
    MortalityRates(BasicValues const&);

    void SetDynamicCOIRate
        (double*        YearsCOIRate
        ,e_basis const& Basis
        ,int            Year
        ,double         CaseExpRatReserve
        ) const;

    std::vector<double> const& MonthlyCoiRates(enum_basis)                const;
    std::vector<double> const& MonthlyCoiRatesBand0(e_basis const& Basis) const;
    std::vector<double> const& MonthlyCoiRatesBand1(e_basis const& Basis) const;
    std::vector<double> const& MonthlyCoiRatesBand2(e_basis const& Basis) const;

    std::vector<double> const& MonthlyTermCoiRates (e_basis const& Basis) const;
    std::vector<double> const& ADDRates            () const;
    std::vector<double> const& WPRates             () const;
    std::vector<double> const& ChildRiderRates     () const;
    std::vector<double> const& SpouseRiderRates    (e_basis const& Basis) const;

    std::vector<double> const& TargetPremiumRates  () const;

    std::vector<double> const& Irc7702Q            () const;
    std::vector<double> const& TableYRates         () const;
    std::vector<double> const& PartialMortalityQ   () const;
    std::vector<double> const& CvatCorridorFactors () const;
    std::vector<double> const& SevenPayRates       () const;
    std::vector<double> const& CvatNspRates        () const;

  private:
    MortalityRates();

    void Init(BasicValues const&);

    void SetGuaranteedRates(BasicValues const&);
    void SetNonguaranteedRates(BasicValues const&);
    void SetOneNonguaranteedRateBand
        (std::vector<double>      & coi_rates
        ,std::vector<double> const& coi_multiplier
        ,BasicValues         const& basic_values
        );
    void SetOtherRates(BasicValues const&);

    void MakeCoiRateSubstandard
        (std::vector<double>      & coi_rates
        ,BasicValues         const& basic_values
        );
    void GradeCurrentCoiRatesFromPartialMortalityAssumption
        (std::vector<double>      & coi_rates
        ,std::vector<double> const& monthly_partial_mortality
        ,std::vector<double> const& grading_factors
        ,BasicValues         const& basic_values
        );

    int Length_;
    double MaxMonthlyRate;

    // Experience-rating COI retention.
    double AdditiveCoiRetention_;
    double MultiplicativeCoiRetention_;

// MonthlyCoiRatesBand2
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
    std::vector<double> ADDRates_;
    std::vector<double> WPRates_;
    std::vector<double> ChildRiderRates_;
    std::vector<double> GuaranteedSpouseRiderRates_;
    std::vector<double> CurrentSpouseRiderRates_;
    std::vector<double> MidpointSpouseRiderRates_;

    std::vector<double> TargetPremiumRates_;

    // This 'alternative' COI rate is used for experience rated group
    // products when the mortality reserve is negative, and is based
    // on the guaranteed COI table.
    std::vector<double> AlternativeMonthlyCoiRates_;

    std::vector<double> Irc7702Q_;
    std::vector<double> TableYRates_;
    std::vector<double> PartialMortalityQ_;
    std::vector<double> CvatCorridorFactors_;
    std::vector<double> SevenPayRates_;
    std::vector<double> CvatNspRates_;
};

inline std::vector<double> const& MortalityRates::ADDRates() const
{
    return ADDRates_;
}

inline std::vector<double> const& MortalityRates::WPRates() const
{
    return WPRates_;
}

inline std::vector<double> const& MortalityRates::ChildRiderRates() const
{
    return ChildRiderRates_;
}

inline std::vector<double> const& MortalityRates::Irc7702Q() const
{
    return Irc7702Q_;
}

inline std::vector<double> const& MortalityRates::TableYRates() const
{
    return TableYRates_;
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

