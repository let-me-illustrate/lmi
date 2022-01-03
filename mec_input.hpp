// MVC Model for MEC testing.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef mec_input_hpp
#define mec_input_hpp

#include "config.hpp"

#include "mvc_model.hpp"

#include "any_member.hpp"
#include "ce_product_name.hpp"
#include "datum_boolean.hpp"
#include "datum_sequence.hpp"
#include "datum_string.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "so_attributes.hpp"
#include "tn_range.hpp"
#include "tn_range_types.hpp"
#include "xml_serializable.hpp"

class product_database;

#include <memory>                       // unique_ptr
#include <string>
#include <vector>

/// This class is the Model of the MVC framework for MEC testing.
///
/// See general notes on class Input.
///
/// These variables:
///   InforceContractYear
///   InforceYear
/// are dependent, but useful. These:
///   UseDOB
///   IssueAge
/// are superfluous, but convenient. These:
///   InforceContractMonth
///   InforceMonth
/// are just excess baggage that can be eliminated once the 7702A
/// calculations are rewritten.

class LMI_SO mec_input final
    :public  xml_serializable           <mec_input>
    ,public  MvcModel
    ,public  MemberSymbolTable          <mec_input>
{
  public:
    mec_input();
    mec_input(mec_input const&);
    ~mec_input() override;

    mec_input& operator=(mec_input const&);
    bool operator==(mec_input const&) const;

    int                maturity_age() const;

    int years_to_maturity  () const;
    int issue_age          () const;
    int inforce_year       () const;
    int effective_year     () const;

    std::vector<std::string> RealizeAllSequenceInput(bool report_errors = true);

    // TAXATION !! Remove because flats and tables are ignored?
    std::vector<double> FlatExtraRealized     () const;
    std::vector<double> PaymentHistoryRealized() const;
    std::vector<double> BenefitHistoryRealized() const;

  private:
    void AscribeMembers();

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    bool is_detritus(std::string const&) const override;
    void redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,std::string      & value
        ) const override;
    void redintegrate_ex_post
        (int                                       file_version
        ,std::map<std::string, std::string> const& detritus_map
        ,std::list<std::string>             const& residuary_names
        ) override;
    void redintegrate_ad_terminum() override;

    // MvcModel required implementation.
    void DoAdaptExternalities() override;
    datum_base const* DoBaseDatumPointer(std::string const&) const override;
    any_entity      & DoEntity(std::string const&) override      ;
    any_entity const& DoEntity(std::string const&) const override;
    NamesType const& DoNames() const override;
    StateType        DoState() const override;
    void DoCustomizeInitialValues() override;
    void DoEnforceCircumscription(std::string const&) override;
    void DoEnforceProscription   (std::string const&) override;
    void DoHarmonize() override;
    void DoTransmogrify() override;

    // TAXATION !! Remove because flats and tables are ignored?
    std::string RealizeFlatExtra     ();
    std::string RealizePaymentHistory();
    std::string RealizeBenefitHistory();

    std::unique_ptr<product_database> database_;

    // Database axes are independent variables; they're "cached" along
    // with the database, which is reset when any of them changes.
    // Dependent variables, stored only as an optimization, are
    // "gleaned" whenever the database is reset.
    std::string              CachedProductName_          ;
    mcenum_gender            CachedGender_               ;
    mcenum_class             CachedUnderwritingClass_    ;
    mcenum_smoking           CachedSmoking_              ;
    int                      CachedIssueAge_             ;
    mcenum_uw_basis          CachedGroupUnderwritingType_;
    mcenum_state             CachedStateOfJurisdiction_  ;
    int                      GleanedMaturityAge_         ;

    mce_yes_or_no            Use7702ATables                  ;
    tnr_age                  IssueAge                        ;
    mce_gender               Gender                          ;
    mce_smoking              Smoking                         ;
    mce_class                UnderwritingClass               ;
    tnr_date                 DateOfBirth                     ;
    // TAXATION !! Remove because flats and tables are ignored?
    mce_table_rating         SubstandardTable                ;
    ce_product_name          ProductName                     ;
    tnr_nonnegative_double   External1035ExchangeAmount      ;
    mce_yes_or_no            External1035ExchangeFromMec     ;
    tnr_nonnegative_double   Internal1035ExchangeAmount      ;
    mce_yes_or_no            Internal1035ExchangeFromMec     ;
    tnr_date                 EffectiveDate                   ;
    mce_defn_life_ins        DefinitionOfLifeInsurance       ;
    mce_defn_material_change DefinitionOfMaterialChange      ;
    mce_uw_basis             GroupUnderwritingType           ;
    datum_string             Comments                        ;
    tnr_date                 InforceAsOfDate                 ;
    tnr_duration             InforceYear                     ;
    tnr_month                InforceMonth                    ;
    tnr_nonnegative_double   InforceTargetSpecifiedAmount    ;
    tnr_nonnegative_double   InforceAccountValue             ;
    mce_yes_or_no            InforceIsMec                    ;
    tnr_date                 LastMaterialChangeDate          ;
    tnr_duration             InforceContractYear             ;
    tnr_month                InforceContractMonth            ;
    tnr_nonnegative_double   InforceAvBeforeLastMc           ;
    tnr_nonnegative_double   InforceDcv                      ;
    tnr_nonnegative_double   InforceLeastDeathBenefit        ;
    numeric_sequence         PaymentHistory                  ;
    numeric_sequence         BenefitHistory                  ;
    mce_state                StateOfJurisdiction             ;
    mce_state                PremiumTaxState                 ;
    // TAXATION !! Remove because flats and tables are ignored?
    numeric_sequence         FlatExtra                       ;
    mce_yes_or_no            UseDOB                          ;
    tnr_nonnegative_double   Payment                         ;
    tnr_nonnegative_double   BenefitAmount                   ;

    // TAXATION !! Remove because flats and tables are ignored?
    std::vector<tnr_unrestricted_double> FlatExtraRealized_     ;
    std::vector<tnr_unrestricted_double> PaymentHistoryRealized_;
    std::vector<tnr_unrestricted_double> BenefitHistoryRealized_;
};

