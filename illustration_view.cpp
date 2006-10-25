// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.41.2.5 2006-10-25 13:00:22 chicares Exp $

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
#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "default_view.hpp"
#include "file_command.hpp"
#include "handle_exceptions.hpp"
#include "illustration_document.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "mvc_controller.hpp"
#include "timer.hpp"
#include "wx_new.hpp"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <sstream>

IMPLEMENT_DYNAMIC_CLASS(IllustrationView, ViewEx)

BEGIN_EVENT_TABLE(IllustrationView, ViewEx)
    EVT_MENU(XRCID("edit_cell"             ),IllustrationView::UponProperties)
    EVT_MENU(wxID_PREVIEW                   ,IllustrationView::UponPreviewPdf)
    EVT_MENU(wxID_PRINT                     ,IllustrationView::UponPrintPdf  )
    EVT_UPDATE_UI(wxID_SAVE                 ,IllustrationView::UponUpdateFileSave)
//    EVT_UPDATE_UI(wxID_SAVEAS               ,IllustrationView::UponUpdateFileSaveAs)
//    EVT_UPDATE_UI(XRCID("wxID_SAVEAS"      ),IllustrationView::UponUpdateFileSaveAs)
    EVT_MENU_OPEN(                           IllustrationView::UponMenuOpen  )
    EVT_UPDATE_UI(XRCID("edit_cell"        ),IllustrationView::UponUpdateProperties)

// There has to be a better way.
    EVT_UPDATE_UI(XRCID("edit_class"       ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),IllustrationView::UponUpdateInapplicable)
END_EVENT_TABLE()

class HtmlWindow
    :public wxHtmlWindow
{
  public:
    HtmlWindow(IllustrationView* lview, wxWindow* parent);
    virtual ~HtmlWindow();

  private:
    IllustrationView* lview_;

    void OnKeyUp(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(HtmlWindow, wxHtmlWindow)
    EVT_KEY_UP(HtmlWindow::OnKeyUp)
END_EVENT_TABLE()

HtmlWindow::HtmlWindow(IllustrationView* lview, wxWindow* parent)
    :wxHtmlWindow(parent), lview_(lview)
{
}

HtmlWindow::~HtmlWindow()
{
}

// WX !! This code is taken from html/htmlwin.h. wxHtml does not seem
// to generate clipboard copy/paste events.
// Etarassov -- Update (20/10/2006): wxHtmlWindow is modified now to generate
// clipboard copy event properly. As soon as the patch makes into wx HEAD,
// i will fix this code to properly use copy event.

void HtmlWindow::OnKeyUp(wxKeyEvent& event)
{
    if(event.GetKeyCode() == 'C' && event.CmdDown())
        {
        lview_->CopyLedgerValues();
        }
    else
        {
        event.Skip();
        }
}

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
    return html_window_ = new(wx) HtmlWindow(this, GetFrame());
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
    DefaultView const default_view;
    MvcController controller(GetFrame(), edited_lmi_input, default_view);
    int rc = controller.ShowModal();
    if(wxID_OK == rc)
        {
        if(document().input_ != edited_lmi_input)
            {
/* TODO ?? Expunge this?
            warning()
                << document().input_.differing_fields(edited_lmi_input)
                << LMI_FLUSH
                ;
*/
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

    std::ostringstream oss;
    ledger_formatter_.FormatAsHtml(oss);
    selected_values_as_html_ = oss.str();

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
        fatal_error()
            << "Unable to load 'illustration_view_menu'."
            << LMI_FLUSH
            ;
        };
    return menu_bar;
}

// This virtual function calls the base-class version explicitly.

bool IllustrationView::OnCreate(wxDocument* doc, long int flags)
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

void IllustrationView::UponMenuOpen(wxMenuEvent&)
{
// TODO ?? WX !! Never gets called. Does this need to be in the document class?
    warning()
        << "This function apparently should not be called."
        << LMI_FLUSH
        ;

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

void IllustrationView::UponPreviewPdf(wxCommandEvent&)
{
    Pdf("open");
}

void IllustrationView::UponPrintPdf(wxCommandEvent&)
{
    Pdf("print");
}

void IllustrationView::UponProperties(wxCommandEvent&)
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

/// This is completely replaces wxDocManager::OnUpdateFileSave(),
/// and doesn't need to call Skip().

void IllustrationView::UponUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_ && document().IsModified());
}

void IllustrationView::UponUpdateFileSaveAs(wxUpdateUIEvent& e)
{
// TODO ?? Doesn't seem to get called.
    warning()
        << "This function apparently should not be called."
        << LMI_FLUSH
        ;

// TODO ?? Is special logic required, here and elsewhere, to prevent
// actions that don't make sense with style LMI_WX_CHILD_DOCUMENT?
    e.Enable(!is_phony_);
}

void IllustrationView::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? Doesn't seem to handle the menuitem--just the toolbar.
void IllustrationView::UponUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_);
}

void IllustrationView::CopyLedgerValues()
{
    wxClipboardLocker clipboardLocker;

    if(!clipboardLocker)
        return;

    std::ostringstream oss;
    ledger_formatter_.FormatAsTabDelimited(oss);
    wxTextDataObject* testDataObject = new wxTextDataObject(oss.str());

    // clipboard owns the data
    wxTheClipboard->SetData(testDataObject);
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

    status() << "Calculate: " << timer.stop().elapsed_msec_str();
    timer.restart();

    SetLedger(av.ledger_from_av());

    status() << "; prepare: " << timer.stop().elapsed_msec_str();
    timer.restart();

    DisplaySelectedValuesAsHtml();

    status() << "; format: " << timer.stop().elapsed_msec_str();
    status() << std::flush;
}

void IllustrationView::SetLedger(boost::shared_ptr<Ledger const> ledger)
{
    ledger_values_ = ledger;
    if (ledger_values_.get())
        {
        ledger_formatter_ = LedgerFormatterFactory::Instance().CreateFormatter(*ledger_values_);
        // in advance generate xml data for the calculation summary
        ledger_formatter_.PrepareXmlDataForHtml();
        ledger_formatter_.PrepareXmlDataForTabDelimited();
        }
    else
        {
        ledger_formatter_ = LedgerFormatter();
        }
}

// This could be generalized as a function template if that ever
// becomes useful.
IllustrationView& MakeNewIllustrationDocAndView
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

    IllustrationDocument* illdoc = dynamic_cast<IllustrationDocument*>
        (new_document
        );
    if(0 == illdoc)
        {
        fatal_error()
            << "dynamic_cast<IllustrationDocument*> failed."
            << LMI_FLUSH
            ;
        }

    new_document->SetFilename(filename, true);

    new_document->Modify(false);
    new_document->SetDocumentSaved(true);

    return illdoc->PredominantView();
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
            IllusInputParms input(false);
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
                IllustrationView& illview = MakeNewIllustrationDocAndView
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
                illview.Run(&x);
                LMI_ASSERT(dynamic_cast<wxFrame*>(illview.GetFrame()));
                dynamic_cast<wxFrame*>(illview.GetFrame())->Maximize();
                }
            }
        }
    catch(...)
        {
        report_exception();
        return true;
        }

    return false;
}

