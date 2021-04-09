// MVC Model base class.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "mvc_model.hpp"

#include "alert.hpp"
#include "any_entity.hpp"
#include "assert_lmi.hpp"

namespace
{
void ComplainAboutAnyDiscrepancies
    (MvcModel::StateType const& old_values
    ,MvcModel::StateType const& new_values
    ,std::string const&  description
    )
{
    if(old_values == new_values)
        {
        return;
        }

    warning() << description << '\n';
    MvcModel::StateType::const_iterator i = old_values.begin();
    MvcModel::StateType::const_iterator j = new_values.begin();
    LMI_ASSERT(old_values.size() == new_values.size());
    for(; i != old_values.end(); ++i, ++j)
        {
        LMI_ASSERT(i->first == j->first);
        if(i->second != j->second)
            {
            warning()
                << "  "
                << i->first
                << " must change from '"
                << i->second
                << "' to '"
                << j->second
                << "'\n"
                ;
            }
        }
    warning() << LMI_FLUSH;
}
} // Unnamed namespace.

datum_base const* MvcModel::BaseDatumPointer(std::string const& name) const
{
    return DoBaseDatumPointer(name);
}

any_entity& MvcModel::Entity(std::string const& name)
{
    return DoEntity(name);
}

any_entity const& MvcModel::Entity(std::string const& name) const
{
    return DoEntity(name);
}

MvcModel::NamesType const& MvcModel::Names() const
{
    return DoNames();
}

MvcModel::StateType MvcModel::State() const
{
    return DoState();
}

void MvcModel::Reconcile()
{
    StateType new_values = State();
    StateType old_values;

    bool okay = false;
    int j = 0;
    int const maximum_iterations = 100;

    for(; !okay && j < maximum_iterations; ++j)
        {
        AdaptExternalities();
        Harmonize();
        Transmogrify();
        old_values.swap(new_values);
        new_values = State();
        okay = new_values == old_values;
        }

    if(!okay)
        {
        alarum()
            << "Unable to make Model consistent after "
            << maximum_iterations
            << " iterations."
            << LMI_FLUSH
            ;
        }
}

void MvcModel::TestInitialConsistency()
{
    std::string description;
    StateType old_values;
    StateType new_values;

    description = "Static initial values are inconsistent with rules:";
    AdaptExternalities();
    old_values = State();
    Harmonize();
    Transmogrify();
    new_values = State();
    ComplainAboutAnyDiscrepancies(old_values, new_values, description);

    description = "Dynamic initial values are inconsistent with rules:";
    CustomizeInitialValues();
    AdaptExternalities();
    old_values = State();
    Harmonize();
    Transmogrify();
    new_values = State();
    ComplainAboutAnyDiscrepancies(old_values, new_values, description);
}

void MvcModel::AdaptExternalities()
{
    DoAdaptExternalities();
}

void MvcModel::CustomizeInitialValues()
{
    DoCustomizeInitialValues();
    AdaptExternalities();
    Harmonize();
    Transmogrify();
}

void MvcModel::Harmonize()
{
    StateType const old_values = State();
    DoHarmonize();
    StateType const new_values = State();
    std::string description = "Harmonize() improperly forces values to change:";
    ComplainAboutAnyDiscrepancies(old_values, new_values, description);
}

void MvcModel::Transmogrify()
{
    for(auto const& i : Names())
        {
        DoEnforceCircumscription(i);
        DoEnforceProscription   (i);
        }
    DoTransmogrify();
}
