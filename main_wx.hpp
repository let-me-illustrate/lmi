// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: main_wx.hpp,v 1.1 2005-03-12 03:01:08 chicares Exp $

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.h (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#ifndef main_wx_hpp
#define main_wx_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <wx/app.h>
#include <wx/defs.h> // WXDLLEXPORT

class DocManagerEx;
class ViewEx;

class WXDLLEXPORT wxConfigBase;
class WXDLLEXPORT wxDocMDIParentFrame;
class WXDLLEXPORT wxDocument;
class WXDLLEXPORT wxMDIChildFrame;

class lmi_wx_app
    :public wxApp
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<lmi_wx_app>
{
  public:
    lmi_wx_app();
    ~lmi_wx_app();

    // Called by view classes when they are instantiated.
    wxMDIChildFrame* CreateChildFrame
        (wxDocument* doc
        ,ViewEx*     view
        );

  private:
    void InitDocManager();
    void InitIcon();
    void InitMenuBar();
    void InitToolBar();

    void OnAbout                  (wxCommandEvent&);
    void OnDropFiles              (wxDropFilesEvent&);
    void OnMenuOpen               (wxMenuEvent&);
    void OnProperties             (wxCommandEvent&);
    void OnTestStandardException  (wxCommandEvent&);
    void OnUpdateInapplicable     (wxUpdateUIEvent&);
    void OnUpdateFileSave         (wxUpdateUIEvent&);
    void OnUpdateUI               (wxUpdateUIEvent&);
    void OnUpdateHelp             (wxUpdateUIEvent&);
    void OnWindowCascade          (wxCommandEvent&);
    void OnWindowNext             (wxCommandEvent&);
    void OnWindowPrevious         (wxCommandEvent&);
    void OnWindowTileHorizontally (wxCommandEvent&);
    void OnWindowTileVertically   (wxCommandEvent&);

    // wxApp overrides.
    virtual void OnUnhandledException ();
    virtual bool OnExceptionInMainLoop();
    virtual int  OnExit               ();
    virtual bool OnInit               ();

    wxConfigBase* config_;
    DocManagerEx* doc_manager_;
    wxDocMDIParentFrame* frame_;

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(lmi_wx_app)

#endif // main_wx_hpp

