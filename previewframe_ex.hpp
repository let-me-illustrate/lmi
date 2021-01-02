// Customize implementation details of library class wxPreviewFrame.
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

#include "wx_utility.hpp"

#include <wx/prntbase.h>

class PreviewFrameEx final
    :public wxPreviewFrame
{
  public:
    PreviewFrameEx
        (wxPrintPreviewBase* preview
        ,wxFrame*            parent = dynamic_cast<wxFrame*>(&TopWindow())
        ,wxString     const& title  = "Print preview"
        ,wxPoint      const& pos    = wxDefaultPosition
        ,wxSize       const& size   = TopWindow().GetSize()
        ,long int            style  = wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE
        ,wxString     const& name   = "Loading print preview..."
        );
    ~PreviewFrameEx() override = default;

    // wxPreviewFrame overrides.
    void Initialize() override;

  private:
    // WX !! Should OnCloseWindow() be virtual in the base class?
    void UponCloseWindow(wxCloseEvent& event);

    DECLARE_CLASS(PreviewFrameEx)
    DECLARE_EVENT_TABLE()
};

#endif // previewframe_ex_hpp
