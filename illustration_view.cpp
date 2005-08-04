// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.21 2005-08-04 02:20:13 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and Julian Smart's
// message here:
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/b05623f68906edbd
//
// The originals were modified by GWC in 2003 to create a standalone
// view class customized for illustration documents, and in the later
// years given in the copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "illustration_view.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "argv0.hpp"
#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "file_command.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "miscellany.hpp"
#include "system_command.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <boost/filesystem/path.hpp>

#include <wx/app.h> // wxTheApp
#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/log.h> // TODO ?? Debugging only: consider removing.
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <cstdio>
#include <fstream>
#include <ios>
#include <locale>
#include <sstream>
#include <stdexcept>

IMPLEMENT_DYNAMIC_CLASS(IllustrationView, ViewEx)

BEGIN_EVENT_TABLE(IllustrationView, ViewEx)
    EVT_MENU(XRCID("edit_cell"             ),IllustrationView::OnProperties)
    EVT_MENU(wxID_PREVIEW                   ,IllustrationView::OnPreviewPdf)
    EVT_MENU(wxID_PRINT                     ,IllustrationView::OnPrintPdf  )
    EVT_UPDATE_UI(wxID_SAVE                 ,IllustrationView::OnUpdateFileSave)
//    EVT_UPDATE_UI(wxID_SAVEAS               ,IllustrationView::OnUpdateFileSaveAs)
//    EVT_UPDATE_UI(XRCID("wxID_SAVEAS"      ),IllustrationView::OnUpdateFileSaveAs)
    EVT_MENU_OPEN(                           IllustrationView::OnMenuOpen  )
    EVT_UPDATE_UI(XRCID("edit_cell"        ),IllustrationView::OnUpdateProperties)

// There has to be a better way.
    EVT_UPDATE_UI(XRCID("edit_class"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),IllustrationView::OnUpdateInapplicable)
END_EVENT_TABLE()

// WX !! The html string must be initialized here, because passing an
// empty string to wxHtmlWindow::SetPage() would crash the application.
//
IllustrationView::IllustrationView()
    :ViewEx                  ()
    ,html_window_            (0)
    ,is_phony_               (false)
    ,selected_values_as_html_("<html><body>Hello, world!</body></html>")
{
}

IllustrationView::~IllustrationView()
{
}

IllustrationDocument& IllustrationView::document() const
{
    return dynamic_cast<IllustrationDocument&>(*GetDocument());
}

wxWindow* IllustrationView::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

int IllustrationView::EditProperties()
{
    if(is_phony_)
        {
warning() << "That command should have been disabled." << LMI_FLUSH;
        return wxID_CANCEL;
        }

    bool dirty = document().IsModified();

    Input edited_lmi_input = document().input_;

    XmlNotebook xml_notebook(GetFrame(), edited_lmi_input);
    int rc = xml_notebook.ShowModal();
    if(wxID_OK == rc)
        {
        if(document().input_ != edited_lmi_input)
            {
// wxLogMessage(document().input_.differing_fields(edited_lmi_input).c_str());
// wxLog::FlushActive();
            document().input_ = edited_lmi_input;
            dirty = true;
            }
        document().Modify(dirty);
        }
    return rc;
}

void IllustrationView::DisplaySelectedValuesAsHtml()
{
    LMI_ASSERT(ledger_values_.get());
    selected_values_as_html_ = FormatSelectedValuesAsHtml(*ledger_values_);
    html_window_->SetPage(selected_values_as_html_.c_str());
}

wxIcon IllustrationView::Icon() const
{
    return IconFromXmlResource("illustration_view_icon");
}

wxMenuBar* IllustrationView::MenuBar() const
{
    wxMenuBar* menu_bar = MenuBarFromXmlResource("illustration_view_menu");
    if(!menu_bar)
        {
//        wxLogError("Unable to load 'illustration_view_menu'.");
//        wxLog::FlushActive();
        throw std::runtime_error("Unable to load 'illustration_view_menu'.");
        };
    return menu_bar;
}

bool IllustrationView::OnCreate(wxDocument* doc, long flags)
{
    if(flags & LMI_WX_CHILD_DOCUMENT)
        {
        is_phony_ = true;
        return ViewEx::OnCreate(doc, flags);
        }

    if(wxID_OK != EditProperties())
        {
        return false;
        }

    if(!ViewEx::OnCreate(doc, flags))
        {
        return false;
        }

    Run();
    return true;
}

void IllustrationView::OnMenuOpen(wxMenuEvent&)
{
// TODO ?? WX !! Never gets called. Does this need to be in the document class?
wxLogMessage("IllustrationView::OnMenuOpen() called.");
wxLog::FlushActive();

    wxMenuItem* file_save = MenuBar()->FindItem
        (XRCID("wxID_SAVE")
        );
    if(file_save)
        {
        file_save->Enable(!is_phony_ && document().IsModified());
        }

    wxMenuItem* file_save_as = MenuBar()->FindItem
        (XRCID("wxID_SAVEAS")
        );
    if(file_save_as)
        {
        file_save_as->Enable(!is_phony_);
        }

    wxMenuItem* edit_cell = MenuBar()->FindItem
        (XRCID("edit_cell")
        );
    if(edit_cell)
        {
        edit_cell->Enable(!is_phony_);
        }
}

void IllustrationView::OnPreviewPdf(wxCommandEvent&)
{
    Pdf("open");
}

