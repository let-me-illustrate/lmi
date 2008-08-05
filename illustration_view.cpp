// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.84 2008-08-05 10:43:19 chicares Exp $

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

#include "alert.hpp"
#include "assert_lmi.hpp"
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
#include "ledgervalues.hpp"
#include "mvc_controller.hpp"
#include "safely_dereference_as.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp" // class ClipboardEx

#include <boost/scoped_ptr.hpp>

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <sstream>

IMPLEMENT_DYNAMIC_CLASS(IllustrationView, ViewEx)

BEGIN_EVENT_TABLE(IllustrationView, ViewEx)
    EVT_MENU(wxID_PRINT                     ,IllustrationView::UponPrintPdf  )
    EVT_MENU(wxID_PREVIEW                   ,IllustrationView::UponPreviewPdf)
    EVT_MENU(XRCID("edit_cell"             ),IllustrationView::UponProperties)
    EVT_MENU(XRCID("copy_summary"          ),IllustrationView::UponCopySummary)
    EVT_MENU(wxID_COPY                      ,IllustrationView::UponCopyFull)
    EVT_MENU(XRCID("preview_summary"       ),IllustrationView::UponPreviewSummary)
    EVT_MENU(XRCID("print_summary"         ),IllustrationView::UponPrintSummary)
    EVT_MENU_OPEN(                           IllustrationView::UponMenuOpen  )
    EVT_UPDATE_UI(wxID_SAVE                 ,IllustrationView::UponUpdateFileSave)
//    EVT_UPDATE_UI(wxID_SAVEAS               ,IllustrationView::UponUpdateFileSaveAs)
//    EVT_UPDATE_UI(XRCID("wxID_SAVEAS"      ),IllustrationView::UponUpdateFileSaveAs)
    EVT_UPDATE_UI(XRCID("edit_cell"        ),IllustrationView::UponUpdateProperties)

// There has to be a better way to inhibit these inapplicable ids.
    EVT_UPDATE_UI(XRCID("edit_class"           ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"           ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"          ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),IllustrationView::UponUpdateInapplicable)
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

inline Input& IllustrationView::input_data()
{
    return *document().doc_.input_data_;
}

IllustrationDocument& IllustrationView::document() const
{
    return safely_dereference_as<IllustrationDocument>(GetDocument());
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

    Input edited_lmi_input = input_data();
    DefaultView const default_view;
    MvcController controller(GetFrame(), edited_lmi_input, default_view);
    int rc = controller.ShowModal();
    if(wxID_OK == rc)
        {
        if(edited_lmi_input != input_data())
            {
/* TODO ?? Expunge this?
            warning()
                << input_data().differing_fields(edited_lmi_input)
                << LMI_FLUSH
                ;
*/
            input_data() = edited_lmi_input;
            dirty = true;
            }
        document().Modify(dirty);
        }
    return rc;
}

void IllustrationView::DisplaySelectedValuesAsHtml()
{
    // TODO ?? CALCULATION_SUMMARY Resolve this issue.
    // EVGENIY Is a stream the best abstraction for LedgerFormatter?
    // Apparently std::ostream.write() is the only stream function
    // that actually gets called. This code could be simpler if a
    // std::string were used instead; is there a reason to do
    // otherwise?
    std::ostringstream oss;
    ledger_formatter_.FormatAsHtml(oss);
    selected_values_as_html_ = oss.str();

    html_window_->SetPage(selected_values_as_html_);
}

wxIcon IllustrationView::Icon() const
{
    return IconFromXmlResource("illustration_view_icon");
}

