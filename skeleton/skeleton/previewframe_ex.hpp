// Customize implementation details of library class wxPreviewFrame.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: previewframe_ex.hpp,v 1.1.1.1 2004-05-15 19:59:12 chicares Exp $

// This implementation is a derived work based on wxWindows code, viz.
//   include/wx/prntbase.h (C) 1997 Julian Smart and Markus Holzem
// both of which are covered by the wxWindows license.
//
// The originals were modified by GWC in 2004 as follows:
//   default arguments:
//     use application's top window as parent
//     use same size as application's top window
//     trivial changes to string arguments
// and in the later years given in the copyright notice above.

#ifndef previewframe_ex_hpp
#define previewframe_ex_hpp

#include "config.hpp"

#include <wx/app.h>
#include <wx/prntbase.h>

class PreviewFrameEx
    :public wxPreviewFrame
{
  public:
    PreviewFrameEx
        (wxPrintPreviewBase* preview
        ,wxFrame*            parent = dynamic_cast<wxFrame*>(wxTheApp->GetTopWindow())
        ,wxString     const& title  = "Print preview"
        ,wxPoint      const& pos    = wxDefaultPosition
        ,wxSize       const& size   = wxTheApp->GetTopWindow()->GetSize()
        ,long                style  = wxDEFAULT_FRAME_STYLE
        ,wxString     const& name   = "Loading print preview..."
        );
    ~PreviewFrameEx();
    void Initialize();

  private:
    void OnCloseWindow(wxCloseEvent& event);

    DECLARE_CLASS(PreviewFrameEx)
    DECLARE_EVENT_TABLE()
};

#endif // previewframe_ex_hpp

