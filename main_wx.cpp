// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: main_wx.cpp,v 1.98 2008-02-26 01:27:41 chicares Exp $

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
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "census_document.hpp"
#include "census_view.hpp"
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "database_document.hpp"
#include "database_view.hpp"
#include "default_view.hpp"
#include "docmanager_ex.hpp"
#include "docmdichildframe_ex.hpp"
#include "fenv_guard.hpp"
#include "fenv_lmi.hpp"
#include "global_settings.hpp"
#include "getopt.hpp"
#include "handle_exceptions.hpp"
#include "ihs_dbdict.hpp"           // print_databases()
#include "illustration_document.hpp"
#include "illustration_view.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "miscellany.hpp"
#include "msw_workarounds.hpp"
#include "mvc_controller.hpp"
#include "path_utility.hpp"
#include "policy_document.hpp"
#include "policy_view.hpp"
#include "preferences_model.hpp"
#include "preferences_view.hpp"
#include "progress_meter.hpp"
#include "rounding_document.hpp"
#include "rounding_view.hpp"
#include "rounding_view_editor.hpp" // RoundingButtonsXmlHandler
#include "system_command.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "tier_document.hpp"
#include "tier_view.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"           // class ClipboardEx

#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/image.h>
#include <wx/log.h>                 // wxSafeShowMessage()
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>             // wxGetTextFromUser()
#include <wx/toolbar.h>
#include <wx/utils.h>               // wxMilliSleep(), wxSafeYield()
#include <wx/xrc/xmlres.h>

#include <iterator>
#include <stdexcept>
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
    EVT_DROP_FILES(                                    Skeleton::UponDropFiles                    )
    EVT_MENU(wxID_ABOUT                               ,Skeleton::UponAbout                        )
    EVT_MENU(XRCID("edit_default_cell"               ),Skeleton::UponEditDefaultCell              )
    EVT_MENU(XRCID("preferences"                     ),Skeleton::UponPreferences                  )
    EVT_MENU(XRCID("test_app_status_alert"           ),Skeleton::UponTestAppStatus                )
    EVT_MENU(XRCID("test_app_warning_alert"          ),Skeleton::UponTestAppWarning               )
    EVT_MENU(XRCID("test_app_hobsons_choice_alert"   ),Skeleton::UponTestAppHobsons               )
    EVT_MENU(XRCID("test_app_fatal_error_alert"      ),Skeleton::UponTestAppFatal                 )
    EVT_MENU(XRCID("test_app_standard_exception"     ),Skeleton::UponTestAppStandardException     )
    EVT_MENU(XRCID("test_app_arbitrary_exception"    ),Skeleton::UponTestAppArbitraryException    )
    EVT_MENU(XRCID("test_lib_status_alert"           ),Skeleton::UponTestLibStatus                )
    EVT_MENU(XRCID("test_lib_warning_alert"          ),Skeleton::UponTestLibWarning               )
    EVT_MENU(XRCID("test_lib_hobsons_choice_alert"   ),Skeleton::UponTestLibHobsons               )
    EVT_MENU(XRCID("test_lib_fatal_error_alert"      ),Skeleton::UponTestLibFatal                 )
    EVT_MENU(XRCID("test_lib_standard_exception"     ),Skeleton::UponTestLibStandardException     )
    EVT_MENU(XRCID("test_lib_arbitrary_exception"    ),Skeleton::UponTestLibArbitraryException    )
    EVT_MENU(XRCID("test_floating_point_environment" ),Skeleton::UponTestFloatingPointEnvironment )
    EVT_MENU(XRCID("test_pasting"                    ),Skeleton::UponTestPasting                  )
    EVT_MENU(XRCID("test_system_command"             ),Skeleton::UponTestSystemCommand            )
    EVT_MENU(XRCID("window_cascade"                  ),Skeleton::UponWindowCascade                )
    EVT_MENU(XRCID("window_next"                     ),Skeleton::UponWindowNext                   )
    EVT_MENU(XRCID("window_previous"                 ),Skeleton::UponWindowPrevious               )
    EVT_MENU(XRCID("window_tile_horizontally"        ),Skeleton::UponWindowTileHorizontally       )
    EVT_MENU(XRCID("window_tile_vertically"          ),Skeleton::UponWindowTileVertically         )
    EVT_MENU_OPEN(                                     Skeleton::UponMenuOpen                     )
    EVT_TIMER(wxID_ANY                                ,Skeleton::UponTimer                        )
