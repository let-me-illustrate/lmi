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

// $Id: main_wx.cpp,v 1.4 2005-03-30 19:28:31 chicares Exp $

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
#include "argv0.hpp"
//#include "alert.hpp" // TODO ?? expunge?
#include "census_document.hpp"
#include "census_view.hpp"
#include "data_directory.hpp"
#include "docmanager_ex.hpp"
#include "docmdichildframe_ex.hpp"
#include "fenv_lmi.hpp"
#include "global_settings.hpp"
#include "getopt.hpp"
#include "illustration_document.hpp"
#include "illustration_view.hpp"
#include "license.hpp"
#include "miscellany.hpp"
#include "secure_date.hpp"
#include "sigfpe.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "wx_new.hpp"

#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <csignal>
#include <cstdlib> // std::free()
#include <stdexcept>
#include <string>
#include <utility>

IMPLEMENT_APP_NO_MAIN(lmi_wx_app)
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
BEGIN_EVENT_TABLE(lmi_wx_app, wxApp)
 EVT_DROP_FILES(                             lmi_wx_app::OnDropFiles             )
 EVT_MENU(wxID_ABOUT                        ,lmi_wx_app::OnAbout                 )
// TODO ?? expunge
// EVT_MENU(XRCID("properties"               ),lmi_wx_app::OnProperties            )
 EVT_MENU(XRCID("test_standard_exception"  ),lmi_wx_app::OnTestStandardException )
 EVT_MENU(XRCID("window_cascade"           ),lmi_wx_app::OnWindowCascade         )
 EVT_MENU(XRCID("window_next"              ),lmi_wx_app::OnWindowNext            )
 EVT_MENU(XRCID("window_previous"          ),lmi_wx_app::OnWindowPrevious        )
 EVT_MENU(XRCID("window_tile_horizontally" ),lmi_wx_app::OnWindowTileHorizontally)
 EVT_MENU(XRCID("window_tile_vertically"   ),lmi_wx_app::OnWindowTileVertically  )
 EVT_MENU_OPEN(                              lmi_wx_app::OnMenuOpen              )
// TODO ?? expunge
// EVT_UPDATE_UI(wxID_ANY                     ,lmi_wx_app::OnUpdateUI              )
 EVT_UPDATE_UI(wxID_SAVE                    ,lmi_wx_app::OnUpdateFileSave        )
 EVT_UPDATE_UI(wxID_HELP                    ,lmi_wx_app::OnUpdateHelp            )
// TODO ?? expunge
// Enabling this line prevents the menuitem from performing its required
// action, whether or not the EVT_UPDATE_UI(wxID_SAVE...) handler is also
// present.
// EVT_UPDATE_UI(XRCID("wxID_SAVE"           ),lmi_wx_app::OnUpdateFileSave        )

// TODO ?? There has to be a better way.
/*
    EVT_UPDATE_UI(XRCID("edit_cell"        ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_class"       ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),lmi_wx_app::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),lmi_wx_app::OnUpdateInapplicable)
*/
END_EVENT_TABLE()

