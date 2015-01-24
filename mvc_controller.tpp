// MVC Controller.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#include "mvc_controller.hpp"

#include "alert.hpp"
#include "any_entity.hpp"
#include "mvc_model.hpp"
#include "rtti_lmi.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_same.hpp>

#include <wx/window.h>
#include <wx/xrc/xmlres.h>

template<typename T>
T const* MvcController::ModelPointer(std::string const& name) const
{
    return dynamic_cast<T const*>(model_.BaseDatumPointer(name));
}

template<typename T>
T const& MvcController::ModelReference(std::string const& name) const
{
    T const* p = ModelPointer<T>(name);
    if(!p)
        {
        fatal_error()
            << "Cannot convert Model datum '"
            << name
            << "' of type '"
            << lmi::TypeInfo(model_.Entity(name).type())
            << "' to type '"
            << lmi::TypeInfo(typeid(T))
            << "'."
            << LMI_FLUSH
            ;
        }
    return *p;
}

// TODO ?? Consider a free-function implementation using static functions
// FindWindowById() and FindWindowByName(). Then consider turning these:
//   Bind()
//   CurrentPage()
//   DiagnosticsWindow()
//   EnsureOptimalFocus()
//   NameOfFocusedTextControl()
// into free functions: "non-friend non-members are better than members".

template<typename T>
T& MvcController::WindowFromXrcName(char const* name) const
{
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);

    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
            boost::is_same<wxWindow,T>::value
        ||  boost::is_base_and_derived<wxWindow,T>::value
        ));

    wxWindow* w = FindWindow(wxXmlResource::GetXRCID(name));
    if(!w)
        {
        fatal_error()
            << "Unable to find '"
            << name
            << "' in xml resources."
            << LMI_FLUSH
            ;
        }

    T* t = dynamic_cast<T*>(w);
    if(!t)
        {
        fatal_error()
            << "Unable to convert '"
            << name
            << "' to type '"
            << lmi::TypeInfo(typeid(T))
            << "'."
            << LMI_FLUSH
            ;
        }

    return *t;
}

template<typename T>
T& MvcController::WindowFromXrcName(std::string const& name) const
{
    return WindowFromXrcName<T>(name.c_str());
}

