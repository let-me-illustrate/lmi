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
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: main_wx.cpp,v 1.56 2006-09-19 21:37:16 chicares Exp $

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
#include "argv0.hpp"
#include "census_document.hpp"
#include "census_view.hpp"
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "default_view.hpp"
#include "docmanager_ex.hpp"
#include "docmdichildframe_ex.hpp"
#include "fenv_lmi.hpp"
#include "global_settings.hpp"
#include "getopt.hpp"
#include "handle_exceptions.hpp"
#include "ihs_dbdict.hpp" // print_databases()
#include "illustration_document.hpp"
#include "illustration_view.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "miscellany.hpp"
#include "msw_workarounds.hpp"
#include "path_utility.hpp"
#include "security.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "wx_new.hpp"

#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/dataobj.h>
#include <wx/docmdi.h>
#include <wx/image.h>
#include <wx/log.h> // wxSafeShowMessage()
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <string>

#if !defined LMI_MSW
#   include "lmi.xpm"
#endif // !defined LMI_MSW

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
 EVT_DROP_FILES(                             Skeleton::UponDropFiles             )
 EVT_MENU(wxID_ABOUT                        ,Skeleton::UponAbout                 )
 EVT_MENU(XRCID("window_cascade"           ),Skeleton::UponWindowCascade         )
 EVT_MENU(XRCID("window_next"              ),Skeleton::UponWindowNext            )
 EVT_MENU(XRCID("window_previous"          ),Skeleton::UponWindowPrevious        )
 EVT_MENU(XRCID("window_tile_horizontally" ),Skeleton::UponWindowTileHorizontally)
 EVT_MENU(XRCID("window_tile_vertically"   ),Skeleton::UponWindowTileVertically  )
 EVT_MENU(XRCID("edit_default_cell"        ),Skeleton::UponEditDefaultCell       )
 EVT_MENU_OPEN(                              Skeleton::UponMenuOpen              )
 EVT_TIMER(wxID_ANY                         ,Skeleton::UponTimer                 )
// TODO ?? expunge
// EVT_UPDATE_UI(wxID_ANY                     ,Skeleton::UponUpdateUI              )
 EVT_UPDATE_UI(wxID_SAVE                    ,Skeleton::UponUpdateFileSave        )
 EVT_UPDATE_UI(wxID_HELP                    ,Skeleton::UponUpdateHelp            )
// TODO ?? expunge
// Enabling this line prevents the menuitem from performing its required
// action, whether or not the EVT_UPDATE_UI(wxID_SAVE...) handler is also
// present.
// EVT_UPDATE_UI(XRCID("wxID_SAVE"           ),Skeleton::UponUpdateFileSave        )

// TODO ?? There has to be a better way.
/*
    EVT_UPDATE_UI(XRCID("edit_cell"        ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_class"       ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),Skeleton::UponUpdateInapplicable)
*/
END_EVENT_TABLE()

#ifdef LMI_MSW
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
#endif // LMI_MSW defined.

#ifndef LMI_MSW
int main(int argc, char* argv[])
#else // LMI_MSW defined.
int WINAPI WinMain
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     lpCmdLine
    ,int       nCmdShow
    )
#endif // LMI_MSW defined.
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
#ifndef LMI_MSW
        result = wxEntry(argc, argv);
