// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.101 2008-12-29 06:09:32 chicares Exp $

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
#include "emit_ledger.hpp"
#include "handle_exceptions.hpp"
#include "illustration_document.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "istream_to_string.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "mvc_controller.hpp"
#include "safely_dereference_as.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp" // class ClipboardEx

#include <boost/scoped_ptr.hpp>

#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <cstdio>         // std::remove()
#include <fstream>
#include <string>

IMPLEMENT_DYNAMIC_CLASS(IllustrationView, ViewEx)

BEGIN_EVENT_TABLE(IllustrationView, ViewEx)
    EVT_MENU(wxID_PRINT                     ,IllustrationView::UponPrintPdf  )
    EVT_MENU(wxID_PREVIEW                   ,IllustrationView::UponPreviewPdf)
    EVT_MENU(XRCID("edit_cell"             ),IllustrationView::UponProperties)
    EVT_MENU(XRCID("copy_summary"          ),IllustrationView::UponCopySummary)
    EVT_MENU(wxID_COPY                      ,IllustrationView::UponCopyFull)
    EVT_UPDATE_UI(wxID_SAVE                 ,IllustrationView::UponUpdateFileSave)
    EVT_UPDATE_UI(wxID_SAVEAS               ,IllustrationView::UponUpdateFileSaveAs)
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
    LMI_ASSERT(ledger_values_.get());
    html_window_->SetPage(FormatSelectedValuesAsHtml(*ledger_values_));
}

wxIcon IllustrationView::Icon() const
{
    return IconFromXmlResource("illustration_view_icon");
}

wxMenuBar* IllustrationView::MenuBar() const
{
    return MenuBarFromXmlResource("illustration_view_menu");
}

/// This virtual function calls its base-class namesake explicitly.
///
/// Trap any exception thrown by EditProperties() to ensure that this
/// function returns 'false' on failure, lest wx's doc-view framework
/// create a zombie view. See:
///   http://lists.nongnu.org/archive/html/lmi/2008-12/msg00017.html

bool IllustrationView::OnCreate(wxDocument* doc, long int flags)
{
    if(flags & LMI_WX_CHILD_DOCUMENT)
        {
        is_phony_ = true;
        return ViewEx::OnCreate(doc, flags);
        }

    try
        {
        if(wxID_OK != EditProperties())
            {
            return false;
            }
        }
    catch(...)
        {
        report_exception();
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
    LMI_ASSERT(ledger_values_.get());
    Timer timer;
    configurable_settings const& c = configurable_settings::instance();
    std::string spreadsheet_filename =
            base_filename()
        +   c.spreadsheet_file_extension()
        ;
    std::remove(spreadsheet_filename.c_str());
    PrintFormTabDelimited(*ledger_values_, spreadsheet_filename);
    std::ifstream ifs(spreadsheet_filename.c_str());
    std::string s;
    istream_to_string(ifs, s);
    ClipboardEx::SetText(s);
    status() << "Copy: " << timer.stop().elapsed_msec_str() << std::flush;
}

void IllustrationView::UponCopySummary(wxCommandEvent&)
{
    LMI_ASSERT(ledger_values_.get());
    Timer timer;
    ClipboardEx::SetText(FormatSelectedValuesAsTsv(*ledger_values_));
    status() << "Copy: " << timer.stop().elapsed_msec_str() << std::flush;
}

void IllustrationView::UponPreviewPdf(wxCommandEvent&)
{
    LMI_ASSERT(ledger_values_.get());
    emit_ledger(base_filename(), *ledger_values_, mce_emit_pdf_to_viewer);
}

void IllustrationView::UponPrintPdf(wxCommandEvent&)
{
    LMI_ASSERT(ledger_values_.get());
    emit_ledger(base_filename(), *ledger_values_, mce_emit_pdf_to_printer);
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

/// This complete replacement for wxDocManager::OnUpdateFileSave()
/// should not call Skip().

void IllustrationView::UponUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_ && document().IsModified());
}

/// This complete replacement for wxDocManager::OnUpdateFileSaveAs()
/// should not call Skip().

void IllustrationView::UponUpdateFileSaveAs(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_);
}

void IllustrationView::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

void IllustrationView::UponUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_);
}

void IllustrationView::Run(Input* overriding_input)
{
    Timer timer;

    if(overriding_input)
        {
        input_data() = *overriding_input;
        }

    illustrator z(mce_emit_nothing);
    z(base_filename() + ".debug", input_data());
    ledger_values_ = z.principal_ledger();

    status() << "Calculate: " << timer.stop().elapsed_msec_str();
    timer.restart();

    DisplaySelectedValuesAsHtml();

    status() << "; format: " << timer.stop().elapsed_msec_str();
    status() << std::flush;
}

/// Set ledger member from a calculation done elsewhere.
///
/// CensusView::ViewComposite() calls MakeNewIllustrationDocAndView()
/// to view a composite whose values are not conveniently calculated
/// in this TU, so they're passed via this function.
///
/// custom_io_0_run_if_file_exists() uses this function similarly.

void IllustrationView::SetLedger(boost::shared_ptr<Ledger const> ledger)
{
    ledger_values_ = ledger;
    LMI_ASSERT(ledger_values_.get());
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
            configurable_settings const& c = configurable_settings::instance();
            illustrator z(mce_emit_custom_0);
            bool close_when_done = z(c.custom_input_filename());
            if(close_when_done)
                {
                return true;
                }
            else
                {
                LMI_ASSERT(0 != dm);
                // Add '.ill' extension to force use of the correct
                // document template, even if the filename didn't
                // match it.
                IllustrationView& illview = MakeNewIllustrationDocAndView
                    (dm
                    ,(c.custom_output_filename() + ".ill").c_str()
                    );
                illview.SetLedger(z.principal_ledger());
                illview.DisplaySelectedValuesAsHtml();
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