/// Specialization of struct template reconstitutor for this Model
/// and the base class that all its UDTs share.

template<> struct reconstitutor<datum_base,mec_input>
{
    typedef datum_base DesiredType;
    static DesiredType* reconstitute(any_member<mec_input>& m)
        {
        DesiredType* z = nullptr;
        z = exact_cast<ce_product_name         >(m); if(z) return z;
        z = exact_cast<datum_string            >(m); if(z) return z;
        // Sequences.
        z = exact_cast<numeric_sequence        >(m); if(z) return z;
        // mc- types.
        z = exact_cast<mce_class               >(m); if(z) return z;
        z = exact_cast<mce_defn_life_ins       >(m); if(z) return z;
        z = exact_cast<mce_defn_material_change>(m); if(z) return z;
        z = exact_cast<mce_gender              >(m); if(z) return z;
        z = exact_cast<mce_smoking             >(m); if(z) return z;
        z = exact_cast<mce_state               >(m); if(z) return z;
        z = exact_cast<mce_table_rating        >(m); if(z) return z;
        z = exact_cast<mce_uw_basis            >(m); if(z) return z;
        z = exact_cast<mce_yes_or_no           >(m); if(z) return z;
        // tnr- types.
        z = exact_cast<tnr_age                 >(m); if(z) return z;
        z = exact_cast<tnr_date                >(m); if(z) return z;
        z = exact_cast<tnr_duration            >(m); if(z) return z;
        z = exact_cast<tnr_month               >(m); if(z) return z;
        z = exact_cast<tnr_nonnegative_double  >(m); if(z) return z;
        z = exact_cast<tnr_unrestricted_double >(m); if(z) return z;
        return z;
        }
};

#endif // mec_input_hpp