void process_command_line(int argc, char* argv[]);
bool security_validated(bool skip_validation);

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
{
#else // __WXMSW__ defined.
int WINAPI WinMain
    (HINSTANCE hInstance
    ,HINSTANCE hPrevInstance
    ,LPSTR     lpCmdLine
    ,int       nCmdShow
    )
{
    int argc = __argc;
    char** argv = __argv;
#endif // __WXMSW__ defined.

    // Set boost filesystem default name-check function to native. Its
    // facilities are used with names the user controls, and users
    // may specify names that are not portable. The default name-check
    // function is set here, before using this boost library in any
    // other way, to ensure uniform name checking.
    fs::path::default_name_check(fs::native);

    fs::path argv0(argv[0]);

    // This line forces mpatrol to link when it otherwise might not.
    // It has no other effect according to C99 7.20.3.2/2, second
    // sentence.
    std::free(0);
    // WX !! and MPATROL !! Using wx-2.5.1 and mpatrol-1.4.8, both
    // dynamically linked to this application built with gcc-3.2.3,
    // three memory leaks are reported with:
    //   MPATROL_OPTIONS='SHOWUNFREED'
    // It's easier to trace them with:
    //   MPATROL_OPTIONS='LOGALL SHOWUNFREED USEDEBUG'
    // Two are apparently mpatrol artifacts traceable to
    //   ___mp_findsource
    //   ___mp_init
    // The third is traceable in 'mpatrol.log' with 'USEDEBUG' to
    //   lmi_wx_app::GetEventHashTable() const
    // (although stepping through the code in gdb suggests it's really
    // WinMain(), and mpatrol or libbfd just got the symbol wrong)
    // and seems to be triggered the first time the program allocates
    // memory. The next line forces that to occur here; otherwise,
    // tracing this 'leak' becomes cumbersome and mysterious.
    std::string unused("Seems to trigger initialization of something.");

    // TODO ?? Instead, consider a singleton that checks the control
    // word upon destruction, or a boost::shared_ptr with a custom
    // dtor that does that.
    initialize_fpu();

    if(SIG_ERR == std::signal(SIGFPE, (void(*)(int))floating_point_error_handler))
        {
        wxSafeShowMessage
            ("Error"
            ,"Cannot install floating point error signal handler"
            );
        return EXIT_FAILURE;
        }

    process_command_line(argc, argv);
    // TODO ?? expunge?
//    DataDir::Get(global_settings::instance().data_directory.c_str());

    // The most privileged password bypasses security validation.
    if(!security_validated(global_settings::instance().ash_nazg))
        {
        return EXIT_FAILURE;
        }

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

// TODO ?? Can't use warning() here--it prevents wx's logging from
// working downstream. Consider an additional 'alert' stream that's
// always safe to use...and perhaps doesn't throw, so that it can
// appropriately be used in dtors.

// TODO ?? Old command-line arguments not yet supported here:
// [custom input]
// -datadir=path
// -testdir=path
// -tdt_too
// -print_databases

void process_command_line(int argc, char* argv[])
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
    static struct Option long_options[] =
      {
        {"accept",       NO_ARG,   0, 'a', 0, "accept license (-l to display)"},
        {"ash_nazg",     NO_ARG,   0, 001, 0, "ash nazg durbatulûk"},
        {"ash_naz",      NO_ARG,   0, 003, 0, "fraud"},
        {"help",         NO_ARG,   0, 'h', 0, "display this help and exit"},
        {"license",      NO_ARG,   0, 'l', 0, "display license and exit"},
        {"mellon",       NO_ARG,   0, 002, 0, "pedo mellon a minno"},
        {"mello",        NO_ARG,   0, 003, 0, "fraud"},
        {"data_path",    REQD_ARG, 0, 'd', 0, "path to data files"},
        {"test_path",    REQD_ARG, 0, 't', 0, "path to test files"},
        {0,              NO_ARG,   0,   0, 0, ""}
      };

    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;

    int c;
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
    while(EOF != (c = getopt_long()))
        {
        switch(c)
            {
            case 001:
                {
                global_settings::instance().ash_nazg = true;
                global_settings::instance().mellon = true;
                }
                break;

            case 002:
                {
                global_settings::instance().mellon = true;
                }
                break;

            case 'a':
                {
                license_accepted = true;
                }
                break;

            case 'd':
                {
                global_settings::instance().data_directory = getopt_long.optarg;
                }
                break;

            case 'h':
                {
                show_help = true;
                }
                break;

            case 'l':
                {
                show_license = true;
                }
                break;

            case 't':
                {
                global_settings::instance().regression_test_directory = getopt_long.optarg;
                }
                break;

// TODO ?? GUI not started yet--no output from warning().

            case '?':
                {
////                warning() << "Unrecognized option '";
                int offset = getopt_long.optind - 1;
                if(0 < offset)
                    {
////                    warning() << getopt_long.nargv[offset];
                    }
////                warning() << "'." << LMI_FLUSH;
                }
                break;

            default:
;            
////                warning() << "Unrecognized option '" << c << "'." << LMI_FLUSH;
            }
        }

    if(!license_accepted)
        {
////        warning() << license_notices() << LMI_FLUSH;
        }

    if(show_license)
        {
////        warning() << license_as_text() << LMI_FLUSH;
        }

    if(show_help)
        {
// TODO ?? Doesn't work--why not?
//::AllocConsole(); // msw workaround.
        getopt_long.usage();
        }
}

bool security_validated(bool skip_validation)
{
    if(skip_validation)
        {
        return true;
        }

    int rc = secure_date::instance()->validate
        (calendar_date()
        ,global_settings::instance().data_directory
        );

    if(0 == rc)
        {
        return true;
        }

    char const* error_messages[] =
        {"Logic error: too low."
        ,"Passkey has unexpected length. Try reinstalling."
        ,"Current date is outside permitted range. Contact the home office."
        ,"At least one file is missing, altered, or invalid. Try reinstalling."
        ,"Passkey is incorrect for this version. Contact the home office."
        ,"Logic error: too high."
        };
    rc = std::min
        (std::max(0, rc)
        ,static_cast<int>(lmi_array_size(error_messages))
        );

    // TODO ?? Brittle. Instead, make secure_date::validate return a string.
    std::string explanation(error_messages[1 + rc]);
    wxSafeShowMessage("Passkey: fatal error", explanation);
    return false;
}

lmi_wx_app::lmi_wx_app()
    :config_      (wxConfigBase::Get())
    ,doc_manager_ (0)
    ,frame_       (0)
{
}

lmi_wx_app::~lmi_wx_app()
{
}

wxMDIChildFrame* lmi_wx_app::CreateChildFrame
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

void lmi_wx_app::InitDocManager()
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

    if(!global_settings::instance().ash_nazg)
        {
        return;
        }

    // TODO ?? Consider eliminating this template and the associated
    // source files.
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

void lmi_wx_app::InitIcon()
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

void lmi_wx_app::InitMenuBar()
{
    wxMenuBar* menu_bar = wxXmlResource::Get()->LoadMenuBar("main_menu");
    if(!menu_bar)
        {
// WX !! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to create menubar.");
        wxLogError("Unable to create menubar.");
        }
    else
        {
        doc_manager_->AssociateFileHistoryWithFileMenu(menu_bar);
        }
    frame_->SetMenuBar(menu_bar);
}

// WX !! It seems odd that LoadMenuBar has two signatures, the simpler
// of which requires no 'parent' argument, while LoadToolBar does not.
void lmi_wx_app::InitToolBar()
{
    wxToolBar* tool_bar = wxXmlResource::Get()->LoadToolBar(frame_, "toolbar");
    if(!tool_bar)
        {
// WX !! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to create toolbar.");
        wxLogError("Unable to create toolbar.");
        }
    frame_->SetToolBar(tool_bar);
}

// WX !! Predefined macro wxID_ABOUT could be mapped by default to
// an OnAbout handler in the application or frame class.
void lmi_wx_app::OnAbout(wxCommandEvent&)
{
    AboutDialog(frame_).ShowModal();
// TODO ?? expunge    wxLog::FlushActive();
}

void lmi_wx_app::OnDropFiles(wxDropFilesEvent& event)
{
    wxString const* filenames = event.GetFiles();
    for(int j = 0; j < event.GetNumberOfFiles(); ++j)
        {
        doc_manager_->CreateDocument(filenames[j], wxDOC_SILENT);
        }
}

// TODO ?? Confirm that wx-2.5.4 solved the underlying problem, so
// that this now works as expected.
#if 1
bool lmi_wx_app::OnExceptionInMainLoop()
{
    wxLog::FlushActive();

    int z = wxMessageBox
        ("Try to resume?"
        ,"Unhandled exception"
        ,wxYES_NO | wxICON_QUESTION
        );
    return wxYES == z;
}
#else // not 1
// INELEGANT !! "Unhandled exception" occurs in msw system messages,
// so a distinct alternative here might be better.
//
// WX !! The dialog wx presents is confusing: "Abort" actually seems
// to be the choice that means 'try to resume', although probably
// that should be the meaning of "Fail".
//
// Experimental: suggested by Vadim Zeitlin 6/17/2004 6:01 AM.
//
bool lmi_wx_app::OnExceptionInMainLoop()
{
    try
        {
        // this just rethrows the exception but it is
        // better practice to call it instead of doing
        // it directly here in case the base class
        // behaviour changes later
        return wxApp::OnExceptionInMainLoop();
        }
    catch(std::exception& e)
        {
// TODO ?? Does wxSafeShowMessage() have any advantage once the main
// loop has begun? Using it here seems to duplicate messages pending
// in the wxLog facility.
//        wxSafeShowMessage("Error caught in OnExceptionInMainLoop().", e.what());
        wxLog::FlushActive();
        int z = wxMessageBox
            ("Try to resume?"
            ,"Unhandled exception"
            ,wxYES_NO | wxICON_QUESTION
            );
        return wxYES == z;
        }
    catch(...)
        {
        wxSafeShowMessage("Error caught in OnExceptionInMainLoop().", "Unknown error");
        wxLog::FlushActive();
        return false;
        }
}
#endif // not 1

int lmi_wx_app::OnExit()
{
    doc_manager_->FileHistorySave(*config_);
    delete doc_manager_;
    delete config_;
    return 0;
}

bool lmi_wx_app::OnInit()
{
// WX !! An exception thrown anywhere in this function, even right
// before the 'return' statement at the end, gets caught by
//   OnUnhandledException()
// instead of
//   OnExceptionInMainLoop()
//
//    throw std::runtime_error("This does not get caught gracefully.");

    wxXmlResource::Get()->InitAllHandlers();

    if(!wxXmlResource::Get()->Load("xml_notebook.xrc"))
        {
// WX !! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to load xml resources.");
        wxLogError("Unable to load xml resources.");
        }

    if(!wxXmlResource::Get()->Load("menus.xrc"))
        {
// WX !! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to load menubar.");
        wxLogError("Unable to load menubar.");
        }

    if(!wxXmlResource::Get()->Load("toolbar.xrc"))
        {
// WX !! When wx handles exceptions more gracefully, throw here:
//        throw std::runtime_error("Unable to load toolbar.");
        wxLogError("Unable to load toolbar.");
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
        ,wxDEFAULT_FRAME_STYLE | wxFRAME_NO_WINDOW_MENU | wxHSCROLL | wxVSCROLL
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
    frame_->Show(true);
    SetTopWindow(frame_);

    if(!global_settings::instance().ash_nazg)
        {
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_ABOUT);
        wxPostEvent(frame_, event);
        }

    return true;
}

void lmi_wx_app::OnMenuOpen(wxMenuEvent& event)
{
    // WX !! The wx-2.5.1 documentation says wxWindow::GetChildren()
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

        // Needed for (xrc) menu enablement: a
        //   EVT_UPDATE_UI(XRCID("wxID_SAVE"),lmi_wx_app::OnUpdateFileSave)
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

/* TODO ?? expunge
void lmi_wx_app::OnProperties(wxCommandEvent&)
{
    XmlNotebook xml_notebook(frame_);
    xml_notebook.ShowModal();
}
*/

void lmi_wx_app::OnTestStandardException(wxCommandEvent&)
{
    throw std::runtime_error("See whether this gets caught.");
}

// WX !! The wx exception-handling code doesn't seem to permit
// graceful handling here.
void lmi_wx_app::OnUnhandledException()
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
void lmi_wx_app::OnUpdateFileSave(wxUpdateUIEvent& event)
{
    wxDocument *doc = doc_manager_->GetCurrentDocument();
    event.Enable(doc && doc->IsModified());

    // Setting the event's Id to the xrc Id fails to handle menu
    // enablement--that is, this does not work:
//    event.SetId(XRCID("wxID_SAVE"));
//    event.Enable(doc && doc->IsModified());
}

void lmi_wx_app::OnUpdateHelp(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? An unsuccessful experiment.
void lmi_wx_app::OnUpdateInapplicable(wxUpdateUIEvent& e)
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
void lmi_wx_app::OnUpdateUI(wxUpdateUIEvent& event)
{
}

void lmi_wx_app::OnWindowCascade(wxCommandEvent&)
{
    frame_->Cascade();
}

void lmi_wx_app::OnWindowNext(wxCommandEvent&)
{
    frame_->ActivateNext();
}

void lmi_wx_app::OnWindowPrevious(wxCommandEvent&)
{
    frame_->ActivatePrevious();
}

void lmi_wx_app::OnWindowTileHorizontally(wxCommandEvent&)
{
    frame_->Tile();
}

// FSF !! Need this in the wx library for GNU/linux.
// WX !! A note in src/msw/mdi.cpp suggests adding an orientation
// argument to Tile(); until that's done, use this workaround.
void lmi_wx_app::OnWindowTileVertically(wxCommandEvent&)
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

