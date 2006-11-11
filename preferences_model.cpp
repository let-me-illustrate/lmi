// MVC Model for user preferences.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: preferences_model.cpp,v 1.2 2006-11-11 02:12:13 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
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
    :UseBuiltinDefaults("No")
{
    AscribeMembers();
    LoadFromSettings();
}

PreferencesModel::~PreferencesModel()
{
}

void PreferencesModel::AscribeMembers()
{
    ascribe("UseBuiltinDefaults"        , &PreferencesModel::UseBuiltinDefaults        );
    ascribe("CalculationSummaryColumn00", &PreferencesModel::CalculationSummaryColumn00);
    ascribe("CalculationSummaryColumn01", &PreferencesModel::CalculationSummaryColumn01);
    ascribe("CalculationSummaryColumn02", &PreferencesModel::CalculationSummaryColumn02);
    ascribe("CalculationSummaryColumn03", &PreferencesModel::CalculationSummaryColumn03);
    ascribe("CalculationSummaryColumn04", &PreferencesModel::CalculationSummaryColumn04);
    ascribe("CalculationSummaryColumn05", &PreferencesModel::CalculationSummaryColumn05);
    ascribe("CalculationSummaryColumn06", &PreferencesModel::CalculationSummaryColumn06);
    ascribe("CalculationSummaryColumn07", &PreferencesModel::CalculationSummaryColumn07);
    ascribe("CalculationSummaryColumn08", &PreferencesModel::CalculationSummaryColumn08);
    ascribe("CalculationSummaryColumn09", &PreferencesModel::CalculationSummaryColumn09);
    ascribe("CalculationSummaryColumn10", &PreferencesModel::CalculationSummaryColumn10);
    ascribe("CalculationSummaryColumn11", &PreferencesModel::CalculationSummaryColumn11);
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
}

void PreferencesModel::DoTransmogrify()
{
}

bool PreferencesModel::IsModified() const
{
    PreferencesModel unchanged;
    unchanged.LoadFromSettings();
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

// TODO ?? CALCULATION_SUMMARY Implement 'UseBuiltinDefaults'.

// TODO ?? CALCULATION_SUMMARY Warn if zero columns selected.

void PreferencesModel::LoadFromSettings()
{
    std::istringstream iss
        (configurable_settings::instance().calculation_summary_columns()
        );
    std::vector<std::string> columns;
    std::copy
        (std::istream_iterator<std::string>(iss)
        ,std::istream_iterator<std::string>()
        ,std::back_inserter(columns)
        );

    // TODO ?? CALCULATION_SUMMARY '-1 +' is a poor way of ignoring
    // 'UseBuiltinDefaults'.
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

void PreferencesModel::SaveToSettings() const
{
    std::ostringstream oss;
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        // TODO ?? CALCULATION_SUMMARY This is poor.
        if("UseBuiltinDefaults" == *i)
            {
            continue;
            }
        std::string const column = operator[](*i).str();
        if(column != magic_null_column_name)
            {
            oss << column << "\n";
            }
        }
    configurable_settings::instance().calculation_summary_columns(oss.str());
}

