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

// $Id: lmi.cpp,v 1.1.1.1 2004-05-15 19:58:51 chicares Exp $

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#include "pchfile.hpp"

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "lmi.hpp"

#include "docmanager_ex.hpp"
#include "docmdichildframe_ex.hpp"
#include "html_doc.hpp"
#include "html_view.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <cstdlib> // std::free()
#include <stdexcept>
#include <string>

IMPLEMENT_APP_NO_MAIN(Skeleton)
IMPLEMENT_WX_THEME_SUPPORT

BEGIN_EVENT_TABLE(Skeleton, wxApp)
 EVT_DROP_FILES(                            Skeleton::OnDropFiles             )
 EVT_MENU(wxID_ABOUT                       ,Skeleton::OnAbout                 )
 EVT_MENU(XRCID("properties"              ),Skeleton::OnProperties            )
 EVT_MENU(XRCID("test_standard_exception" ),Skeleton::OnTestStandardException )
 EVT_MENU(XRCID("window_cascade"          ),Skeleton::OnWindowCascade         )
 EVT_MENU(XRCID("window_next"             ),Skeleton::OnWindowNext            )
 EVT_MENU(XRCID("window_previous"         ),Skeleton::OnWindowPrevious        )
 EVT_MENU(XRCID("window_tile_horizontally"),Skeleton::OnWindowTileHorizontally)
 EVT_MENU(XRCID("window_tile_vertically"  ),Skeleton::OnWindowTileVertically  )
 EVT_MENU_OPEN(                             Skeleton::OnMenuOpen              )
 EVT_UPDATE_UI(wxID_ANY,                    Skeleton::OnUpdateUI              )
END_EVENT_TABLE()

#ifdef __WXMSW__
// WX!! Oddly enough, wx seems to require this declaration, even
// though <wx/app.h> has been included and that header in turn
// includes <wx/msw/app.h>. Apparently the prototype in the latter
// header differs in the types of the first two arguments.
extern int wxEntry
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     pCmdLine
    ,int       nCmdShow
    );
#endif // __WXMSW__ defined.

#ifndef __WXMSW__
int main(int argc, char **argv)
#else // __WXMSW__ defined.
int WINAPI WinMain
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     lpCmdLine
    ,int       nCmdShow
    )
#endif // __WXMSW__ defined.
{
    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(0);
    // WX!! and MPATROL!! Using wx-2.5.1 and mpatrol-1.4.8, both
    // dynamically linked to this application built with gcc-3.2.3,
    // three memory leaks are reported with:
    //   MPATROL_OPTIONS='SHOWUNFREED'
    // It's easier to trace them with:
    //   MPATROL_OPTIONS='LOGALL SHOWUNFREED USEDEBUG'
    // Two are apparently mpatrol artifacts traceable to
    //   ___mp_findsource
    //   ___mp_init
    // The third is traceable in 'mpatrol.log' with 'USEDEBUG' to
    //   Skeleton::GetEventHashTable() const
    // (although stepping through the code in gdb suggests it's really
    // WinMain(), and mpatrol or libbfd just got the symbol wrong)
    // and seems to be triggered the first time the program allocates
    // memory. The next line forces that to occur here; otherwise,
    // tracing this 'leak' becomes cumbersome and mysterious.
    std::string unused("Seems to trigger initialization of something.");
    int r = EXIT_SUCCESS;
    try
        {
#ifndef __WXMSW__
        r = wxEntry(argc, argv);
#else // __WXMSW__ defined.
        r = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#endif // __WXMSW__ defined.
        }
    catch(std::exception& e)
        {
        wxSafeShowMessage(e.what(), "Fatal error");
        return EXIT_FAILURE;
        }
    catch(...)
        {
        wxSafeShowMessage("Unknown error", "Fatal error");
        return EXIT_FAILURE;
        }
    return r;
}

Skeleton::Skeleton()
    :config_      (wxConfigBase::Get())
    ,doc_manager_ (0)
    ,frame_       (0)
{
}

