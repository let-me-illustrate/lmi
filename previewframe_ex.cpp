// Customize implementation details of library class wxPreviewFrame.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// This implementation is a derived work based on wxWindows code, viz.
//   src/common/prntbase.cpp (C) 1998 Julian Smart and Markus Holzem
// both of which are covered by the wxWindows license.
//
// The originals were modified by GWC in 2004 as follows:
//   while preview is active, application hidden as well as disabled
// and in the later years given in the copyright notice above.

#include "pchfile_wx.hpp"

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
    ,long int            style
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

void PreviewFrameEx::Initialize()
{
    wxPreviewFrame::Initialize();
    TopWindow().Show(false);
}

void PreviewFrameEx::UponCloseWindow(wxCloseEvent& event)
{
    TopWindow().Show(true);
    event.Skip();
}
