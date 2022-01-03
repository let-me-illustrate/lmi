// Document view for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "gpt_view.hpp"

#include "edit_mvc_docview_parameters.hpp"
#include "gpt_document.hpp"
#include "gpt_input.hpp"
#include "gpt_server.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "safely_dereference_as.hpp"
#include "wx_new.hpp"

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/xrc/xmlres.h>

char const* gpt_mvc_view::DoBookControlName() const
{
    return "gpt_notebook";
}

char const* gpt_mvc_view::DoMainDialogName() const
{
    return "dialog_containing_gpt_notebook";
}

char const* gpt_mvc_view::DoResourceFileName() const
{
    return "gpt.xrc";
}

IMPLEMENT_DYNAMIC_CLASS(gpt_view, ViewEx)

BEGIN_EVENT_TABLE(gpt_view, ViewEx)
    EVT_MENU(XRCID("edit_cell"                 ),gpt_view::UponProperties        )
    EVT_UPDATE_UI(wxID_SAVE                     ,gpt_view::UponUpdateFileSave    )
    EVT_UPDATE_UI(wxID_SAVEAS                   ,gpt_view::UponUpdateFileSaveAs  )
    EVT_UPDATE_UI(XRCID("print_pdf"            ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_cell"            ),gpt_view::UponUpdateProperties  )
    EVT_UPDATE_UI(XRCID("edit_class"           ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"           ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"          ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_group_roster"   ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_group_quote"    ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("copy_census"          ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),gpt_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),gpt_view::UponUpdateInapplicable)
END_EVENT_TABLE()

inline gpt_input& gpt_view::input_data()
{
    return document().doc_.input_data_;
}

gpt_document& gpt_view::document() const
{
    return safely_dereference_as<gpt_document>(GetDocument());
}

wxWindow* gpt_view::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

oenum_mvc_dv_rc gpt_view::edit_parameters()
{
    return edit_mvc_docview_parameters<gpt_mvc_view>
        (input_data()
        ,document()
        ,GetFrame()
        );
}

char const* gpt_view::icon_xrc_resource() const
{
    return "gpt_view_icon";
}

char const* gpt_view::menubar_xrc_resource() const
{
    return "gpt_view_menu";
}

/// Pop up an input dialog; iff it's not cancelled, create a view.
///
/// Trap exceptions to ensure that this function returns 'false' on
/// failure, lest wx's doc-view framework create a zombie view. See:
///   https://lists.nongnu.org/archive/html/lmi/2008-12/msg00017.html

bool gpt_view::OnCreate(wxDocument* doc, long int flags)
{
    bool has_view_been_created = false;
    try
        {
        if(oe_mvc_dv_cancelled == edit_parameters())
            {
            return has_view_been_created;
            }

        has_view_been_created = ViewEx::DoOnCreate(doc, flags);
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

wxPrintout* gpt_view::OnCreatePrintout()
{
    wxHtmlPrintout* z = new(wx) wxHtmlPrintout;
    safely_dereference_as<wxHtmlPrintout>(z).SetHtmlText(html_content_.c_str());
    return z;
}

void gpt_view::UponProperties(wxCommandEvent&)
{
    if(oe_mvc_dv_changed == edit_parameters())
        {
        Run();
        }
}

/// This complete replacement for wxDocManager::OnUpdateFileSave()
/// should not call Skip().

void gpt_view::UponUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(document().IsModified());
}

/// This complete replacement for wxDocManager::OnUpdateFileSaveAs()
/// should not call Skip().

void gpt_view::UponUpdateFileSaveAs(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void gpt_view::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

void gpt_view::UponUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void gpt_view::Run()
{
    input_data().RealizeAllSequenceInput();
    gpt_server server(mce_emit_test_data);
    server(base_filename(), input_data());
    html_content_ = server.state().format_as_html(input_data()["Comments"].str());
    html_window_->SetPage(html_content_);
}
