// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: main_wx.cpp,v 1.41 2006-01-15 12:45:07 chicares Exp $

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "main_wx.hpp"

#include "about_dialog.hpp"
#include "alert.hpp"
#include "armor.hpp"
#include "argv0.hpp"
#include "census_document.hpp"
#include "census_view.hpp"
#include "data_directory.hpp"
#include "docmanager_ex.hpp"
#include "docmdichildframe_ex.hpp"
#include "fenv_lmi.hpp"
#include "global_settings.hpp"
#include "getopt.hpp"
#include "ihs_dbdict.hpp" // print_databases()
#include "illustration_document.hpp"
#include "illustration_view.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "security.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "wx_new.hpp"

#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/image.h>
#include <wx/log.h> // wxSafeShowMessage()
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <stdexcept>
#include <string>

#if !defined __WXMSW__
#   include "lmi.xpm"
#endif // !defined __WXMSW__

IMPLEMENT_APP_NO_MAIN(Skeleton)
IMPLEMENT_WX_THEME_SUPPORT

// Where a builtin wxID_X identifier exists, use it as such, even if
// it's used as the 'name=' attribute of an entity in an '.xrc' file.
// For example, write 'wxID_SAVE' here, not 'XRCID("wxID_SAVE")'.
// The builtin doc-view framework uses the builtin identifiers only;
// using the XRCID here prevents the menu command from working, but
// either one makes toolbar enablement work correctly.
//
// WX !! However, menu enablement still doesn't seem to work with the
// EVT_UPDATE_UI(wxID_SAVE,...) handler here; that seems to require
// the EVT_MENU_OPEN handler.
//
BEGIN_EVENT_TABLE(Skeleton, wxApp)
 EVT_DROP_FILES(                             Skeleton::OnDropFiles             )
 EVT_MENU(wxID_ABOUT                        ,Skeleton::OnAbout                 )
 EVT_MENU(XRCID("window_cascade"           ),Skeleton::OnWindowCascade         )
 EVT_MENU(XRCID("window_next"              ),Skeleton::OnWindowNext            )
 EVT_MENU(XRCID("window_previous"          ),Skeleton::OnWindowPrevious        )
 EVT_MENU(XRCID("window_tile_horizontally" ),Skeleton::OnWindowTileHorizontally)
 EVT_MENU(XRCID("window_tile_vertically"   ),Skeleton::OnWindowTileVertically  )
 EVT_MENU_OPEN(                              Skeleton::OnMenuOpen              )
// TODO ?? expunge
// EVT_UPDATE_UI(wxID_ANY                     ,Skeleton::OnUpdateUI              )
 EVT_UPDATE_UI(wxID_SAVE                    ,Skeleton::OnUpdateFileSave        )
 EVT_UPDATE_UI(wxID_HELP                    ,Skeleton::OnUpdateHelp            )
// TODO ?? expunge
// Enabling this line prevents the menuitem from performing its required
// action, whether or not the EVT_UPDATE_UI(wxID_SAVE...) handler is also
// present.
// EVT_UPDATE_UI(XRCID("wxID_SAVE"           ),Skeleton::OnUpdateFileSave        )

// TODO ?? There has to be a better way.
/*
    EVT_UPDATE_UI(XRCID("edit_cell"        ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_class"       ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),Skeleton::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),Skeleton::OnUpdateInapplicable)
*/
END_EVENT_TABLE()

#ifdef __WXMSW__
// WX !! Oddly enough, wx seems to require this declaration, even
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
int main(int argc, char* argv[])
#else // __WXMSW__ defined.
int WINAPI WinMain
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     lpCmdLine
    ,int       nCmdShow
    )