// TODO ?? expunge
//  EVT_UPDATE_UI(wxID_ANY                            ,Skeleton::UponUpdateUI                     )
    EVT_UPDATE_UI(wxID_SAVE                           ,Skeleton::UponUpdateFileSave               )
    EVT_UPDATE_UI(wxID_HELP                           ,Skeleton::UponUpdateHelp                   )
// TODO ?? expunge
// Enabling this line prevents the menuitem from performing its required
// action, whether or not the EVT_UPDATE_UI(wxID_SAVE...) handler is also
// present.
//  EVT_UPDATE_UI(XRCID("wxID_SAVE"                  ),Skeleton::UponUpdateFileSave               )

// TODO ?? There has to be a better way.
/*
    EVT_UPDATE_UI(XRCID("edit_cell"            ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_class"           ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"           ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"          ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),Skeleton::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),Skeleton::UponUpdateInapplicable)
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
    // [space follows leading underscores in reserved names]
    //   "_ _ _ mp_findsource"
    //   "_ _ _ mp_init"
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
    child_frame->SetMenuBar(AdjustMenus(view->MenuBar()));

    // Style flag wxMAXIMIZE could have been used instead, but that
    // seems to work only with the msw platform.
    if(maximize_child)
        {
        child_frame->Maximize();
        }

    return child_frame;
}

/// Adjust menus read from wxxrc resources.
///
/// Whatever can be done in wxxrc generally should be done there. Use
/// this function to add finishing touches. Prefer to call it before
/// wxFrame::SetMenuBar(), to avoid flicker.
///
/// The "Test" menu should not be exposed to end users. All of lmi's
/// wxxrc resources include it; this function removes it whenever it's
/// not wanted. Alternatively, it could be coded as a separate wxxrc
/// resource and conditionally inserted here, but that would be less
/// flexible: e.g., menu order couldn't be controlled completely in
/// the wxxrc file.

wxMenuBar* Skeleton::AdjustMenus(wxMenuBar* argument)
{
    LMI_ASSERT(argument);
    wxMenuBar& menu_bar = *argument;

    if(!global_settings::instance().ash_nazg())
        {
        int test_menu_index = menu_bar.FindMenu("Test");
        if(wxNOT_FOUND == test_menu_index)
            {
            warning() << "No 'Test' menu found.";
            }
        else
            {
            delete menu_bar.Remove(test_menu_index);
            }
        }

    LMI_ASSERT(argument);
    return argument;
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
        ,"Census Document"
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
        ,"Illustration Document"
        ,"Illustration View"
        ,CLASSINFO(IllustrationDocument)
        ,CLASSINFO(IllustrationView)
        );

    if(!global_settings::instance().mellon())
        {
        return;
        }

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Database"
        ,"*.db4"
        ,""
        ,"db4"
        ,"Database Document"
        ,"Database View"
        ,CLASSINFO(DatabaseDocument)
        ,CLASSINFO(DatabaseView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Policy"
        ,"*.pol"
        ,""
        ,"pol"
        ,"Policy Document"
        ,"Policy View"
        ,CLASSINFO(PolicyDocument)
        ,CLASSINFO(PolicyView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Rounding"
        ,"*.rnd"
        ,""
        ,"rnd"
        ,"Rounding Document"
        ,"Rounding View"
        ,CLASSINFO(RoundingDocument)
        ,CLASSINFO(RoundingView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Tier"
        ,"*.tir"
        ,""
        ,"tir"
        ,"Tier Document"
        ,"Tier View"
        ,CLASSINFO(TierDocument)
        ,CLASSINFO(TierView)
        );

    if(!global_settings::instance().ash_nazg())
        {
        return;
        }

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Te&xt"
        ,"*.txt"
        ,""
        ,"txt"
        ,"Text Document"
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
    frame_->SetMenuBar(AdjustMenus(menu_bar));
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
        (configurable_settings::instance().default_input_filename()
        ,wxDOC_SILENT
        );
}

/// Rethrow an exception caught by wx into a local catch clause.
///
/// This virtual function exists only to be overridden. Calling the
/// base-class implementation would be pointless.
///
/// For MinGW gcc-3.4.4 and earlier
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/18594
///     [2006-01-10T22:00:24Z from Danny Smith]
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

        authenticate_system();

        wxXmlResource& xml_resources = *wxXmlResource::Get();

        xml_resources.InitAllHandlers();

        // TODO ?? Should not it be moved directly into rounding_view.hpp
        // or rounding_view_editor.hpp?
        xml_resources.AddHandler(new(wx) RoundingButtonsXmlHandler);

        DefaultView const v0;
        if(!xml_resources.Load(AddDataDir(v0.ResourceFileName())))
            {
            fatal_error() << "Unable to load xml resources." << LMI_FLUSH;
            }

        PreferencesView const v1;
        if(!xml_resources.Load(AddDataDir(v1.ResourceFileName())))
            {
            fatal_error() << "Unable to load xml resources." << LMI_FLUSH;
            }

        if(!xml_resources.Load(AddDataDir("menus.xrc")))
            {
            fatal_error() << "Unable to load menubar." << LMI_FLUSH;
            }

        if(!xml_resources.Load(AddDataDir("toolbar.xrc")))
            {
            fatal_error() << "Unable to load toolbar." << LMI_FLUSH;
            }

        if(!xml_resources.Load(AddDataDir(PolicyView::resource_file_name())))
            {
            fatal_error() << "Unable to load Policy resources." << LMI_FLUSH;
            }

        if(!xml_resources.Load(AddDataDir(RoundingView::resource_file_name())))
            {
            fatal_error() << "Unable to load Rounding resources." << LMI_FLUSH;
            }

        wxInitAllImageHandlers();
        InitDocManager();

        frame_ = new(wx) wxDocMDIParentFrame
            (doc_manager_
            ,NULL
            ,wxID_ANY
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

        if(custom_io_0_run_if_file_exists(doc_manager_))
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

    wxTextCtrl* t = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if(!t)
        {
        return;
        }

    std::string const s(ClipboardEx::GetText());
    if(s.empty())
        {
        return;
        }

    t->WriteText(redelimit_with_semicolons(s));
    event.Skip(false);
}

void Skeleton::UponPreferences(wxCommandEvent&)
{
    PreferencesModel preferences;
    PreferencesView const preferences_view;
    MvcController controller(frame_, preferences, preferences_view);
    int const rc = controller.ShowModal();
    if(wxID_OK == rc && preferences.IsModified())
        {
        preferences.Save();
        configurable_settings::instance().save();
        UpdateViews();
        }
}

void Skeleton::UponTestAppStatus(wxCommandEvent&)
{
    status()         << "Test status() ."         << LMI_FLUSH;
}

void Skeleton::UponTestAppWarning(wxCommandEvent&)
{
    warning()        << "Test warning() ."        << LMI_FLUSH;
}

void Skeleton::UponTestAppHobsons(wxCommandEvent&)
{
    hobsons_choice() << "Test hobsons_choice() ." << LMI_FLUSH;
}

void Skeleton::UponTestAppFatal(wxCommandEvent&)
{
    fatal_error()    << "Test fatal_error() ."    << LMI_FLUSH;
}

void Skeleton::UponTestAppStandardException(wxCommandEvent&)
{
    throw std::runtime_error("Test a standard exception.");
}

void Skeleton::UponTestAppArbitraryException(wxCommandEvent&)
{
    throw "Test an arbitrary exception.";
}

void Skeleton::UponTestLibStatus(wxCommandEvent&)
{
    test_status();
}

void Skeleton::UponTestLibWarning(wxCommandEvent&)
{
    test_warning();
}

void Skeleton::UponTestLibHobsons(wxCommandEvent&)
{
    test_hobsons_choice();
}

void Skeleton::UponTestLibFatal(wxCommandEvent&)
{
    test_fatal_error();
}

void Skeleton::UponTestLibStandardException(wxCommandEvent&)
{
    test_standard_exception();
}

void Skeleton::UponTestLibArbitraryException(wxCommandEvent&)
{
    test_arbitrary_exception();
}

void Skeleton::UponTestFloatingPointEnvironment(wxCommandEvent&)
{
    status() << "Begin test of floating-point environment." << std::flush;

    warning()
        << "Expect 'Resetting floating-point control word.' on statusbar."
        << std::flush
        ;
    x87_control_word(0x027f);
    wxMilliSleep(500); wxSafeYield();
    LMI_ASSERT(fenv_is_valid());

    warning() << "Expect statusbar to be cleared." << std::flush;
    status() << "" << std::flush;
    wxMilliSleep(500); wxSafeYield();

    warning()
        << "Expect a messagebox complaining about '0x007f',"
        << " and 'Resetting floating-point control word.' on statusbar."
        << std::flush
        ;
    x87_control_word(0x007f);
    wxMilliSleep(500); wxSafeYield();

    warning() << "Expect statusbar to be cleared." << std::flush;
    status() << "" << std::flush;
    wxMilliSleep(500); wxSafeYield();

    {
    fenv_guard fg;
    warning()
        << "Test '0x027f' as if in guarded calculations."
        << " Expect a messagebox complaining about that."
        << std::flush
        ;
    x87_control_word(0x027f);
    wxMilliSleep(500); wxSafeYield();
    LMI_ASSERT(!fenv_is_valid());
    }
    LMI_ASSERT(fenv_is_valid());

    {
    fenv_guard fg;
    warning()
        << "Test '0x007f' as if in guarded calculations."
        << " Expect a messagebox complaining about that."
        << std::flush
        ;
    x87_control_word(0x007f);
    wxMilliSleep(500); wxSafeYield();
    LMI_ASSERT(!fenv_is_valid());
    }
    LMI_ASSERT(fenv_is_valid());

    status() << "End test of floating-point environment." << std::flush;
}

namespace
{
/// Send a paste message to a window.
///
/// Design rationale--see:
///   http://lists.nongnu.org/archive/html/lmi/2008-02/msg00005.html
///
/// SOMEDAY !! Seek a gtk equivalent for msw-specific code.

void send_paste_message_to(wxWindow const& w)
{
#if defined __WXGTK__
    warning() << "Not yet implemented." << LMI_FLUSH;
#elif defined __WXMSW__
    ::SendMessage(reinterpret_cast<HWND>(w.GetHandle()), WM_PASTE, 0, 0);
#else  // Unsupported platform.
#   error Platform not yet supported. Consider contributing support.
#endif // Unsupported platform.
}
} // Unnamed namespace.

/// Test custom handler UponPaste().
///
/// See:
///   http://savannah.nongnu.org/task/?5224

void Skeleton::UponTestPasting(wxCommandEvent&)
{
    wxTextCtrl* t = new wxTextCtrl(frame_, wxID_ANY, "Testing...");
    LMI_ASSERT(t);

    ClipboardEx::SetText("1\r\n2\r\n3\r\n");
    t->SetSelection(-1L, -1L);
    send_paste_message_to(*t);
    if("1;2;3" != t->GetValue())
        {
        warning() << "'1;2;3' != '" << t->GetValue() << "'" << LMI_FLUSH;
        }

    ClipboardEx::SetText("X\tY\tZ\t");
    t->SetSelection(-1L, -1L);
    send_paste_message_to(*t);
    if("X;Y;Z" != t->GetValue())
        {
        warning() << "'X;Y;Z' != '" << t->GetValue() << "'" << LMI_FLUSH;
        }

    t->Destroy();
    status() << "Pasting test finished." << std::flush;
}

/// SOMEDAY !! Cancelling the wxGetTextFromUser() dialog causes it to
/// return an empty string. It might be nicer to use a more elaborate
/// facility that exits immediately in that case, because wxExecute()
/// asserts that its first argument is nonempty in wx's debug mode.
/// However, in the present implementation, it would be wrong to exit
/// immediately: that would mask any such side effects and make it
/// impossible to use this function to test the consequences of
/// attempting to execute an empty command.

void Skeleton::UponTestSystemCommand(wxCommandEvent&)
{
    wxString const w = wxGetTextFromUser
        ("Type a command."
        ,"Test system_command()"
        ,""
        ,wxTheApp->GetTopWindow()
        );
    std::string const z(w.c_str());
    system_command(z);
}

/// Periodically test the floating-point control word when no critical
/// calculation is being performed. If some rogue dll has changed it
/// to the undesirable but nonegregious value 0x027f, then reset it,
/// displaying a message on the statusbar; if it has been changed to
/// any other value, which could interfere with the orderly operation
/// of the program or even cause a crash, then reset it and pop up a
/// message box.
///
/// If an fenv_guard object exists, do nothing: in that case, some
/// critical calculation is being performed, so resetting the control
/// word would prevent the fenv_guard object from detecting a change
/// when any value but 0x037f is unacceptable.

void Skeleton::UponTimer(wxTimerEvent&)
{
    if(0 == fenv_guard::instance_count())
        {
        if(!fenv_is_valid())
            {
            status() << "Resetting floating-point control word. " << std::flush;
            }
        fenv_validate(e_fenv_indulge_0x027f);
        }
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
    wxDocument* doc = doc_manager_->GetCurrentDocument();
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
    frame_->Tile(wxHORIZONTAL);
}

void Skeleton::UponWindowTileVertically(wxCommandEvent&)
{
#if wxCHECK_VERSION(2,6,0)
    frame_->Tile(wxVERTICAL);
#else  // !wxCHECK_VERSION(2,6,0)
    warning()
        << "Vertical tiling not supported in this old wx version."
        << LMI_FLUSH
        ;
#endif // !wxCHECK_VERSION(2,6,0)
}

bool Skeleton::ProcessCommandLine(int argc, char* argv[])
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
    static struct Option long_options[] =
      {
        {"ash_nazg"  ,NO_ARG   ,0 ,001 ,0 ,"ash nazg durbatul�k"},
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
                {
                warning()
                    << "Unrecognized option character '"
                    << c
                    << "'."
                    << std::flush
                    ;
                }
            }
        }

    if((c = getopt_long.optind) < argc)
        {
        warning() << "Unrecognized parameters:\n";
        while(c < argc)
            {
            warning() << "  '" << argv[c++] << "'\n";
            }
        warning() << std::flush;
        }

    if(show_help)
        {
        getopt_long.usage(warning());
        warning() << std::flush;
        return false;
        }

    return true;
}

// TODO ?? CALCULATION_SUMMARY It would probably be in much better
// taste to use wxView::OnUpdate() for this purpose.

// TODO ?? CALCULATION_SUMMARY The progress meter's count is wrong.
// Consider writing a function to get a container of pointers to
// children of a given type.
//
// TODO ?? CALCULATION_SUMMARY Instead, why not just update the
// topmost window first, then update other windows, putting some
// progress indication on the statusbar?

void Skeleton::UpdateViews()
{
    wxWindowList wl = frame_->GetChildren();
    boost::shared_ptr<progress_meter> meter
        (create_progress_meter
            (wl.size()
            ,"Updating calculation summaries"
            )
        );
    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
        {
        wxDocMDIChildFrame const* c = dynamic_cast<wxDocMDIChildFrame*>(*i);
        if(c)
            {
            IllustrationView* v = dynamic_cast<IllustrationView*>(c->GetView());
            if(v)
                {
                v->DisplaySelectedValuesAsHtml();
                }
            }
        if(!meter->reflect_progress())
            {
            break;
            }
        }
    meter->culminate();
}

