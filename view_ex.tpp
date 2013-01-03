// Enhanced wxView class: function templates.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "rtti_lmi.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

#include <wx/docview.h>

#include <string>
#include <typeinfo>

// WX !! Exceptions thrown here seem to be regenerated ad infinitum.
// Perhaps the wx doc-view framework retries the failing operation
// repeatedly when it ought to give up.

/// ViewName() returns the type_info name of ViewType. Although its
/// implementation is simple enough to write inline, it's abstracted
/// here because the correct implementation is not obvious.
///
/// wxView::GetViewName() can't be used because it requires an object,
/// whereas this function template is usable for reporting failure to
/// create any such object. Attempting to default-construct such an
/// object here could cause unbounded recursion, probably resulting
/// in a stack fault.
///
/// As long as all function templates in the present file call
/// ViewName(), the type assertion here need be written only once.

template<typename ViewType>
std::string ViewName()
{
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT((boost::is_base_and_derived<wxView,ViewType>::value));
    return lmi::TypeInfo(typeid(ViewType)).Name();
}

/// PredominantView() returns a reference to the natural view that
/// should be created by default for a document, which should be
/// registered in a wxDocTemplate. If the document has more than
/// one view of the appropriate type, then the first such view found
/// is chosen. An exception is thrown if no such view is found.

template<typename ViewType>
ViewType& PredominantView(wxDocument const& document)
{
    ViewType* view = 0;
    wxList const& views = document.GetViews();
    for(wxList::const_iterator i = views.begin(); i != views.end(); ++i)
        {
        wxObject* p = *i;
        LMI_ASSERT(0 != p);
        if(p->IsKindOf(CLASSINFO(ViewType)))
            {
            view = dynamic_cast<ViewType*>(p);
            break;
            }
        }
    if(!view)
        {
        fatal_error()
            << ViewName<ViewType>()
            << ": view not found."
            << LMI_FLUSH
            ;
        }
    return *view;
}

/// PredominantViewWindow() returns a reference to PredominantView()'s
/// main view window. An exception is thrown if that window doesn't
/// exist.

template<typename ViewType, typename ViewWindowType>
ViewWindowType& PredominantViewWindow
    (wxDocument const&           document
    ,ViewWindowType* ViewType::* view_window_member
    )
{
    ViewType const& view = PredominantView<ViewType>(document);
    ViewWindowType* view_window_pointer = view.*view_window_member;
    if(!view_window_pointer)
        {
        fatal_error()
            << ViewName<ViewType>()
            << ": window not found."
            << LMI_FLUSH
            ;
        }
    return *view_window_pointer;
}

