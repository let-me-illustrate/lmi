// MVC Model for user preferences.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef preferences_model_hpp
#define preferences_model_hpp

#include "config.hpp"

#include "mvc_model.hpp"

#include "any_member.hpp"
#include "ce_skin_name.hpp"
#include "datum_string.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "so_attributes.hpp"
#include "tn_range.hpp"
#include "tn_range_types.hpp"

#include <string>

class LMI_SO PreferencesModel final
    :public MvcModel
    ,public MemberSymbolTable<PreferencesModel>
{
  public:
    PreferencesModel();
    ~PreferencesModel() override;

    bool IsModified() const;
    void Load();
    void Save() const;

  private:
    void AscribeMembers();

    std::string string_of_column_names() const;

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

    mce_report_column       CalculationSummaryColumn00;
    mce_report_column       CalculationSummaryColumn01;
    mce_report_column       CalculationSummaryColumn02;
    mce_report_column       CalculationSummaryColumn03;
    mce_report_column       CalculationSummaryColumn04;
    mce_report_column       CalculationSummaryColumn05;
    mce_report_column       CalculationSummaryColumn06;
    mce_report_column       CalculationSummaryColumn07;
    mce_report_column       CalculationSummaryColumn08;
    mce_report_column       CalculationSummaryColumn09;
    mce_report_column       CalculationSummaryColumn10;
    mce_report_column       CalculationSummaryColumn11;
    mce_yes_or_no           CensusPastePalimpsestically;
    datum_string            DefaultInputFilename;
    datum_string            PrintDirectory;
    tnr_nonnegative_integer SecondsToPauseBetweenPrintouts;
    ce_skin_name            SkinFileName;
    mce_yes_or_no           UseBuiltinCalculationSummary;
};

/// Specialization of struct template reconstitutor for this Model
/// and the base class that all its UDTs share.

template<> struct reconstitutor<datum_base,PreferencesModel>
{
    typedef datum_base DesiredType;
    static DesiredType* reconstitute(any_member<PreferencesModel>& m)
        {
        DesiredType* z = nullptr;
        z = exact_cast<ce_skin_name            >(m); if(z) return z;
        z = exact_cast<datum_string            >(m); if(z) return z;
        z = exact_cast<mce_report_column       >(m); if(z) return z;
        z = exact_cast<mce_yes_or_no           >(m); if(z) return z;
        z = exact_cast<tnr_nonnegative_integer >(m); if(z) return z;
        return z;
        }
};

#endif // preferences_model_hpp
