// Customize implementation details of library class wxDocMDIChildFrame.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "docmdichildframe_ex.hpp"

#include <wx/menu.h>

#include <stdexcept>

IMPLEMENT_CLASS(DocMDIChildFrameEx, wxDocMDIChildFrame)

BEGIN_EVENT_TABLE(DocMDIChildFrameEx, wxDocMDIChildFrame)
END_EVENT_TABLE()

DocMDIChildFrameEx::DocMDIChildFrameEx
    (wxDocument*       doc
    ,wxView*           view
    ,wxMDIParentFrame* parent
    ,wxWindowID        id
    ,wxString   const& title
    ,wxPoint    const& pos
    ,wxSize     const& size
    ,long int          style
    ,wxString   const& name
    )
    :wxDocMDIChildFrame(doc, view, parent, id, title, pos, size, style, name)
{
}

#if !wxCHECK_VERSION(2,5,4)
    // The MDI "Window" menu did not display correctly for wx
    // versions prior to 2.5.4 . This error directive may be
    // suppressed if living with that problem is acceptable.
#   error Outdated library: wx-2.5.4 or greater is required.
#endif // !wxCHECK_VERSION(2,5,4)

