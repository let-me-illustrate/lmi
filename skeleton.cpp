// Main file for life insurance illustrations with wx interface.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

// Portions of this file are derived from wxWindows files
//   samples/docvwmdi/docview.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and
//   samples/html/printing/printing.cpp
// which bears no copyright or license notice.
//
// GWC used that code as an application skeleton and printing
// implementation, modifying it in 2002 and the later years given in
// the copyright notice above.

#include "pchfile_wx.hpp"

#include "skeleton.hpp"

#include "about_dialog.hpp"
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "census_document.hpp"
#include "census_view.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "data_directory.hpp"
#include "database_document.hpp"
#include "database_view.hpp"
#include "dbdict.hpp"                   // print_databases()
#include "default_view.hpp"
#include "docmanager_ex.hpp"
#include "fenv_guard.hpp"
#include "fenv_lmi.hpp"
#include "getopt.hpp"
#include "global_settings.hpp"
#include "gpt_document.hpp"
#include "gpt_view.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "icon_monger.hpp"
#include "illustration_document.hpp"
#include "illustration_view.hpp"
#include "input_sequence_entry.hpp"     // InputSequenceEntryXmlHandler
#include "license.hpp"
#include "mec_document.hpp"
#include "mec_view.hpp"
#include "miscellany.hpp"
#include "msw_workarounds.hpp"          // PreloadDesignatedDlls()
#include "mvc_controller.hpp"
#include "path.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "policy_document.hpp"
#include "policy_view.hpp"
#include "preferences_model.hpp"
#include "preferences_view.hpp"
#include "rounding_document.hpp"
#include "rounding_view.hpp"
#include "rounding_view_editor.hpp"     // RoundingButtonsXmlHandler
#include "system_command.hpp"
#include "text_doc.hpp"
#include "text_view.hpp"
#include "tier_document.hpp"
#include "tier_view.hpp"
#include "verify_products.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"               // class ClipboardEx

#include <wx/artprov.h>
#include <wx/config.h>
#include <wx/cshelp.h>
#include <wx/docmdi.h>                  // class wxDocMDIChildFrame
#include <wx/image.h>
#include <wx/log.h>                     // wxSafeShowMessage()
#include <wx/math.h>                    // wxRound()
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/msgout.h>
#include <wx/persist/toplevel.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>                 // wxGetTextFromUser()
#include <wx/toolbar.h>
#include <wx/utils.h>                   // wxMilliSleep(), wxSafeYield()
#include <wx/xrc/xmlres.h>

#include <iterator>                     // insert_iterator
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
void load_xrc_file_from_data_directory
    (wxXmlResource& xml_resources
    ,char const* xrc_filename
    )
{
    if(!xml_resources.LoadFile(wxFileName(AddDataDir(xrc_filename))))
        {
        alarum() << "Unable to load xml resources." << LMI_FLUSH;
        }
}
} // Unnamed namespace.