#endif // __WXMSW__ defined.
{
    // WX !! and MPATROL !! Using wx-2.5.1 and mpatrol-1.4.8, both
    // dynamically linked to this application built with gcc-3.2.3,
    // three memory leaks are reported with:
    //   MPATROL_OPTIONS='SHOWUNFREED'
    // It's easier to trace them with:
    //   MPATROL_OPTIONS='LOGALL SHOWUNFREED USEDEBUG'
    // Two are apparently mpatrol artifacts traceable to symbols:
    //   "___mp_findsource"
    //   "___mp_init"
    // The third is traceable in 'mpatrol.log' with 'USEDEBUG' to
    //   Skeleton::GetEventHashTable() const
    // (although stepping through the code in gdb suggests it's really
    // WinMain(), and mpatrol or libbfd just got the symbol wrong)
    // and seems to be triggered the first time the program allocates
    // memory. The next line forces that to occur here; otherwise,
    // tracing this 'leak' becomes cumbersome and mysterious.
    std::string unused("Seems to trigger initialization of something.");

    int result = EXIT_FAILURE;

    try
        {
        initialize_application();
        initialize_filesystem();
#ifndef __WXMSW__
        result = wxEntry(argc, argv);
#else // __WXMSW__ defined.
        result = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#endif // __WXMSW__ defined.
        }
    catch(...)
        {
        report_exception();
        }

    fenv_validate();

    return result;
}

// 'config_' can't be initialized in the initializer list, because
// wxConfigBase::Get() must be called after SetAppName() and
// SetVendorName(). Otherwise, the configuration object wouldn't
// reflect the vendor and application name; on the msw platform,
// for instance, that would prevent writing to a registry key based
// on the application's name.
//
// The application name contains 'wx' because it may someday become
// desirable to maintain different configuration information in a
// similar manner for other lmi user interfaces.

Skeleton::Skeleton()
    :doc_manager_ (0)
    ,frame_       (0)
{
    SetAppName("lmi_wx");
    SetVendorName("lmi");
    config_ = wxConfigBase::Get();
}

Skeleton::~Skeleton()
{
}

wxMDIChildFrame* Skeleton::CreateChildFrame
    (wxDocument* doc
    ,ViewEx*     view
    )
{
// WX !! An exception thrown here does not get caught gracefully, e.g.
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

    // Style flag wxMAXIMIZE could have been used instead, but that
    // seems to work only with the msw platform.
    if(maximize_child)
        {
        child_frame->Maximize();
        }

    return child_frame;
}

void Skeleton::InitDocManager()
{
    // WX !! At least in wx-2.5.1, this can't be created in the
    // constructor, because that would try to create an instance of
    // class wxPageSetupDialogData, which apparently mustn't be done
    // before the application object is constructed.
    doc_manager_ = new DocManagerEx;
    doc_manager_->FileHistoryLoad(*config_);

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Census"
        ,"*.cns"
        ,""
        ,"cns"
        ,"Census Doc"
        ,"Census View"
        ,CLASSINFO(CensusDocument)
        ,CLASSINFO(CensusView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Illustration"
        ,"*.ill"
        ,""
        ,"ill"
        ,"Illustration Doc"
        ,"Illustration View"
        ,CLASSINFO(IllustrationDocument)
        ,CLASSINFO(IllustrationView)
        );

    if(!global_settings::instance().ash_nazg())
        {
        return;
        }

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
    // lossy shrinkage of a large icon could occur on msw; this
    // approach uses a smaller icon in the '.ico' file instead.
    //
    // WX !! However, this method:
//    frame_->SetIcons(wxIconBundle("mondrian.ico", wxBITMAP_TYPE_ICO));
    // displays a black-and-white icon in the alt-tab task switcher
    // on msw, even though the 256-color 32x32 icon comes first in the
    // '.ico' file; but this shows the color icon:
    frame_->SetIcons(wxICON(AAAAAAAA));
#else // Not defined __WXMSW__.
    frame_->SetIcon(wxICON(lmi));
#endif // Not defined __WXMSW__.
}

void Skeleton::InitMenuBar()
{
    wxMenuBar* menu_bar = wxXmlResource::Get()->LoadMenuBar("main_menu");
    if(!menu_bar)
        {
        fatal_error() << "Unable to create menubar." << LMI_FLUSH;
        }
    else
        {
        doc_manager_->AssociateFileHistoryWithFileMenu(menu_bar);
        }
    frame_->SetMenuBar(menu_bar);
}

// WX !! It seems odd that LoadMenuBar has two signatures, the simpler
// of which requires no 'parent' argument, while LoadToolBar does not.
//
void Skeleton::InitToolBar()
{
    wxToolBar* tool_bar = wxXmlResource::Get()->LoadToolBar(frame_, "toolbar");
    if(!tool_bar)
        {
        fatal_error() << "Unable to create toolbar." << LMI_FLUSH;
        }
    frame_->SetToolBar(tool_bar);
}