wxMenuBar* IllustrationView::MenuBar() const
{
    return MenuBarFromXmlResource("illustration_view_menu");
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

void IllustrationView::UponCopyFull(wxCommandEvent&)
{
    CopyLedgerToClipboard(e_copy_full);
}

void IllustrationView::UponCopySummary(wxCommandEvent&)
{
    CopyLedgerToClipboard(e_copy_summary);
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

void IllustrationView::UponPreviewSummary(wxCommandEvent&)
{
    PrintOrPreviewHtmlSummary(e_print_preview);
}

void IllustrationView::UponPrintPdf(wxCommandEvent&)
{
    Pdf("print");
}

void IllustrationView::UponPrintSummary(wxCommandEvent&)
{
    PrintOrPreviewHtmlSummary(e_print_printer);
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

/// This completely replaces wxDocManager::OnUpdateFileSave(),
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

void IllustrationView::CopyLedgerToClipboard(enum_copy_option option)
{
    Timer timer;

    std::ostringstream oss;
    if(e_copy_full == option)
        {
        ledger_formatter_.FormatAsTabDelimited(oss);
        }
    // TODO ?? CALCULATION_SUMMARY This assumes, without asserting,
    // that the enumeration has exactly two enumerators.
    else
        {
        ledger_formatter_.FormatAsLightTSV(oss);
        }

    ClipboardEx::SetText(oss.str());

    status() << "Format: " << timer.stop().elapsed_msec_str() << std::flush;
}

void IllustrationView::Pdf(std::string const& action) const
{
    LMI_ASSERT(ledger_values_.get());

    std::string filename(document().GetUserReadableName());

    std::string pdf_out_file = write_ledger_as_pdf(*ledger_values_, filename);
    file_command()(pdf_out_file, action);
}

// TODO ?? CALCULATION_SUMMARY This should use either the code or the
// ideas in DocManagerEx::UponPreview().

void IllustrationView::PrintOrPreviewHtmlSummary(enum_print_option option) const
{
    std::string disclaimer
        ("FOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS."
        " (Page @PAGENUM@ of @PAGESCNT@)<hr/>"
        );
    wxHtmlEasyPrinting printer("Calculation Summary", html_window_);

    printer.SetHeader(disclaimer, wxPAGE_ALL);

    // TODO ?? CALCULATION_SUMMARY Resolve this issue. This advice:
    //   You should create an instance on app startup and use this
    //   instance for all printing operations. The reason is that this
    //   class stores various settings in it.
    // from the wxHtmlEasyPrinting documentation has not been heeded.
    // Furthermore, no corresponding change has been made anywhere
    // else that lmi uses a wxPrintData object.
    //
    // WX !! Could printer settings should be set globally, OAOO,
    // for all classes that ought to use them? It was reported that
    // 'A4' was used here unless explicitly overridden, but it seems
    // that the paper id was actually wxPAPER_NONE; if that causes
    // 'A4' to be used, then should wx instead use wxPAPER_LETTER in
    // a US locale, where 'A4' is a poor default?
    printer.GetPrintData()->SetPaperId(wxPAPER_LETTER);

    if(e_print_printer == option)
        {
        printer.PrintText(selected_values_as_html_);
        }
    // TODO ?? CALCULATION_SUMMARY This assumes, without asserting,
    // that the enumeration has exactly two enumerators.
    else
        {
        printer.PreviewText(selected_values_as_html_);
        }
}

void IllustrationView::Run(Input* overriding_input)
{
    Timer timer;

    if(overriding_input)
        {
        input_data() = *overriding_input;
        }

    IllusVal IV(base_filename() + ".debug");
    IV.run(input_data());
    boost::shared_ptr<Ledger> resulting_ledger(new Ledger(mce_ill_reg));
    *resulting_ledger = IV.ledger();

// EVGENIY !! Originally, we had this:
//    status() << "Calculate: " << timer.stop().elapsed_msec_str();
// and that used to work; but now the call to
//   DisplaySelectedValuesAsHtml() seems to empty that stream,
// apparently on the line above that calls this function:
//   ledger_formatter_.FormatAsHtml
// Do you have any idea why? I don't see why that should have any
// such effect.

    std::ostringstream oss;
    oss << "Calculate: " << timer.stop().elapsed_msec_str();
    timer.restart();

// TODO ?? CALCULATION_SUMMARY Consider restoring this line:
//    ledger_values_ = resulting_ledger;
// in place of the following, which is discussed below:
    SetLedger(resulting_ledger);

    DisplaySelectedValuesAsHtml();

    status() << oss.str();
    status() << "; format: " << timer.stop().elapsed_msec_str();
    status() << std::flush;
}

// TODO ?? EVGENIY This function was created merely as a kludge: class
// CensusView calls MakeNewIllustrationDocAndView(), and for some
// forgotten reason I didn't find a better way to pass the 'ledger'
// shared_ptr.
//
// I'd prefer not to call it anywhere else, because that makes it
// harder to remove the kludge.
//
// Anyway, I think that it's an error if '0 == get()' for that
// shared_ptr argument, and that I should have asserted that here.
//
// Therefore, I think we can avoid the else-statement, which seems
// suspicious anyway: how would a default-constructed LedgerFormatter
// behave when we try to use it?
//
// And should ledger_formatter_ be held by value anyway? Why not use a
// shared_ptr for it, as is done for ledger_values_, or probably a
// scoped_ptr member instead? Then "ledger_formatter.hpp" could be
// removed from the header, simplifying the physical design.

void IllustrationView::SetLedger(boost::shared_ptr<Ledger const> ledger)
{
    ledger_values_ = ledger;
    if(ledger_values_.get())
        {
        LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
        ledger_formatter_ = factory.CreateFormatter(*ledger_values_);
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

    IllustrationDocument& illdoc =
        safely_dereference_as<IllustrationDocument>(new_document)
        ;
    illdoc.SetFilename(filename, true);
    illdoc.Modify(false);
    illdoc.SetDocumentSaved(true);
    return illdoc.PredominantView();
}

// Must follow document-manager initialization.
// Return value: prevent displaying GUI.
bool custom_io_0_run_if_file_exists(wxDocManager* dm)
{
// TODO ?? It's silly to write try...catch here, but the customer that
// needs this is a thousand miles away, and wx still seems not to
// handle exceptions well--an exception thrown here was demonstrably
// unobservable without this extra work.
    try
        {
        if(custom_io_0_file_exists())
            {
            IllusInputParms input;
            bool close_when_done = custom_io_0_read(input, "");
            IllusVal IV;
            IV.run(input);
            custom_io_0_write(IV.ledger(), "");
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
                safely_dereference_as<wxFrame>(illview.GetFrame()).Maximize();
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

