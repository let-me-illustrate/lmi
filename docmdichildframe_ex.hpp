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

// This will soon be removed because it no longer serves any purpose.

#ifndef docmdichildframe_ex_hpp
#define docmdichildframe_ex_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <wx/docmdi.h>

class DocMDIChildFrameEx
    :public  wxDocMDIChildFrame
    ,private lmi::uncopyable<DocMDIChildFrameEx>
{
  public:
    DocMDIChildFrameEx
        (wxDocument*       doc
        ,wxView*           view
        ,wxMDIParentFrame* parent
        ,wxWindowID        id     = wxID_ANY
        ,wxString   const& title  = "Loading..."
        ,wxPoint    const& pos    = wxDefaultPosition
        ,wxSize     const& size   = wxDefaultSize
        ,long int          style  = wxDEFAULT_FRAME_STYLE
        ,wxString   const& name   = "child frame"
        );

  private:
    DECLARE_CLASS(DocMDIChildFrameEx)
    DECLARE_EVENT_TABLE()
};

#endif // docmdichildframe_ex_hpp

