// MVC Model for user preferences.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "preferences_model.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"

#include <cstddef> // std::size_t
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace
{
// TODO ?? CALCULATION_SUMMARY Evgeniy has asked:
// what is the proper way to retrieve the magic "[none]" value?
//
// Answer: I guess I'd clone some similar code, like
// configuration_filename() in 'configurable_settings.cpp', just for
// consistency; and then, what is most important, search for every use
// of that string in every other module and replace them all with
// calls to a function like that--which therefore might belong in some
// other module.
//
// However, it would be even better to reimplement this class to
// configure every data member in class configurable_settings, and
// store the twelve instances of mce_report_column here the same way
// that class 'Input' stores its twelve in its xml file. Then the
// whole issue would vanish.

std::string magic_null_column_name("[none]");
}

PreferencesModel::PreferencesModel()
    :UseBuiltinCalculationSummary("No")
{
    AscribeMembers();
    Load();
}

PreferencesModel::~PreferencesModel()
{
}

void PreferencesModel::AscribeMembers()
{
    ascribe("UseBuiltinCalculationSummary", &PreferencesModel::UseBuiltinCalculationSummary);
    ascribe("CalculationSummaryColumn00"  , &PreferencesModel::CalculationSummaryColumn00);
    ascribe("CalculationSummaryColumn01"  , &PreferencesModel::CalculationSummaryColumn01);
    ascribe("CalculationSummaryColumn02"  , &PreferencesModel::CalculationSummaryColumn02);
    ascribe("CalculationSummaryColumn03"  , &PreferencesModel::CalculationSummaryColumn03);
    ascribe("CalculationSummaryColumn04"  , &PreferencesModel::CalculationSummaryColumn04);
    ascribe("CalculationSummaryColumn05"  , &PreferencesModel::CalculationSummaryColumn05);
    ascribe("CalculationSummaryColumn06"  , &PreferencesModel::CalculationSummaryColumn06);
    ascribe("CalculationSummaryColumn07"  , &PreferencesModel::CalculationSummaryColumn07);
    ascribe("CalculationSummaryColumn08"  , &PreferencesModel::CalculationSummaryColumn08);
    ascribe("CalculationSummaryColumn09"  , &PreferencesModel::CalculationSummaryColumn09);
    ascribe("CalculationSummaryColumn10"  , &PreferencesModel::CalculationSummaryColumn10);
    ascribe("CalculationSummaryColumn11"  , &PreferencesModel::CalculationSummaryColumn11);
}

void PreferencesModel::DoAdaptExternalities()
{
}

datum_base const* PreferencesModel::DoBaseDatumPointer
    (std::string const& name
    ) const
{
    return member_cast<datum_base>(operator[](name));
}

any_entity& PreferencesModel::DoEntity(std::string const& name)
{
    return MemberSymbolTable<PreferencesModel>::operator[](name);
}

any_entity const& PreferencesModel::DoEntity(std::string const& name) const
{
    return MemberSymbolTable<PreferencesModel>::operator[](name);
}

MvcModel::NamesType const& PreferencesModel::DoNames() const
{
    return member_names();
}

MvcModel::StateType PreferencesModel::DoState() const
{
    return member_state(*this);
}

void PreferencesModel::DoCustomizeInitialValues()
{
}

void PreferencesModel::DoEnforceCircumscription(std::string const&)
{
}

void PreferencesModel::DoEnforceProscription(std::string const&)
{
}

void PreferencesModel::DoHarmonize()
{
    bool do_not_use_builtin_defaults = "No" == UseBuiltinCalculationSummary;
    CalculationSummaryColumn00.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn01.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn02.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn03.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn04.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn05.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn06.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn07.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn08.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn09.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn10.enable(do_not_use_builtin_defaults);
    CalculationSummaryColumn11.enable(do_not_use_builtin_defaults);

    // TODO ?? EGREGIOUS_DEFECT: instead, don't offer these columns at all.
    std::vector<mcenum_report_column> weird_report_columns;
    weird_report_columns.push_back(mce_current_0_account_value_general_account    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_general_account );
    weird_report_columns.push_back(mce_current_0_account_value_separate_account   );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_separate_account);
    weird_report_columns.push_back(mce_current_0_account_value                    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value                 );
    weird_report_columns.push_back(mce_current_0_cash_surrender_value             );
    weird_report_columns.push_back(mce_guaranteed_0_cash_surrender_value          );

    typedef std::vector<mcenum_report_column>::const_iterator vrci;
    for(vrci i = weird_report_columns.begin(); i != weird_report_columns.end(); ++i)
        {
        CalculationSummaryColumn00.allow(*i, false);
        CalculationSummaryColumn01.allow(*i, false);
        CalculationSummaryColumn02.allow(*i, false);
        CalculationSummaryColumn03.allow(*i, false);
        CalculationSummaryColumn04.allow(*i, false);
        CalculationSummaryColumn05.allow(*i, false);
        CalculationSummaryColumn06.allow(*i, false);
        CalculationSummaryColumn07.allow(*i, false);
        CalculationSummaryColumn08.allow(*i, false);
        CalculationSummaryColumn09.allow(*i, false);
        CalculationSummaryColumn10.allow(*i, false);
        CalculationSummaryColumn11.allow(*i, false);
        }
}

void PreferencesModel::DoTransmogrify()
{
}

bool PreferencesModel::IsModified() const
{
    PreferencesModel unchanged;
    unchanged.Load();
    if(unchanged.UseBuiltinCalculationSummary != UseBuiltinCalculationSummary)
        {
        return true;
        }
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        if(operator[](*i) != unchanged[*i])
            {
            return true;
            }
        }
    return false;
}

void PreferencesModel::Load()
{
    configurable_settings const& z = configurable_settings::instance();
    std::istringstream iss(z.calculation_summary_columns());
    std::vector<std::string> columns;
    std::copy
        (std::istream_iterator<std::string>(iss)
        ,std::istream_iterator<std::string>()
        ,std::back_inserter(columns)
        );

    bool b = z.use_builtin_calculation_summary();
    UseBuiltinCalculationSummary = b ? "Yes" : "No";

    // TODO ?? CALCULATION_SUMMARY '-1 +' is a poor way of ignoring
    // 'UseBuiltinCalculationSummary'.
    for(std::size_t i = 0; i < -1 + member_names().size(); ++i)
        {
        std::string const& name = member_names()[i];
        if(columns.size() <= i)
            {
            operator[](name) = magic_null_column_name;
            }
        else
            {
            operator[](name) = columns[i];
            }
        }
}

void PreferencesModel::Save() const
{
    std::ostringstream oss;
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        // TODO ?? CALCULATION_SUMMARY This is poor.
        if("UseBuiltinCalculationSummary" == *i)
            {
            continue;
            }
        std::string const column = operator[](*i).str();
        if(column != magic_null_column_name)
            {
            oss << column << " ";
            }
        }
    if(oss.str().empty() && "Yes" != UseBuiltinCalculationSummary)
        {
        warning()
            << "Calculation summary will be empty: no columns chosen."
            << LMI_FLUSH
            ;
        }
    configurable_settings& z = configurable_settings::instance();
    z.calculation_summary_columns(oss.str());
    z.use_builtin_calculation_summary("Yes" == UseBuiltinCalculationSummary);
}