void IllustrationView::OnPrintPdf(wxCommandEvent&)
{
    Pdf("print");
}

void IllustrationView::OnProperties(wxCommandEvent&)
{
// may have to check is_phony_ here--but that's bogus
    if(is_phony_)
        {
//        return;
        }
    if(wxID_OK == EditProperties())
        {
        Run();
        }
}

void IllustrationView::OnUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_ && document().IsModified());
}

void IllustrationView::OnUpdateFileSaveAs(wxUpdateUIEvent& e)
{
// TODO ?? Doesn't seem to get called.

// TODO ?? Is special logic required, here and elsewhere, to prevent
// actions that don't make sense with style LMI_WX_CHILD_DOCUMENT?

wxLogMessage("OnUpdateFileSaveAs");
wxLog::FlushActive();
    e.Enable(!is_phony_);
}

void IllustrationView::OnUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? Doesn't seem to handle the menuitem--just the toolbar.
void IllustrationView::OnUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_);
}

void IllustrationView::Pdf(std::string const& action) const
{
    LMI_ASSERT(ledger_values_.get());

    wxString z;
    document().GetPrintableName(z);
    std::string document_file(z.c_str());

    std::string pdf_out_file = write_ledger_to_pdf
        (*ledger_values_
        ,document_file
        );
    file_command()(pdf_out_file, action);
}

void IllustrationView::Run(Input* overriding_input)
{
    Timer timer;

    if(overriding_input)
        {
        document().input_ = *overriding_input;
        }

    // TODO ?? For now, convert input on the fly.
    IllusInputParms ihs_input;
    convert_to_ihs(ihs_input, document().input_);

    AccountValue av(ihs_input);
    av.SetDebugFilename(base_filename() + ".debug");
    av.RunAV();

    status() << "Calculate: " << timer.stop().elapsed_msec_str() << std::flush;

    ledger_values_ = av.ledger_from_av();
    DisplaySelectedValuesAsHtml();
}

void IllustrationView::SetLedger(boost::shared_ptr<Ledger const> ledger)
{
    ledger_values_ = ledger;
}

// This could be generalized as a function template if that ever
// becomes useful.
IllustrationView* MakeNewIllustrationDocAndView
    (wxDocManager* dm
    ,char const*   filename
    )
{
    LMI_ASSERT(0 != dm);
    LMI_ASSERT(0 != filename);

    wxDocTemplate* dt = dm->FindTemplateForPath(filename);
    LMI_ASSERT(0 != dt);

    wxDocument* new_document = dt->CreateDocument
        (filename
        ,wxDOC_SILENT | LMI_WX_CHILD_DOCUMENT
        );

    IllustrationDocument* illdoc = dynamic_cast<IllustrationDocument*>(new_document);
    if(0 == illdoc)
        {
        fatal_error() << "dynamic_cast<IllustrationDocument*> failed." << LMI_FLUSH;
        return 0;
        }

    new_document->SetFilename(filename, true);

    new_document->Modify(false);
    new_document->SetDocumentSaved(true);

    IllustrationView* illview = 0;
    while(wxList::compatibility_iterator node = new_document->GetViews().GetFirst())
        {
        if(node->GetData()->IsKindOf(CLASSINFO(IllustrationView)))
            {
            illview = dynamic_cast<IllustrationView*>(node->GetData());
            break;
            }
        node = node->GetNext();
        }
    if(0 == illview)
        {
        fatal_error() << "dynamic_cast<IllustrationView*> failed." << LMI_FLUSH;
        return 0;
        }
    return illview;
}

// Must follow document-manager initialization.
// Return value: prevent displaying GUI.
bool RunSpecialInputFileIfPresent(wxDocManager* dm)
{
// TODO ?? It's silly to write try...catch here, but the customer that
// needs this is a thousand miles away, and wx still seems not to
// handle exceptions well--an exception thrown here was demonstrably
// unobservable without this extra work.
    try
        {
        if(DoesSpecialInputFileExist())
            {
            IllusInputParms input;
            bool close_when_done = SetSpecialInput(input);
            AccountValue av(input);
            av.RunAV();
            PrintFormSpecial(*av.ledger_from_av());
            if(close_when_done)
                {
                return true;
                }
            else
                {
                LMI_ASSERT(0 != dm);
                std::string f
                    (configurable_settings::instance().custom_output_filename()
                    );
                // Add '.ill' extension to force use of the correct
                // document template, even if the filename didn't
                // match it.
                f += ".ill";
                IllustrationView* illview = MakeNewIllustrationDocAndView
                    (dm
                    ,f.c_str()
                    );

                // It seems silly to perform this conversion here,
                // because Run() performs an inverse conversion.
                // But time is better spent removing the root of
                // this silliness--the pair of duplicative input
                // classes--than fretting over its symptoms.
                Input x;
                convert_from_ihs(input, x);
                illview->Run(&x);
                LMI_ASSERT(dynamic_cast<wxFrame*>(illview->GetFrame()));
                dynamic_cast<wxFrame*>(illview->GetFrame())->Maximize();
                }
            }
        }
    catch(std::exception& e)
        {
        wxSafeShowMessage("Fatal error", e.what());
        throw;
        }
    catch(...)
        {
        wxSafeShowMessage("Fatal error", "Unknown error");
        throw;
        }
    return false;
}

