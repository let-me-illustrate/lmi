// Customize implementation details of library class wxPreviewFrame.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: previewframe_ex.cpp,v 1.3 2006-03-09 01:58:18 chicares Exp $

// This implementation is a derived work based on wxWindows code, viz.
//   src/common/prntbase.cpp (C) 1998 Julian Smart and Markus Holzem
// both of which are covered by the wxWindows license.
//
// The originals were modified by GWC in 2004 as follows:
//   while preview is active, application hidden as well as disabled
// and in the later years given in the copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "previewframe_ex.hpp"

IMPLEMENT_CLASS(PreviewFrameEx, wxPreviewFrame)

BEGIN_EVENT_TABLE(PreviewFrameEx, wxPreviewFrame)
    EVT_CLOSE(PreviewFrameEx::UponCloseWindow)
END_EVENT_TABLE()

PreviewFrameEx::PreviewFrameEx
    (wxPrintPreviewBase* preview
    ,wxFrame*            parent
    ,wxString     const& title
    ,wxPoint      const& pos
    ,wxSize       const& size
    ,long                style
    ,wxString     const& name
    )
    :wxPreviewFrame(preview, parent, title, pos, size, style, name)
{
    // WX !! It would seem equivalent (and better) to default the
    // 'style' argument to
    //   wxDEFAULT_FRAME_STYLE | parent->IsMaximized() ? wxMAXIMIZE : 0
    // but that causes the preview frame of a maximized parent to
    // become a full-screen window, at least with wx-2.5.1 and msw.
    if(parent->IsMaximized())
        {
        Maximize();
        }
}

PreviewFrameEx::~PreviewFrameEx()
{
}

void PreviewFrameEx::Initialize()
{
    wxPreviewFrame::Initialize();
    wxTheApp->GetTopWindow()->Show(false);
}

/// TODO ?? WX NAME CONFLICT
/// This augments wxPreviewFrame::OnCloseWindow(), but isn't a
/// complete replacement. It calls that base-class function
/// explicitly; would Skip() work here?

void PreviewFrameEx::UponCloseWindow(wxCloseEvent& event)
{
    wxPreviewFrame::OnCloseWindow(event);
    wxTheApp->GetTopWindow()->Show(true);
}