// WX !! Predefined macro wxID_ABOUT could be mapped by default to
// an OnAbout handler in the application or frame class.
//
void Skeleton::OnAbout(wxCommandEvent&)
{
    AboutDialog(frame_).ShowModal();
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
    try
        {
        // This just rethrows the exception. For msw at least, it
        // seems crucial that the exception be thrown from the same
        // shared library that caught it. This works only with a
        // 'monolithic' wx shared library.
        return wxAppConsole::OnExceptionInMainLoop();
        }
    catch(...)
        {
        report_exception();
        }
    return true;
}

int Skeleton::OnExit()
{
    doc_manager_->FileHistorySave(*config_);
    delete doc_manager_;
    delete config_;
    return 0;
}

// WX !! An exception thrown anywhere in this function, even right
// before the 'return' statement at the end, either causes a crash
// (wx-2.5.1) or gets caught by OnUnhandledException() (which loses
// exception information) instead of by OnExceptionInMainLoop().
// Therefore, exceptions must be trapped explicitly.
//
bool Skeleton::OnInit()
{
    try
        {
        if(false == ProcessCommandLine(argc, argv))
            {
            return false;
            }

        validate_security(!global_settings::instance().ash_nazg());

        wxXmlResource::Get()->InitAllHandlers();

        if(!wxXmlResource::Get()->Load(AddDataDir("xml_notebook.xrc")))
            {
            fatal_error() << "Unable to load xml resources." << LMI_FLUSH;
            }

        if(!wxXmlResource::Get()->Load(AddDataDir("menus.xrc")))
            {
            fatal_error() << "Unable to load menubar." << LMI_FLUSH;
            }

        if(!wxXmlResource::Get()->Load(AddDataDir("toolbar.xrc")))
            {
            fatal_error() << "Unable to load toolbar." << LMI_FLUSH;
            }

        wxInitAllImageHandlers();
        InitDocManager();

        frame_ = new(wx) wxDocMDIParentFrame
            (doc_manager_
            ,0     // Parent: always null.
            ,-1    // Window ID.
            ,"lmi"
            ,wxDefaultPosition
            ,wxDefaultSize
            ,   wxDEFAULT_FRAME_STYLE
            |   wxFRAME_NO_WINDOW_MENU
            |   wxHSCROLL
            |   wxVSCROLL
            );

        InitIcon();
        InitMenuBar();
        InitToolBar();
        frame_->CreateStatusBar();
    // FSF !! Need to implement this in a generic way for GNU/linux.
#ifdef __WXMSW__
        frame_->DragAcceptFiles(true);
#endif // __WXMSW__ defined.
        frame_->Centre(wxBOTH);
        frame_->Maximize(true);

        if(RunSpecialInputFileIfPresent(doc_manager_))
            {
            return false;
            }

        frame_->Show(true);
        SetTopWindow(frame_);

        if
            (!
                (   global_settings::instance().ash_nazg()
                ||  global_settings::instance().custom_io_0()
                )
            )
            {
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_ABOUT);
            wxPostEvent(frame_, event);
            }
        }
    catch(...)
        {
        report_exception();
        // Orderly termination: see
        //   http://lists.gnu.org/archive/html/lmi/2005-12/msg00020.html
        // Returning 'true' here without creating a frame would leave
        // the application running as an apparent zombie.
        if(GetTopWindow())
            {
            GetTopWindow()->Close();
            }
        return false;
        }
    return true;
}

void Skeleton::OnMenuOpen(wxMenuEvent&)
{
    int child_frame_count = 0;
    wxWindowList wl = frame_->GetChildren();
    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
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

        // Needed for (xrc) menu enablement: a
        //   EVT_UPDATE_UI(XRCID("wxID_SAVE"),Skeleton::OnUpdateFileSave)
        // handler fails to update enablement for that menu item.
        // However, enablement of an item with the same ID on the
        // toolbar apparently requires the EVT_UPDATE_UI technique.
        wxDocument* doc = doc_manager_->GetCurrentDocument();
        wxMenuItem* file_save = child_frame->GetMenuBar()->FindItem
            (XRCID("wxID_SAVE")
            );
        if(file_save)
            {
            file_save->Enable(doc && doc->IsModified());
            }
/*
        wxMenuItem* file_save_as = child_frame->GetMenuBar()->FindItem
            (XRCID("wxID_SAVEAS")
            );
        if(file_save_as)
            {
            file_save_as->Enable(true);
            }
*/
        }
    // (else) Parent menu enablement could be handled here, but, for
    // now at least, none is required.
}

