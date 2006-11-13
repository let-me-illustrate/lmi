// Model class for configurable settings.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "properties_model.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "properties_view.hpp"

#include <stdexcept>
#include <sstream>

namespace
{

// TODO ?? dirty! what is the proper way to retrieve the magic
// "[none]" value?
std::string magic_null_column_name("[none]");

}

PropertiesModel::PropertiesModel()
{
    AscribeMembers();
    LoadFromSettings();
}

PropertiesModel::~PropertiesModel()
{
}

void PropertiesModel::AscribeMembers()
{
    ascribe("CalculationSummaryColumn00", &PropertiesModel::CalculationSummaryColumn00);
    ascribe("CalculationSummaryColumn01", &PropertiesModel::CalculationSummaryColumn01);
    ascribe("CalculationSummaryColumn02", &PropertiesModel::CalculationSummaryColumn02);
    ascribe("CalculationSummaryColumn03", &PropertiesModel::CalculationSummaryColumn03);
    ascribe("CalculationSummaryColumn04", &PropertiesModel::CalculationSummaryColumn04);
    ascribe("CalculationSummaryColumn05", &PropertiesModel::CalculationSummaryColumn05);
    ascribe("CalculationSummaryColumn06", &PropertiesModel::CalculationSummaryColumn06);
    ascribe("CalculationSummaryColumn07", &PropertiesModel::CalculationSummaryColumn07);
    ascribe("CalculationSummaryColumn08", &PropertiesModel::CalculationSummaryColumn08);
    ascribe("CalculationSummaryColumn09", &PropertiesModel::CalculationSummaryColumn09);
    ascribe("CalculationSummaryColumn10", &PropertiesModel::CalculationSummaryColumn10);
    ascribe("CalculationSummaryColumn11", &PropertiesModel::CalculationSummaryColumn11);
}

void PropertiesModel::DoAdaptExternalities()
{
}

datum_base const* PropertiesModel::DoBaseDatumPointer
    (std::string const& name
    ) const
{
    return member_cast<datum_base>(operator[](name));
}

any_entity& PropertiesModel::DoEntity(std::string const& name)
{
    return MemberSymbolTable<PropertiesModel>::operator[](name);
}

any_entity const& PropertiesModel::DoEntity(std::string const& name) const
{
    return MemberSymbolTable<PropertiesModel>::operator[](name);
}

MvcModel::NamesType const& PropertiesModel::DoNames() const
{
    return member_names();
}

MvcModel::StateType PropertiesModel::DoState() const
{
    return member_state(*this);
}

void PropertiesModel::DoCustomizeInitialValues()
{
}

void PropertiesModel::DoEnforceCircumscription(std::string const&)
{
}

void PropertiesModel::DoEnforceProscription(std::string const&)
{
}

void PropertiesModel::DoHarmonize()
{
}

void PropertiesModel::DoTransmogrify()
{
}

bool PropertiesModel::HasErrors(std::vector<std::string>& errors) const
{
    unsigned int not_empty_columns_count = 0;
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const column = operator[](*i).str();
        if(column != magic_null_column_name)
            {
            ++not_empty_columns_count;
            }
        }
    if(0 == not_empty_columns_count)
        {
        errors.push_back("Select at least one column");
        return true;
        }

    // no errors, everything is fine
    return false;
}

bool PropertiesModel::IsModified() const
{
    PropertiesModel unchanged;
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

void PropertiesModel::LoadFromSettings()
{
    std::istringstream iss
        (configurable_settings::instance().calculation_summary_colums()
        );
    std::vector<std::string> columns;
    // split column names list by spaces and put it into a string vector
    std::copy
        (std::istream_iterator<std::string>(iss)
        ,std::istream_iterator<std::string>()
        ,std::back_inserter(columns)
        );

    // member_names() is an immutable and _sorted_ vector of names
    for(std::size_t i = 0; i < member_names().size(); ++i)
        {
        std::string const& name = member_names()[i];
        if(i >= columns.size())
            {
            operator[](name) = magic_null_column_name;
            }
        else
            {
            operator[](name) = columns[i];
            }
        }
}

void PropertiesModel::SaveToSettings() const
{
    std::ostringstream oss;
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const column = operator[](*i).str();
        if(column != magic_null_column_name)
            {
            // use LF instead of a space to make it look better in xml file
            oss << column << "\n";
            }
        }
    configurable_settings::instance().set_calculation_summary_colums
        (oss.str()
        );
}

