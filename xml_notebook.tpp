// Input 'notebook' (tabbed dialog) driven by xml resources.
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

// $Id: xml_notebook.tpp,v 1.3 2005-12-07 16:56:36 chicares Exp $

#include "xml_notebook.hpp"

#include "alert.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <wx/window.h>
#include <wx/xrc/xmlres.h>

template<typename T>
T& XmlNotebook::WindowFromXrcName(char const* name) const
{
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);

    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
            boost::is_same<wxWindow,T>::value
        ||  boost::is_base_and_derived<wxWindow,T>::value
        ));

    T* ptr = dynamic_cast<T*>(FindWindow(XRCID(name)));
    if(!ptr)
        {
        fatal_error() << "No control named '" << name << "'." << LMI_FLUSH;
        }
    return *ptr;
}

template<typename T>
T& XmlNotebook::WindowFromXrcName(std::string const& name) const
{
    return WindowFromXrcName<T>(name.c_str());
}