// WX !! The wx exception-handling code doesn't seem to permit
// graceful handling here.
//
void Skeleton::OnUnhandledException()
{
    wxSafeShowMessage("Terminating due to unhandled exception.", "Fatal error");
}

// Required for toolbar enablement. Although '.xrc' files use the same
// xrc Id:
//   <object class="wxMenuItem" name="wxID_SAVE">
//   <object class="tool"       name="wxID_SAVE">
// the OnMenuOpen() handler above doesn't handle toolbar enablement,
// even when the menu is pulled down; and, OTOH, this function alone
// doesn't handle menuitem enablement.
//
void Skeleton::OnUpdateFileSave(wxUpdateUIEvent& event)
{
    wxDocument *doc = doc_manager_->GetCurrentDocument();
    event.Enable(doc && doc->IsModified());

    // Setting the event's Id to the xrc Id fails to handle menu
    // enablement--that is, this does not work:
//    event.SetId(XRCID("wxID_SAVE"));
//    event.Enable(doc && doc->IsModified());
}

void Skeleton::OnUpdateHelp(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? An unsuccessful experiment.
void Skeleton::OnUpdateInapplicable(wxUpdateUIEvent& e)
{
// This handler seems to override mdi childrens'.
//    e.Enable(0 != frame_->GetChildren().GetCount());

/*
This doesn't undo that override.
    e.Enable(false);
    if(0 != frame_->GetChildren().GetCount())
        {
        e.Skip();
        }
*/

// Presumably we need to use ProcessEvent(), somehow.
}

// WX !! It seems that a function like this should be able to handle
// all toolbar and menu enablement. But it appears that a much more
// complex implementation is required for wxID_SAVE.
//
void Skeleton::OnUpdateUI(wxUpdateUIEvent&)
{
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

// FSF !! Need this in the wx library for GNU/linux.
// WX !! A note in src/msw/mdi.cpp suggests adding an orientation
// argument to Tile(); until that's done, use this workaround.
//
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

bool Skeleton::ProcessCommandLine(int argc, char* argv[])
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
    static struct Option long_options[] =
      {
        {"ash_nazg"  ,NO_ARG   ,0 ,001 ,0 ,"ash nazg durbatulûk"},
        {"ash_naz"   ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"help"      ,NO_ARG   ,0 ,'h' ,0 ,"display this help and exit"},
        {"mellon"    ,NO_ARG   ,0 ,002 ,0 ,"pedo mellon a minno"},
        {"mello"     ,NO_ARG   ,0 ,003 ,0 ,"fraud"},
        {"data_path" ,REQD_ARG ,0 ,'d' ,0 ,"path to data files"},
        {"test_path" ,REQD_ARG ,0 ,'t' ,0 ,"path to test files"},
        {"print_db"  ,NO_ARG   ,0 ,'p' ,0 ,"print product databases"},
        {0           ,NO_ARG   ,0 ,0   ,0 ,""}
      };

    bool show_help        = false;

    int option_index = 0;
    GetOpt getopt_long
        (argc
        ,argv
        ,""
        ,long_options
        ,&option_index
        ,true
        );
    getopt_long.opterr = false;
    int c;
    while(EOF != (c = getopt_long()))
        {
        switch(c)
            {
            case 001:
                {
                global_settings::instance().set_ash_nazg(true);
                }
                break;

            case 002:
                {
                global_settings::instance().set_mellon(true);
                }
                break;

            case 'd':
                {
                global_settings::instance().set_data_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case 'h':
                {
                show_help = true;
                }
                break;

            case 'p':
                {
                print_databases();
                }
                break;

            case 't':
                {
                global_settings::instance().set_regression_test_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case '?':
                {
                warning() << "Unrecognized option '";
                int offset = getopt_long.optind - 1;
                if(0 < offset)
                    {
                    warning() << getopt_long.nargv[offset];
                    }
                warning() << "'." << std::flush;
                }
                break;

            default:
                warning() << "Unrecognized option '" << c << "'." << std::flush;
            }
        }

    if(show_help)
        {
        getopt_long.usage(warning());
        warning() << std::flush;
        return false;
        }

    return true;
}

