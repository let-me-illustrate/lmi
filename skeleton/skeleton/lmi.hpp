// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004 Gregory W. Chicares.
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

// $Id: lmi.hpp,v 1.1.1.1 2004-05-15 19:58:51 chicares Exp $

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.h (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#ifndef lmi_hpp
#define lmi_hpp

#include "config.hpp"

#include <wx/app.h>
#include <wx/defs.h> // WXDLLEXPORT

class DocManagerEx;
class ViewEx;

class WXDLLEXPORT wxConfigBase;
class WXDLLEXPORT wxDocMDIParentFrame;
class WXDLLEXPORT wxDocument;
class WXDLLEXPORT wxMDIChildFrame;

class Skeleton
    :public wxApp
{
  public:
    Skeleton();
    ~Skeleton();

    // Called by view classes when they are instantiated.
    wxMDIChildFrame* CreateChildFrame
        (wxDocument* doc
        ,ViewEx*     view
        );

  private:
    Skeleton(Skeleton const&);
    Skeleton& operator=(Skeleton const&);

    void InitDocManager();
    void InitIcon();
    void InitMenuBar();

    void OnAbout                  (wxCommandEvent&);
    void OnDropFiles              (wxDropFilesEvent&);
    bool OnExceptionInMainLoop    ();
    int  OnExit                   ();
    bool OnInit                   ();
    void OnMenuOpen               (wxMenuEvent&);
    void OnProperties             (wxCommandEvent&);
    void OnTestStandardException  (wxCommandEvent&);
    void OnUnhandledException     ();
    void OnUpdateUI               (wxUpdateUIEvent&);
    void OnWindowCascade          (wxCommandEvent&);
    void OnWindowNext             (wxCommandEvent&);
    void OnWindowPrevious         (wxCommandEvent&);
    void OnWindowTileHorizontally (wxCommandEvent&);
    void OnWindowTileVertically   (wxCommandEvent&);

    wxConfigBase* config_;
    DocManagerEx* doc_manager_;
    wxDocMDIParentFrame* frame_;

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(Skeleton)

#endif // lmi_hpp