// Where a builtin wxID_X identifier exists, use it as such, even if
// it's used as the 'name=' attribute of an entity in an '.xrc' file.
// For example, write 'wxID_SAVE' here, not 'XRCID("wxID_SAVE")'.
// The builtin doc-view framework uses the builtin identifiers only;
// using the XRCID here prevents the menu command from working, but
// either one makes toolbar enablement work correctly.
//
BEGIN_EVENT_TABLE(Skeleton, wxApp)
    EVT_DROP_FILES(                                    Skeleton::UponDropFiles                    )
    EVT_BUTTON(wxID_HELP                              ,Skeleton::UponHelp                         )
    EVT_MENU(  wxID_HELP                              ,Skeleton::UponHelp                         )
    EVT_MENU(wxID_ABOUT                               ,Skeleton::UponAbout                        )
    EVT_MENU(wxID_PREFERENCES                         ,Skeleton::UponPreferences                  )
    EVT_MENU(XRCID("edit_default_cell"               ),Skeleton::UponEditDefaultCell              )
    EVT_MENU(XRCID("test_app_status_alert"           ),Skeleton::UponTestAppStatus                )
    EVT_MENU(XRCID("test_app_warning_alert"          ),Skeleton::UponTestAppWarning               )
    EVT_MENU(XRCID("test_app_hobsons_choice_alert"   ),Skeleton::UponTestAppHobsons               )
    EVT_MENU(XRCID("test_app_alarum_alert"           ),Skeleton::UponTestAppFatal                 )
    EVT_MENU(XRCID("test_app_standard_exception"     ),Skeleton::UponTestAppStandardException     )
    EVT_MENU(XRCID("test_app_arbitrary_exception"    ),Skeleton::UponTestAppArbitraryException    )
    EVT_MENU(XRCID("test_lib_status_alert"           ),Skeleton::UponTestLibStatus                )
    EVT_MENU(XRCID("test_lib_warning_alert"          ),Skeleton::UponTestLibWarning               )
    EVT_MENU(XRCID("test_lib_hobsons_choice_alert"   ),Skeleton::UponTestLibHobsons               )
    EVT_MENU(XRCID("test_lib_alarum_alert"           ),Skeleton::UponTestLibFatal                 )
    EVT_MENU(XRCID("test_lib_standard_exception"     ),Skeleton::UponTestLibStandardException     )
    EVT_MENU(XRCID("test_lib_arbitrary_exception"    ),Skeleton::UponTestLibArbitraryException    )
    EVT_MENU(XRCID("test_lib_catastrophe_report"     ),Skeleton::UponTestLibCatastropheReport     )
    EVT_MENU(XRCID("test_date_conversions"           ),Skeleton::UponTestDateConversions          )
    EVT_MENU(XRCID("test_floating_point_environment" ),Skeleton::UponTestFloatingPointEnvironment )
    EVT_MENU(XRCID("test_pasting"                    ),Skeleton::UponTestPasting                  )
    EVT_MENU(XRCID("test_system_command"             ),Skeleton::UponTestSystemCommand            )
    EVT_MENU(XRCID("window_cascade"                  ),Skeleton::UponWindowCascade                )
    EVT_MENU(XRCID("window_next"                     ),Skeleton::UponWindowNext                   )
    EVT_MENU(XRCID("window_previous"                 ),Skeleton::UponWindowPrevious               )
    EVT_MENU(XRCID("window_tile_horizontally"        ),Skeleton::UponWindowTileHorizontally       )
    EVT_MENU(XRCID("window_tile_vertically"          ),Skeleton::UponWindowTileVertically         )
    EVT_UPDATE_UI(XRCID("print_pdf"                  ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("edit_cell"                  ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("edit_class"                 ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("edit_case"                  ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("run_cell"                   ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("run_class"                  ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("run_case"                   ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_cell"                 ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_class"                ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_case"                 ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_case_to_disk"         ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_spreadsheet"          ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_group_roster"         ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("print_group_quote"          ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("copy_census"                ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("paste_census"               ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("add_cell"                   ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("delete_cells"               ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("column_width_varying"       ),Skeleton::UponUpdateInapplicable           )
    EVT_UPDATE_UI(XRCID("column_width_fixed"         ),Skeleton::UponUpdateInapplicable           )
END_EVENT_TABLE()

/// 'config_' can't be initialized in the initializer list with
/// wxConfigBase::Get(), which must be called after SetAppName() and
/// SetVendorName(): otherwise, the configuration object wouldn't
/// reflect the vendor and application name; on the msw platform,
/// for instance, that would prevent writing to a registry key based
/// on the application's name.
///
/// The application name contains 'wx' because it may someday become
/// desirable to maintain different configuration information in a
/// similar manner for other lmi user interfaces.
///
/// The application display name, however, omits 'wx', because:
///  - it'll always be specific to wx; and
///  - 'lmi' is all lowercase, but wx capitalizes the first letter of
///     the "AppName" (but not the "AppDisplayName").

Skeleton::Skeleton()
    :config_          {nullptr}
    ,doc_manager_     {nullptr}
    ,frame_           {nullptr}
    ,timer_           {this}
{
    SetAppName("lmi_wx");

    SetAppDisplayName("lmi...");

    SetVendorName("lmi");
    config_ = wxConfigBase::Get();
    timer_.Start(100);
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
    wxDocMDIChildFrame* child_frame = new(wx) wxDocMDIChildFrame
        (doc
        ,view
        ,frame_
        ,wxID_ANY
        ,"Loading..."
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

/// Create the MDI document manager.
///
/// This uses '::new' rather than 'new(wx)' because the object is
/// explicitly deleted in OnExit().

DocManagerEx* Skeleton::CreateDocManager()
{
    return ::new DocManagerEx;
}

void Skeleton::InitDocManager()
{
    // WX !! At least in wx-2.5.1, the DocManagerEx instance must be
    // created here. It can't be instantiated in the constructor
    // because it creates a wxPageSetupDialogData instance, which
    // apparently mustn't be done before the application object has
    // been constructed.
    doc_manager_ = CreateDocManager();
    doc_manager_->FileHistoryLoad(*config_);

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Census"
        ,"*.cns"
        ,""
        ,"cns"
        ,"Census document"
        ,"Census view"
        ,CLASSINFO(CensusDocument)
        ,CLASSINFO(CensusView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Illustration"
        ,"*.ill"
        ,""
        ,"ill"
        ,"Illustration document"
        ,"Illustration view"
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
        ,"*.database"
        ,""
        ,"database"
        ,"Database document"
        ,"Database view"
        ,CLASSINFO(DatabaseDocument)
        ,CLASSINFO(DatabaseView)
        );

    // There is intentionally no GUI editor for '.lingo' files.
    // There's no GUI editor for '.funds' files either.

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Policy"
        ,"*.policy"
        ,""
        ,"policy"
        ,"Policy document"
        ,"Policy view"
        ,CLASSINFO(PolicyDocument)
        ,CLASSINFO(PolicyView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Rounding"
        ,"*.rounding"
        ,""
        ,"rounding"
        ,"Rounding document"
        ,"Rounding view"
        ,CLASSINFO(RoundingDocument)
        ,CLASSINFO(RoundingView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Strata"
        ,"*.strata"
        ,""
        ,"strata"
        ,"Tier document"
        ,"Tier view"
        ,CLASSINFO(TierDocument)
        ,CLASSINFO(TierView)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"MEC testing"
        ,"*.mec"
        ,""
        ,"mec"
        ,"MEC-testing document"
        ,"MEC-testing view"
        ,CLASSINFO(mec_document)
        ,CLASSINFO(mec_view)
        );

    new(wx) wxDocTemplate
        (doc_manager_
        ,"Guideline premium test"
        ,"*.gpt"
        ,""
        ,"gpt"
        ,"GPT document"
        ,"GPT view"
        ,CLASSINFO(gpt_document)
        ,CLASSINFO(gpt_view)
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
        ,"Text document"
        ,"Text view"
        ,CLASSINFO(TextEditDocument)
        ,CLASSINFO(TextEditView)
        );
}

/// Initialize help subsystem.
///
/// Contextual <help> elements in wxxrc files are made available by
/// wxSimpleHelpProvider. No fancier version of that class is needed.
///
/// An html user manual is displayed by wxLaunchDefaultBrowser(),
/// which requires no initialization here.

void Skeleton::InitHelp()
{
    wxHelpProvider::Set(new(wx) wxSimpleHelpProvider);
    LMI_ASSERT(wxHelpProvider::Get());
}

void Skeleton::InitIcon()
{
    frame_->SetIcons(wxIconBundle(AddDataDir("lmi.ico"), wxBITMAP_TYPE_ICO));
}

void Skeleton::InitMenuBar()
{
    wxMenuBar* menu_bar = wxXmlResource::Get()->LoadMenuBar("main_menu");
    if(!menu_bar)
        {
        alarum() << "Unable to create menubar." << LMI_FLUSH;
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
        alarum() << "Unable to create toolbar." << LMI_FLUSH;
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
    configurable_settings& z = configurable_settings::instance();
    fs::path const p(z.default_input_filename());

    if(p.empty() || !fs::exists(p) || fs::is_directory(p))
        {
        alarum()
            << "The default input file, '"
            << p
            << "', could not be read.\n\n"
            << "Use the \"Preferences\" dialog to select any saved"
            << " illustration-input file as the default."
            << LMI_FLUSH
            ;
        }

    doc_manager_->CreateDocument(p.string(), wxDOC_SILENT);
}

/// Display user manual in default browser.
///
/// If this changes the x86 floating-point control word, suppress the
/// resulting diagnostic unless it changed to a really bizarre value.
///
/// If wxLaunchDefaultBrowser() fails, then it normally displays an
/// error message of its own, which is suppressed here. See:
///   https://lists.nongnu.org/archive/html/lmi/2009-03/msg00039.html

void Skeleton::UponHelp(wxCommandEvent&)
{
    fenv_guard fg;

    std::string const canonical_url("https://lmi.nongnu.org/user_manual.html");

    std::string s(AddDataDir("user_manual.html"));
    fs::path p(fs::absolute(fs::path(s)));
    if(fs::exists(p))
        {
        s = "file://" + p.string();
        }
    else
        {
        warning()
            << "A local copy of the user manual should have been placed here:"
            << "\n    " << p
            << "\nbut was not. Try reinstalling."
            << '\n'
            << "\nMeanwhile, the online user manual will be used if possible."
            << std::flush
            ;
        s = canonical_url;
        }

    bool r = false;
    {
    wxLogNull x;
    r = wxLaunchDefaultBrowser(wxString::FromUTF8(s));
    }
    if(!r)
        {
        alarum()
            << "Unable to open"
            << "\n    " << s
            << "\nin default browser."
            ;
        if(canonical_url != s)
            {
            alarum()
                << '\n'
                << "\nThe user manual can be read online here:"
                << "\n    " << canonical_url
                ;
            }
        alarum() << std::flush;
        }

    fenv_validate(e_fenv_indulge_0x027f);
}

/// Handle wx assertion failure.
///
/// By default, wx displays a "Do you want to stop the program?"
/// messagebox, with MB_YESNOCANCEL buttons. The first choice traps to
/// a debugger; end users who pick it will be startled by the ensuing
/// behavior. This overriding implementation essentially forces the
/// choice "No", which lets the program attempt to continue.
///
/// Ignore the 'func' argument: it is superfluous.
///
/// Ignore the 'msg' argument iff it represents an empty string.

void Skeleton::OnAssertFailure
    (wxChar const* file
    ,int           line
    ,wxChar const* // func
    ,wxChar const* cond
    ,wxChar const* msg
    )
{
    wxString m(msg);
    if(!m.IsEmpty()) {m = "\n(" + m + ")";}
    std::ostringstream oss;
    oss
        << "Assertion '" << wxString(cond) << "' failed"
        <<                           m     << "."
        << "\n[file "    << wxString(file)
        << ", line "     <<          line  << "]\n"
        ;
    safely_show_message(oss.str().c_str());
}

/// Rethrow an exception caught by wx into a local catch clause.
///
/// Report the exception, then return 'true' to continue processing.
///
/// This virtual function exists only to be overridden. Calling the
/// base-class implementation would normally be pointless. However,
/// for MinGW gcc-3.4.4 and earlier
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/18594
///     [2006-01-10T22:00:24Z from Danny Smith]
/// it is crucial that the exception be rethrown from the same shared
/// library that caught it. That need is met by calling a base class's
/// OnExceptionInMainLoop() explicitly; since wx-2.7 at least, that
/// function is provided by class wxAppBase, but some earlier versions
/// provide it only in class wxAppConsole, and the latter requires a
/// 'monolithic' wx shared library.

bool Skeleton::OnExceptionInMainLoop()
{
    try
        {
#if !(defined __GNUC__ && LMI_GCC_VERSION < 30405)
        throw;
#else  // defined __GNUC__ && LMI_GCC_VERSION < 30405
        return wxAppBase::OnExceptionInMainLoop();
#endif // defined __GNUC__ && LMI_GCC_VERSION < 30405
        }
    catch(...)
        {
        report_exception();
        }
    return true;
}

/// wxApp::OnExit() override.
///
/// Call the base class's implementation--see:
///   https://lists.nongnu.org/archive/html/lmi/2013-11/msg00020.html

int Skeleton::OnExit()
{
    try
        {
        doc_manager_->FileHistorySave(*config_);
        delete doc_manager_;
        }
    catch(...)
        {
        report_exception();
        }
    return wxApp::OnExit(); // Deletes config_.
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
#if defined __WXMSW__
        // Send log messages of debug (and trace, which are roughly equivalent
        // to debug) severity, which are usually not shown at all under MSW, to
        // stderr.
        //
        // The end users wouldn't see them there as they don't run the program
        // from a terminal, but they could be potentially valuable to the
        // developers.
        struct DebugStderrLog : wxLogInterposer
        {
            void DoLogTextAtLevel(wxLogLevel level, wxString const& msg) override
                {
                switch(level)
                    {
                    case wxLOG_FatalError:
                    case wxLOG_Error:
                    case wxLOG_Warning:
                    case wxLOG_Message:
                    case wxLOG_Status:
                    case wxLOG_Info:
                        break;

                    case wxLOG_Debug:
                    case wxLOG_Trace:
                        wxMessageOutputStderr().Output(msg);
                        break;
                    }
                }
        };

        wxLog::SetActiveTarget(new(wx) DebugStderrLog);
#endif // defined __WXMSW__

        if(false == ProcessCommandLine())
            {
            return false;
            }

#if defined LMI_MSW
        // Preload DLLs after calling ProcessCommandLine(). Reason:
        // command-line option '--data_path' may specify the directory
        // from which 'configurable_settings.xml' is read, and that
        // XML file gives the list of DLLs to preload.
        MswDllPreloader::instance().PreloadDesignatedDlls();
#endif // defined LMI_MSW

        authenticate_system();

        wxInitAllImageHandlers();

        // For GTK+, native theme takes precedence over local icons.
        // For other platforms, local icons take precedence.
#if defined __WXGTK__
        wxArtProvider::PushBack(new(wx) icon_monger);
#else  // !defined __WXGTK__
        wxArtProvider::Push    (new(wx) icon_monger);
#endif // !defined __WXGTK__

        wxXmlResource& xml_resources = *wxXmlResource::Get();

        xml_resources.InitAllHandlers();
        xml_resources.AddHandler(new(wx) RoundingButtonsXmlHandler);
        xml_resources.AddHandler(new(wx) InputSequenceEntryXmlHandler);

        load_xrc_file_from_data_directory(xml_resources, DefaultView().ResourceFileName());
        load_xrc_file_from_data_directory(xml_resources, PreferencesView().ResourceFileName());
        load_xrc_file_from_data_directory(xml_resources, mec_mvc_view().ResourceFileName());
        load_xrc_file_from_data_directory(xml_resources, gpt_mvc_view().ResourceFileName());
        load_xrc_file_from_data_directory(xml_resources, "menus.xrc");
        load_xrc_file_from_data_directory(xml_resources, "toolbar.xrc");
        load_xrc_file_from_data_directory(xml_resources, PolicyView::resource_file_name());
        load_xrc_file_from_data_directory(xml_resources, RoundingView::resource_file_name());

        InitDocManager();

        frame_ = new(wx) wxDocMDIParentFrame
            (doc_manager_
            ,nullptr
            ,wxID_ANY
            ,"lmi"
            ,wxDefaultPosition
            ,wxDefaultSize
            ,   wxDEFAULT_FRAME_STYLE
            |   wxFRAME_NO_WINDOW_MENU
            |   wxHSCROLL
            |   wxVSCROLL
            );

        InitHelp();
        InitIcon();
        InitMenuBar();
        InitToolBar();
        frame_->CreateStatusBar();

        frame_->Bind(wxEVT_MENU_OPEN, &Skeleton::UponMenuOpen, this);
        frame_->DragAcceptFiles(true);

        if(!wxPersistentRegisterAndRestore(frame_, "lmi_main"))
            {
            frame_->Center(wxBOTH);
            frame_->Maximize(true);
            }

        if
            (  custom_io_0_run_if_file_exists(doc_manager_)
            || custom_io_1_run_if_file_exists()
            )
            {
            return false;
            }

        frame_->Show(true);
        SetTopWindow(frame_);

        // This handler may write to the statusbar, so connect it only
        // after the frame has been created.
        Connect
            (wxEVT_TIMER
            ,wxTimerEventHandler(Skeleton::UponTimer)
            );

        // Intercept 'Text Paste' events for all windows.
        Connect
            (wxEVT_COMMAND_TEXT_PASTE
            ,wxClipboardTextEventHandler(Skeleton::UponPaste)
            );

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
        //   https://lists.nongnu.org/archive/html/lmi/2005-12/msg00020.html
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

void Skeleton::UponMenuOpen(wxMenuEvent& event)
{
    event.Skip();

    wxMDIChildFrame* child_frame = frame_->GetActiveChild();
    if(child_frame)
        {
        bool has_multiple_mdi_children = false;
        for(auto const& i : frame_->GetChildren())
            {
            wxMDIChildFrame const*const child = dynamic_cast<wxMDIChildFrame*>(i);
            if(child && child != child_frame)
                {
                has_multiple_mdi_children = true;
                break;
                }
            }

        wxMenuItem* window_next = child_frame->GetMenuBar()->FindItem
            (XRCID("window_next")
            );
        if(window_next)
            {
            window_next->Enable(has_multiple_mdi_children);
            }

        wxMenuItem* window_previous = child_frame->GetMenuBar()->FindItem
            (XRCID("window_previous")
            );
        if(window_previous)
            {
            window_previous->Enable(has_multiple_mdi_children);
            }
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
        for(auto const& i : original_text)
            {
            switch(i)
                {
                case '\n': {*j++ = ';';} break;
                case '\r': {           } break;
                case '\t': {*j++ = ';';} break;
                default  : {*j++ =   i;}
                }
            }

        return new_text;
        }
} // Unnamed namespace.

/// Handle 'Text Paste' events for all windows.
///
/// The behavior depends upon the event-object type.
///
/// Type InputSequenceEntry: Paste "\n"-, "\r\n"-, or "\t"-delimited
/// clipboard contents into the associated text control, replacing
/// nonterminal delimiters with semicolons to form an input sequence,
/// and removing any terminal delimiters. The motivation is to permit
/// pasting spreadsheet columns.
///
/// Type wxTextCtrl: Paste literal clipboard contents. Text delimited
/// with "\n" or "\r\n" is shown on distinct lines in a multiline text
/// control with all delimiters removed; in single-line text controls,
/// all delimiters are replaced by RETURN_SYMBOL.
///
/// All other types: ignore the paste event.

void Skeleton::UponPaste(wxClipboardTextEvent& event)
{
    event.Skip();

    wxTextCtrl* t = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if(!t)
        {
        return;
        }

    if(!dynamic_cast<InputSequenceEntry*>(t->GetParent()))
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
        configurable_settings& z = configurable_settings::instance();
        std::string const orig_skin_filename = z.skin_filename();
        preferences.Save();
        z.save();
        if(z.skin_filename() != orig_skin_filename)
            {
            // If the default skin file name has changed, we need to explicitly
            // re-load the corresponding file, otherwise the old definitions of
            // the objects defined in it would be still used when they're
            // needed the next time.
            wxXmlResource& res = *wxXmlResource::Get();
            res.Unload(AddDataDir(orig_skin_filename));
            load_xrc_file_from_data_directory(res, DefaultView().ResourceFileName());
            }

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
    alarum()         << "Test alarum() ."         << LMI_FLUSH;
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
    test_alarum();
}

void Skeleton::UponTestLibStandardException(wxCommandEvent&)
{
    test_standard_exception();
}

void Skeleton::UponTestLibArbitraryException(wxCommandEvent&)
{
    test_arbitrary_exception();
}

/// Test catastrophic-error report.
///
/// This error occurs only when normal error reporting is impossible;
/// it is internal to 'alert.cpp', so no corresponding application-
/// level UponTestAppCatastropheReport() can be written.

void Skeleton::UponTestLibCatastropheReport(wxCommandEvent&)
{
    test_catastrophe_report();
}

void Skeleton::UponTestDateConversions(wxCommandEvent&)
{
    TestDateConversions();
}

void Skeleton::UponTestFloatingPointEnvironment(wxCommandEvent&)
{
#if defined LMI_X87
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
#else  // !defined LMI_X87
    warning() << "This test does nothing at present." << std::flush;
#endif // !defined LMI_X87
}

/// Test custom handler UponPaste().
///
/// See:
///   https://savannah.nongnu.org/task/?5224

void Skeleton::UponTestPasting(wxCommandEvent&)
{
    auto const z = std::make_unique<InputSequenceEntry>
        (frame_
        ,wxID_ANY
        ,"Testing..."
        );
    wxTextCtrl& t = z->text_ctrl();

    ClipboardEx::SetText("1\r\n2\r\n3\r\n");
    t.SetSelection(-1L, -1L);
    t.Paste();
    if("1;2;3;" != t.GetValue())
        {
        warning() << "'1;2;3;' != '" << t.GetValue() << "'" << LMI_FLUSH;
        }

    ClipboardEx::SetText("X\tY\tZ\t");
    t.SetSelection(-1L, -1L);
    t.Paste();
    if("X;Y;Z;" != t.GetValue())
        {
        warning() << "'X;Y;Z;' != '" << t.GetValue() << "'" << LMI_FLUSH;
        }

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
        ,&TopWindow()
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
    wxSafeShowMessage("Fatal error", "Terminating due to unhandled exception.");
}

void Skeleton::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
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
    frame_->Tile(wxVERTICAL);
}

bool Skeleton::ProcessCommandLine()
{
    // TRICKY !! Some long options are aliased to unlikely octal values.
    static Option long_options[] =
      {
        {"ash_nazg"     ,NO_ARG   ,nullptr ,001 ,nullptr ,"ash nazg durbatulûk"},
        {"ash_naz"      ,NO_ARG   ,nullptr ,077 ,nullptr ,"fraud"},
        {"mellon"       ,NO_ARG   ,nullptr ,002 ,nullptr ,"pedo mellon a minno"},
        {"mello"        ,NO_ARG   ,nullptr ,077 ,nullptr ,"fraud"},
        {"prospicience" ,REQD_ARG ,nullptr ,003 ,nullptr ,"validation date"},
        {"data_path"    ,REQD_ARG ,nullptr ,'d' ,nullptr ,"path to data files"},
        {"file"         ,REQD_ARG ,nullptr ,'f' ,nullptr ,"input file to run"},
        {"help"         ,NO_ARG   ,nullptr ,'h' ,nullptr ,"display this help and exit"},
        {"print_db"     ,NO_ARG   ,nullptr ,'p' ,nullptr ,"print products and exit"},
        {"test_db"      ,NO_ARG   ,nullptr ,'t' ,nullptr ,"test products and exit"},
        {"pyx"          ,REQD_ARG ,nullptr ,'x' ,nullptr ,"for docimasy"},
        {nullptr        ,NO_ARG   ,nullptr ,000 ,nullptr ,""}
      };

    std::vector<std::string> input_files;

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

            case 003:
                {
                std::istringstream iss(getopt_long.optarg);
                int ymd_as_int;
                iss >> ymd_as_int;
                if(!iss || !iss.eof())
                    {
                    warning() << "Invalid prospicience option value '"
                              << getopt_long.optarg
                              << "' (must be in YYYYMMDD format)."
                              << std::flush
                              ;
                    }
                else
                    {
                    global_settings::instance().set_prospicience_date
                        (calendar_date(ymd_t(ymd_as_int))
                        );
                    }
                }
                break;

            case 'd':
                {
                global_settings::instance().set_data_directory
                    (getopt_long.optarg
                    );
                }
                break;

            case 'f':
                {
                LMI_ASSERT(nullptr != getopt_long.optarg);
                input_files.push_back(getopt_long.optarg);
                }
                break;

            case 'h':
                {
                std::ostringstream oss;
                getopt_long.usage(oss);
                wxMessageBox(oss.str(), "Command-line options");
                return false;
                }
                break;

            case 'p':
                {
                print_databases();
                return false;
                }
                break;

            case 't':
                {
                verify_products();
                return false;
                }
                break;

            case 'x':
                {
                global_settings::instance().set_pyx(getopt_long.optarg);
                if(contains(global_settings::instance().pyx(), "system_testing"))
                    {
                    global_settings::instance().set_regression_testing(true);
                    }
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

    if(!input_files.empty())
        {
        // Can't open files until main window is initialized.
        CallAfter(&Skeleton::OpenCommandLineFiles, input_files);
        }

    return true;
}

void Skeleton::OpenCommandLineFiles(std::vector<std::string> const& files)
{
    LMI_ASSERT(doc_manager_);

    for(auto const& i : files)
        {
        if(!doc_manager_->CreateDocument(i, wxDOC_SILENT))
            {
            warning()
                << "Document '"
                << i
                << "' specified on command line couldn't be opened."
                << LMI_FLUSH
                ;
            }
        }
}

/// Update all MVC views potentially affected by a global change.
///
/// As of 2018-02, the only update trigger is editing the preferences
/// dialog, which may change the calculation-summary column selection.
///
/// To improve responsiveness, update any active child first.

void Skeleton::UpdateViews()
{
    wxBusyCursor reverie;

    // Make a local copy of the list for modification.
    // Bring any active child to front so it's updated first.
    // It doesn't matter here if it's null: that's filtered below.
    wxMDIChildFrame*     a = frame_->GetActiveChild();
    wxWindowList const&  y = frame_->GetChildren();
    std::list<wxWindow*> z(y.begin(), y.end());
    z.remove(a);
    z.push_front(a);

    for(auto const& i : z)
        {
        wxDocMDIChildFrame* c = dynamic_cast<wxDocMDIChildFrame*>(i);
        if(c)
            {
            IllustrationView* v = dynamic_cast<IllustrationView*>(c->GetView());
            if(v)
                {
                v->DisplaySelectedValuesAsHtml();
                c->Update();
                }
            }
        }
}