#else // LMI_MSW defined.
        MswDllPreloader::instance().PreloadDesignatedDlls();
        result = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#endif // LMI_MSW defined.
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
    ,timer_       (this)
{
    SetAppName("lmi_wx");
    SetVendorName("lmi");
    config_ = wxConfigBase::Get();
    timer_.Start(100);
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
#ifdef LMI_MSW
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
#else // Not defined LMI_MSW.
    frame_->SetIcon(wxICON(lmi));
#endif // Not defined LMI_MSW.
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

void Skeleton::UponAbout(wxCommandEvent&)
{
    AboutDialog(frame_).ShowModal();
}

void Skeleton::UponDropFiles(wxDropFilesEvent& event)
{
    wxString const* filenames = event.GetFiles();
    for(int j = 0; j < event.GetNumberOfFiles(); ++j)
        {
        doc_manager_->CreateDocument(filenames[j], wxDOC_SILENT);
        }
}

void Skeleton::UponEditDefaultCell(wxCommandEvent&)
{
    doc_manager_->CreateDocument
        (configurable_settings::instance().default_input_filename().c_str()
        ,wxDOC_SILENT
        );
}

/// Rethrow an exception caught by wx into a local catch clause.
///
/// This virtual function exists only to be overridden. Calling the
/// base-class implementation would be pointless.
///
/// For MinGW gcc-3.4.4 and earlier
///   http://sourceforge.net/mailarchive/message.php?msg_id=14436989
/// it is crucial that the exception be thrown from the same shared
/// library that caught it. This workaround requires a 'monolithic'
/// wx shared library.

bool Skeleton::OnExceptionInMainLoop()
{
    try
        {
#if !(defined __GNUC__ && LMI_GCC_VERSION < 30405)
        throw;
#else  // defined __GNUC__ && LMI_GCC_VERSION < 30405
#   if wxCHECK_VERSION(2,7,0)
        return wxAppBase::OnExceptionInMainLoop();
#   else  // !wxCHECK_VERSION(2,7,0)
        return wxAppConsole::OnExceptionInMainLoop();
#   endif // !wxCHECK_VERSION(2,7,0)
#endif // defined __GNUC__ && LMI_GCC_VERSION < 30405
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

        DefaultView const v;
        if(!wxXmlResource::Get()->Load(AddDataDir(v.ResourceFileName())))
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

        // Intercept 'Text Paste' events for all windows.
        Connect
            (wxID_ANY
            ,wxEVT_COMMAND_TEXT_PASTE
            ,wxClipboardTextEventHandler(Skeleton::UponPaste)
            );

        InitIcon();
        InitMenuBar();
        InitToolBar();
        frame_->CreateStatusBar();
    // FSF !! Need to implement this in a generic way for GNU/linux.
#ifdef LMI_MSW
        frame_->DragAcceptFiles(true);
#endif // LMI_MSW defined.
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

// TODO ?? Should this call Skip()?

void Skeleton::UponMenuOpen(wxMenuEvent&)
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
        //   EVT_UPDATE_UI(XRCID("wxID_SAVE"),Skeleton::UponUpdateFileSave)
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

namespace
{
    std::string redelimit_with_semicolons(std::string const& original_text)
        {
        std::string new_text;
        new_text.reserve(original_text.size());

        std::insert_iterator<std::string> j(new_text, new_text.begin());
        typedef std::string::const_iterator sci;
        std::string::const_iterator i = original_text.begin();
        for(sci i = original_text.begin(); i != original_text.end(); ++i)
            {
            switch(*i)
                {
                case '\n': {*j++ = ';';} break;
                case '\r': {           } break;
                case '\t': {*j++ = ' ';} break;
                default  : {*j++ =  *i;}
                }
            }

        new_text.resize(1 + new_text.find_last_not_of(';'));

        return new_text;
        }

} // Unnamed namespace.

/// Paste "\n"- or "\r\n"-delimited clipboard contents into a control,
/// replacing nonterminal delimiters with semicolons to form an input
/// sequence. The motivation is to permit pasting spreadsheet columns.
///
/// At least for now, this transformation is performed iff the paste
/// target is a wxTextCtrl.

void Skeleton::UponPaste(wxClipboardTextEvent& event)
{
    event.Skip();

    wxTextCtrl* target = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if(!target)
        {
        return;
        }

    wxDataFormat link_data_format("Link");
    if(!wxTheClipboard->IsSupported(link_data_format))
        {
        return;
        }

    wxClipboardLocker clipboard_locker;

    wxCustomDataObject data_object(link_data_format);
    wxTheClipboard->GetData(data_object);
    wxCharBuffer buffer(data_object.GetDataSize(link_data_format));
    if(!data_object.GetDataHere(link_data_format, buffer.data()))
        {
        return;
        }

    wxTextDataObject test_data;
    if(!wxTheClipboard->GetData(test_data))
        {
        return;
        }

    std::string const original_string(test_data.GetText());
    target->WriteText(redelimit_with_semicolons(original_string));

    event.Skip(false);
}

void Skeleton::UponTimer(wxTimerEvent&)
{
    fenv_validate();
}

// WX !! The wx exception-handling code doesn't seem to permit
// graceful handling here.
//
void Skeleton::OnUnhandledException()
{
    wxSafeShowMessage("Terminating due to unhandled exception.", "Fatal error");
}

// TODO ?? Should this call Skip()?

// Required for toolbar enablement. Although '.xrc' files use the same
// xrc Id:
//   <object class="wxMenuItem" name="wxID_SAVE">
//   <object class="tool"       name="wxID_SAVE">
// the UponMenuOpen() handler above doesn't handle toolbar enablement,
// even when the menu is pulled down; and, OTOH, this function alone
// doesn't handle menuitem enablement.
//
void Skeleton::UponUpdateFileSave(wxUpdateUIEvent& event)
{
    wxDocument *doc = doc_manager_->GetCurrentDocument();
    event.Enable(doc && doc->IsModified());

    // Setting the event's Id to the xrc Id fails to handle menu
    // enablement--that is, this does not work:
//    event.SetId(XRCID("wxID_SAVE"));
//    event.Enable(doc && doc->IsModified());
}

void Skeleton::UponUpdateHelp(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? An unsuccessful experiment.
void Skeleton::UponUpdateInapplicable(wxUpdateUIEvent& e)
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

// TODO ?? Should this be expunged?

// WX !! It seems that a function like this should be able to handle
// all toolbar and menu enablement. But it appears that a much more
// complex implementation is required for wxID_SAVE.
//
void Skeleton::UponUpdateUI(wxUpdateUIEvent&)
{
}

void Skeleton::UponWindowCascade(wxCommandEvent&)
{
    frame_->Cascade();
}

void Skeleton::UponWindowNext(wxCommandEvent&)
{
    frame_->ActivateNext();
}

void Skeleton::UponWindowPrevious(wxCommandEvent&)
{
    frame_->ActivatePrevious();
}

void Skeleton::UponWindowTileHorizontally(wxCommandEvent&)
{
    frame_->Tile();
}

// FSF !! Need this in the wx library for GNU/linux.
// WX !! A note in src/msw/mdi.cpp suggests adding an orientation
// argument to Tile(); until that's done, use this workaround.
//
void Skeleton::UponWindowTileVertically(wxCommandEvent&)
{
#ifdef LMI_MSW
    ::SendMessage
        ((HWND)frame_->GetClientWindow()->GetHWND()
        ,WM_MDITILE
        ,MDITILE_VERTICAL | MDITILE_SKIPDISABLED
        ,0
        );
#endif // LMI_MSW
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