Skeleton::~Skeleton()
{
}

wxMDIChildFrame* Skeleton::CreateChildFrame
    (wxDocument* doc
    ,ViewEx*     view
    )
{
// TODO ?? This is not exception safe: e.g., try this:
//    throw std::runtime_error("This does not get caught gracefully.");
    bool maximize_child =
            frame_->GetActiveChild()
        &&  frame_->GetActiveChild()->IsMaximized()
        ;
    DocMDIChildFrameEx* child_frame = new DocMDIChildFrameEx
        (doc
        ,view
        ,frame_
        );
    child_frame->SetIcon(view->Icon());
    child_frame->SetMenuBar(view->MenuBar());
    child_frame->SetMdiWindowMenu();

    // Alternatively, style flag wxMAXIMIZE could be used, but that
    // seems to be specific to the msw platform.
    if(maximize_child)
        {
        child_frame->Maximize();
        }

    return child_frame;
}

void Skeleton::InitDocManager()
{
    // WX!! At least in wx-2.5.1, this can't be created in the
    // constructor, because that would try to create an instance of
    // class wxPageSetupDialogData, which apparently mustn't be done
    // before the application object is constructed.
    doc_manager_ = new DocManagerEx;
    doc_manager_->FileHistoryLoad(*config_);

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Html"
        ,"*.html"
        ,""
        ,"html"
        ,"Html Doc"
        ,"Html View"
        ,CLASSINFO(HtmlDocument)
        ,CLASSINFO(HtmlView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Text"
        ,"*.txt"
        ,""
        ,"txt"
        ,"Text Doc"
        ,"Text View"
        ,CLASSINFO(TextEditDocument)
        ,CLASSINFO(TextEditView)
        );
}

void Skeleton::InitIcon()
{
#ifdef __WXMSW__
    // If a wxIcon rather than a wxIconBundle were used here, then
    // lossy shrinkage of a large icon would occur on msw; this
    // approach uses a smaller icon in the '.ico' file instead.
    frame_->SetIcons(wxIconBundle("mondrian.ico", wxBITMAP_TYPE_ICO));
#else // Not defined __WXMSW__.
    // WX!! Macro 'wxICON' would be more useful if it supplied file
    // extensions: '.ico' for msw, and '.xpm' for other platforms.
    frame_->SetIcon(wxICON("mondrian.xpm"));
#endif // Not defined __WXMSW__.
}

void Skeleton::InitMenuBar()
{
    wxMenuBar* menu_bar = wxXmlResource::Get()->LoadMenuBar("main_menu");
    if(!menu_bar)
        {
// WX!! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to create menubar.");
        wxLogError("Unable to create menubar.");
        }
    else
        {
        doc_manager_->AssociateFileHistoryWithFileMenu(menu_bar);
        }
    frame_->SetMenuBar(menu_bar);
}

// WX!! Predefined macro wxID_ABOUT could be mapped by default to
// an OnAbout handler in the application or frame class.
void Skeleton::OnAbout(wxCommandEvent&)
{
    wxMessageBox
        ("wxWindows demo for life insurance illustrations\n\n"
         "Copyright (C) 2002, 2003, 2004 Gregory W. Chicares"
        ,"About lmi..."
        );
}

void Skeleton::OnDropFiles(wxDropFilesEvent& event)
{
    wxString const* filenames = event.GetFiles();
    for(int j = 0; j < event.GetNumberOfFiles(); ++j)
        {
        doc_manager_->CreateDocument(filenames[j], wxDOC_SILENT);
        }
}

bool Skeleton::OnExceptionInMainLoop()
{
    wxLog::FlushActive();

    int z = wxMessageBox
        ("Try to resume?"
        ,"Unhandled exception"
        ,wxYES_NO | wxICON_QUESTION
        );
    return wxYES == z;
}

int Skeleton::OnExit()
{
    doc_manager_->FileHistorySave(*config_);
    delete doc_manager_;
    delete config_;
    return 0;
}

