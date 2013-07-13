// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

// $Id$

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license, and Julian Smart's
// message here:
//   http://groups.google.com/group/comp.soft-sys.wxwindows/msg/b05623f68906edbd
//
// The originals were modified by GWC in 2003 to create a standalone
// view class customized for illustration documents, and in the later
// years given in the copyright notice above.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "illustration_view.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "default_view.hpp"
#include "edit_mvc_docview_parameters.hpp"
#include "emit_ledger.hpp"
#include "handle_exceptions.hpp"
#include "illustration_document.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "istream_to_string.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "safely_dereference_as.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"               // class ClipboardEx

#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <cstdio>                       // std::remove()
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
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_group_roster"   ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),IllustrationView::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),IllustrationView::UponUpdateInapplicable)
END_EVENT_TABLE()

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
    return document().doc_.input_data_;
}

IllustrationDocument& IllustrationView::document() const
{
    return safely_dereference_as<IllustrationDocument>(GetDocument());
}

wxWindow* IllustrationView::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

oenum_mvc_dv_rc IllustrationView::edit_parameters()
{
    if(is_phony_)
        {
warning() << "That command should have been disabled." << LMI_FLUSH;
        return oe_mvc_dv_cancelled;
        }

    return edit_mvc_docview_parameters<DefaultView>
        (input_data()
        ,document()
        ,GetFrame()
        );
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
/// Trap exceptions to ensure that this function returns 'false' on
/// failure, lest wx's doc-view framework create a zombie view. See:
///   http://lists.nongnu.org/archive/html/lmi/2008-12/msg00017.html

bool IllustrationView::OnCreate(wxDocument* doc, long int flags)
{
    bool has_view_been_created = false;

    try
        {
        if(flags & LMI_WX_CHILD_DOCUMENT)
            {
            is_phony_ = true;
            has_view_been_created = ViewEx::OnCreate(doc, flags);
            return has_view_been_created;
            }

        if(oe_mvc_dv_cancelled == edit_parameters())
            {
            return has_view_been_created;
            }

        has_view_been_created = ViewEx::OnCreate(doc, flags);
        if(!has_view_been_created)
            {
            return has_view_been_created;
            }

        Run();
        }
    catch(...)
        {
        report_exception();
        }

    return has_view_been_created;
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
    PrintCellTabDelimited(*ledger_values_, spreadsheet_filename);
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
    emit_ledger(base_filename(), "", *ledger_values_, mce_emit_pdf_to_viewer);
}

void IllustrationView::UponPrintPdf(wxCommandEvent&)
{
    LMI_ASSERT(ledger_values_.get());
    emit_ledger(base_filename(), "", *ledger_values_, mce_emit_pdf_to_printer);
}

void IllustrationView::UponProperties(wxCommandEvent&)
{
// may have to check is_phony_ here--but that's bogus
    if(is_phony_)
        {
//        return;
        }

    if(oe_mvc_dv_changed == edit_parameters())
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
    z(base_filename(), input_data());
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

/// Create a phantom child document and an associated corporeal view.
///
/// This could be generalized as a function template if that ever
/// becomes useful.

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

/// Run an illustration from "custom" input.
///
/// The return value indicates whether to prevent displaying the GUI.
///
/// If the GUI is to be displayed, then an '.ill' extension is added
/// to the output filename in order to force selection of the correct
/// document template.
///
/// This must be called only after document-manager initialization.
///
/// Because this function may prevent the GUI from being displayed, it
/// must trap and handle its own exceptions rather than letting them
/// escape to wx.

bool custom_io_0_run_if_file_exists(wxDocManager* dm)
{
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

