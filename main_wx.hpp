// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
#include "uncopyable_lmi.hpp"

#include <wx/app.h>
#include <wx/timer.h>

class DocManagerEx;
class ViewEx;

class WXDLLIMPEXP_FWD_CORE wxConfigBase;
class WXDLLIMPEXP_FWD_CORE wxDocMDIParentFrame;
class WXDLLIMPEXP_FWD_CORE wxDocument;
class WXDLLIMPEXP_FWD_CORE wxMDIChildFrame;
class WXDLLIMPEXP_FWD_CORE wxMenuBar;

class Skeleton
    :        public  wxApp
    ,        private lmi::uncopyable <Skeleton>
    ,virtual private obstruct_slicing<Skeleton>
{
  public:
    Skeleton();
    ~Skeleton();

    // Called by view classes when they are instantiated.
    wxMDIChildFrame* CreateChildFrame(wxDocument*, ViewEx*);

  private:
    wxMenuBar* AdjustMenus(wxMenuBar*);

    void InitDocManager ();
    void InitHelp       ();
    void InitIcon       ();
    void InitMenuBar    ();
    void InitToolBar    ();

    void UponAbout                        (wxCommandEvent&);
    void UponDropFiles                    (wxDropFilesEvent&);
    void UponEditDefaultCell              (wxCommandEvent&);
    void UponHelp                         (wxCommandEvent&);
    void UponMenuOpen                     (wxMenuEvent&);
    void UponPaste                        (wxClipboardTextEvent&);
    void UponPreferences                  (wxCommandEvent&);

    // Test alerts from application.
    void UponTestAppStatus                (wxCommandEvent&);
    void UponTestAppWarning               (wxCommandEvent&);
    void UponTestAppHobsons               (wxCommandEvent&);
    void UponTestAppFatal                 (wxCommandEvent&);
    void UponTestAppStandardException     (wxCommandEvent&);
    void UponTestAppArbitraryException    (wxCommandEvent&);

    // Test alerts from shared library.
    void UponTestLibStatus                (wxCommandEvent&);
    void UponTestLibWarning               (wxCommandEvent&);
    void UponTestLibHobsons               (wxCommandEvent&);
    void UponTestLibFatal                 (wxCommandEvent&);
    void UponTestLibStandardException     (wxCommandEvent&);
    void UponTestLibArbitraryException    (wxCommandEvent&);
    void UponTestLibCatastropheReport     (wxCommandEvent&);

    // Miscellaneous tests.
    void UponTestDateConversions          (wxCommandEvent&);
    void UponTestFloatingPointEnvironment (wxCommandEvent&);
    void UponTestPasting                  (wxCommandEvent&);
    void UponTestSystemCommand            (wxCommandEvent&);

    void UponTimer                        (wxTimerEvent&);
    void UponUpdateInapplicable           (wxUpdateUIEvent&);
    void UponWindowCascade                (wxCommandEvent&);
    void UponWindowNext                   (wxCommandEvent&);
    void UponWindowPrevious               (wxCommandEvent&);
    void UponWindowTileHorizontally       (wxCommandEvent&);
    void UponWindowTileVertically         (wxCommandEvent&);

    // wxApp overrides.
    virtual bool OnExceptionInMainLoop ();
    virtual int  OnExit                ();
    virtual bool OnInit                ();
    virtual void OnUnhandledException  ();

    bool ProcessCommandLine(int argc, char* argv[]);
    void UpdateViews();

    wxConfigBase*         config_;
    DocManagerEx*         doc_manager_;
    wxDocMDIParentFrame*  frame_;
    wxTimer               timer_;

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(Skeleton)

#endif // main_wx_hpp

