// MVC Model for MEC testing.
//
// Copyright (C) 2009, 2010 Gregory W. Chicares.
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

#ifndef mec_input_hpp
#define mec_input_hpp

#include "config.hpp"

#include "mvc_model.hpp"

#include "any_member.hpp"
#include "ce_product_name.hpp"
#include "datum_boolean.hpp"
#include "datum_string.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "tn_range.hpp"
#include "tn_range_types.hpp"
#include "xml_serializable.hpp"

class InputSequence;
class product_database;

#include <boost/operators.hpp>
#include <boost/scoped_ptr.hpp>

#include <string>
#include <vector>

/// Eventually it may become important to distinguish strings that
/// represent input sequences, for interactive validation.

typedef datum_string datum_sequence;

/// This class is the Model of the MVC framework for MEC testing.
///
/// See general notes on class Input.
///
/// These variables:
///   InforceContractYear
///   InforceYear
/// are dependent, but useful. These:
///   DeprecatedUseDOB
///   IssueAge
/// are superfluous, but convenient. These:
///   InforceContractMonth
///   InforceMonth
/// are just excess baggage that can be eliminated once the 7702A
/// calculations are rewritten.

class LMI_SO mec_input
    :virtual private obstruct_slicing           <mec_input>
    ,        public  xml_serializable           <mec_input>
    ,        public  MvcModel
    ,        public  MemberSymbolTable          <mec_input>
    ,        private boost::equality_comparable <mec_input>
{
  public:
    mec_input();
    mec_input(mec_input const&);
    virtual ~mec_input();

    mec_input& operator=(mec_input const&);
    bool operator==(mec_input const&) const;

    std::vector<std::string> RealizeAllSequenceInput(bool report_errors = true);

    int                maturity_age() const {return GleanedMaturityAge_;}

    int years_to_maturity  () const {return maturity_age() - issue_age();}
    int issue_age          () const {return IssueAge     .value();}
    int inforce_year       () const {return InforceYear  .value();}
    int effective_year     () const {return EffectiveDate.value().year();}

    std::vector<double> FlatExtraRealized     () const;
    std::vector<double> PaymentHistoryRealized() const;
    std::vector<double> BenefitHistoryRealized() const;

  private:
    void AscribeMembers();

    // xml_serializable required implementation.
    virtual int         class_version() const;
    virtual std::string xml_root_name() const;

    // xml_serializable overrides.
    virtual bool is_detritus(std::string const&) const;
    virtual void redintegrate_ad_terminum();

    // MvcModel required implementation.
    virtual void DoAdaptExternalities();
    virtual datum_base const* DoBaseDatumPointer(std::string const&) const;
    virtual any_entity      & DoEntity(std::string const&)      ;
    virtual any_entity const& DoEntity(std::string const&) const;
    virtual NamesType const& DoNames() const;
    virtual StateType        DoState() const;
    virtual void DoCustomizeInitialValues();
    virtual void DoEnforceCircumscription(std::string const&);
    virtual void DoEnforceProscription   (std::string const&);
    virtual void DoHarmonize();
    virtual void DoTransmogrify();

    std::string RealizeFlatExtra     ();
    std::string RealizePaymentHistory();
    std::string RealizeBenefitHistory();

    boost::scoped_ptr<product_database> database_;

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
    tnr_issue_age            IssueAge                        ;
    mce_gender               Gender                          ;
    mce_smoking              Smoking                         ;
    mce_class                UnderwritingClass               ;
    tnr_date                 DateOfBirth                     ;
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
    tnr_duration             InforceYear                     ;
    tnr_month                InforceMonth                    ;
    tnr_nonnegative_double   InforceTargetSpecifiedAmount    ;
    tnr_nonnegative_double   InforceAccountValue             ;
    tnr_date                 InforceAsOfDate                 ;
    mce_yes_or_no            InforceIsMec                    ;
    tnr_date                 LastMaterialChangeDate          ;
    tnr_nonnegative_double   InforceDcv                      ;
    tnr_nonnegative_double   InforceAvBeforeLastMc           ;
    tnr_duration             InforceContractYear             ;
    tnr_month                InforceContractMonth            ;
    tnr_nonnegative_double   InforceLeastDeathBenefit        ;
    mce_state                StateOfJurisdiction             ;
    datum_sequence           FlatExtra                       ;
    datum_sequence           PaymentHistory                  ;
    datum_sequence           BenefitHistory                  ;
    mce_yes_or_no            DeprecatedUseDOB                ;
    tnr_nonnegative_double   Payment                         ;
    tnr_nonnegative_double   BenefitAmount                   ;

    std::vector<tnr_unrestricted_double> FlatExtraRealized_     ;
    std::vector<tnr_unrestricted_double> PaymentHistoryRealized_;
    std::vector<tnr_unrestricted_double> BenefitHistoryRealized_;
};

/// Specialization of struct template reconstitutor for this Model
/// and the base class that all its UDTs share.

template<> struct reconstitutor<datum_base, mec_input>
{
    typedef datum_base DesiredType;
    static DesiredType* reconstitute(any_member<mec_input>& m)
        {
        DesiredType* z = 0;
        z = exact_cast<ce_product_name         >(m); if(z) return z;
        z = exact_cast<datum_string            >(m); if(z) return z;
        z = exact_cast<mce_class               >(m); if(z) return z;
        z = exact_cast<mce_defn_life_ins       >(m); if(z) return z;
        z = exact_cast<mce_defn_material_change>(m); if(z) return z;
        z = exact_cast<mce_gender              >(m); if(z) return z;
        z = exact_cast<mce_smoking             >(m); if(z) return z;
        z = exact_cast<mce_state               >(m); if(z) return z;
        z = exact_cast<mce_table_rating        >(m); if(z) return z;
        z = exact_cast<mce_uw_basis            >(m); if(z) return z;
        z = exact_cast<mce_yes_or_no           >(m); if(z) return z;
        z = exact_cast<tnr_date                >(m); if(z) return z;
        z = exact_cast<tnr_duration            >(m); if(z) return z;
        z = exact_cast<tnr_issue_age           >(m); if(z) return z;
        z = exact_cast<tnr_month               >(m); if(z) return z;
        z = exact_cast<tnr_nonnegative_double  >(m); if(z) return z;
        z = exact_cast<tnr_unrestricted_double >(m); if(z) return z;
        return z;
        }
};

#endif // mec_input_hpp