bool Skeleton::OnInit()
{
// WX!! An exception thrown anywhere in this function, even right
// before the 'return' statement at the end, gets caught by
//   OnUnhandledException()
// instead of
//   OnExceptionInMainLoop()
//
//    throw std::runtime_error("This does not get caught gracefully.");

    wxXmlResource::Get()->InitAllHandlers();
    wxXmlResource::Get()->Load("xml_notebook.xrc");
    wxXmlResource::Get()->Load("menus.xrc");
    wxXmlResource::Get()->Load("toolbar.xrc");

    wxInitAllImageHandlers();
    InitDocManager();

    frame_ = new(wx) wxDocMDIParentFrame
        (doc_manager_
        ,0     // Parent: always null.
        ,-1    // Window ID.
        ,"lmi wxwindows demo"
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxDEFAULT_FRAME_STYLE | wxFRAME_NO_WINDOW_MENU | wxHSCROLL | wxVSCROLL
        );

    InitIcon();
    InitMenuBar();
    frame_->CreateStatusBar();
    frame_->SetToolBar(wxXmlResource::Get()->LoadToolBar(frame_, "toolbar"));
    frame_->DragAcceptFiles(true);
    frame_->Centre(wxBOTH);
    frame_->Maximize(true);
    frame_->Show(true);
    SetTopWindow(frame_);
    return true;
}

void Skeleton::OnMenuOpen(wxMenuEvent& event)
{
    // WX!! The wx-2.5.1 documentation says wxWindow::GetChildren()
    // returns a wxList&, but it really returns a wxWindowList&.
    int child_frame_count = 0;
    for
        (wxWindowList::const_iterator i = frame_->GetChildren().begin()
        ;i != frame_->GetChildren().end()
        ;++i
        )
        {
        child_frame_count += !!dynamic_cast<wxMDIChildFrame*>(*i);
        }

    wxMDIChildFrame* child_frame = frame_->GetActiveChild();
    if(child_frame)
        {
        wxMenuItem* window_next = child_frame->GetMenuBar()->FindItem
            (XRCID("window_next")
            );
        if(window_next)
            {
            window_next->Enable(1 < child_frame_count);
            }
        wxMenuItem* window_previous = child_frame->GetMenuBar()->FindItem
            (XRCID("window_previous")
            );
        if(window_previous)
            {
            window_previous->Enable(1 < child_frame_count);
            }
        }
    // (else) Handle parent menu enablement here as necessary.
}

void Skeleton::OnProperties(wxCommandEvent&)
{
    XmlNotebook xml_notebook(frame_);
    xml_notebook.ShowModal();
}

void Skeleton::OnTestStandardException(wxCommandEvent&)
{
    throw std::runtime_error("See whether this gets caught.");
}

// WX!! The wx exception-handling code doesn't seem to permit graceful
// handling here.
void Skeleton::OnUnhandledException()
{
    wxSafeShowMessage("Terminating due to unhandled exception.", "Fatal error");
}

void Skeleton::OnUpdateUI(wxUpdateUIEvent& event)
{
    // Handle toolbar enablement here as necessary.
}

void Skeleton::OnWindowCascade(wxCommandEvent&)
{
    frame_->Cascade();
}

void Skeleton::OnWindowNext(wxCommandEvent&)
{
    frame_->ActivateNext();
}

void Skeleton::OnWindowPrevious(wxCommandEvent&)
{
    frame_->ActivatePrevious();
}

void Skeleton::OnWindowTileHorizontally(wxCommandEvent&)
{
    frame_->Tile();
}

// WX!! A note in src/msw/mdi.cpp suggests adding an orientation
// argument to Tile(); until that's done, use this workaround.
void Skeleton::OnWindowTileVertically(wxCommandEvent&)
{
#ifdef __WXMSW__
    ::SendMessage
        ((HWND)frame_->GetClientWindow()->GetHWND()
        ,WM_MDITILE
        ,MDITILE_VERTICAL | MDITILE_SKIPDISABLED
        ,0
        );
#endif // __WXMSW__
}

