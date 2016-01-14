// Document view for MEC testing.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_view.hpp"

#include "edit_mvc_docview_parameters.hpp"
#include "handle_exceptions.hpp"
#include "mec_document.hpp"
#include "mec_input.hpp"
#include "mec_server.hpp"
#include "safely_dereference_as.hpp"
#include "wx_new.hpp"

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

mec_mvc_view::mec_mvc_view()
{
}

mec_mvc_view::~mec_mvc_view()
{
}

char const* mec_mvc_view::DoBookControlName() const
{
    return "mec_notebook";
}

char const* mec_mvc_view::DoMainDialogName() const
{
    return "dialog_containing_mec_notebook";
}

char const* mec_mvc_view::DoResourceFileName() const
{
    return "mec.xrc";
}

IMPLEMENT_DYNAMIC_CLASS(mec_view, ViewEx)

BEGIN_EVENT_TABLE(mec_view, ViewEx)
    EVT_MENU(XRCID("edit_cell"                 ),mec_view::UponProperties        )
    EVT_UPDATE_UI(wxID_SAVE                     ,mec_view::UponUpdateFileSave    )
    EVT_UPDATE_UI(wxID_SAVEAS                   ,mec_view::UponUpdateFileSaveAs  )
    EVT_UPDATE_UI(XRCID("print_pdf"            ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_cell"            ),mec_view::UponUpdateProperties  )
    EVT_UPDATE_UI(XRCID("edit_class"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"          ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_group_roster"   ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),mec_view::UponUpdateInapplicable)
END_EVENT_TABLE()

mec_view::mec_view()
    :ViewEx       ()
    ,html_content_("Unable to display results.")
    ,html_window_ (0)
{
}

mec_view::~mec_view()
{
}

inline mec_input& mec_view::input_data()
{
    return document().doc_.input_data_;
}

mec_document& mec_view::document() const
{
    return safely_dereference_as<mec_document>(GetDocument());
}

wxWindow* mec_view::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

oenum_mvc_dv_rc mec_view::edit_parameters()
{
    return edit_mvc_docview_parameters<mec_mvc_view>
        (input_data()
        ,document()
        ,GetFrame()
        );
}

wxIcon mec_view::Icon() const
{
    return IconFromXmlResource("mec_view_icon");
}

wxMenuBar* mec_view::MenuBar() const
{
    return MenuBarFromXmlResource("mec_view_menu");
}

/// This virtual function calls its base-class namesake explicitly.
///
/// Trap exceptions to ensure that this function returns 'false' on
/// failure, lest wx's doc-view framework create a zombie view. See:
///   http://lists.nongnu.org/archive/html/lmi/2008-12/msg00017.html

bool mec_view::OnCreate(wxDocument* doc, long int flags)
{
    bool has_view_been_created = false;
    try
        {
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

wxPrintout* mec_view::OnCreatePrintout()
{
    wxHtmlPrintout* z = new(wx) wxHtmlPrintout;
    safely_dereference_as<wxHtmlPrintout>(z).SetHtmlText(html_content_.c_str());
    return z;
}

void mec_view::UponProperties(wxCommandEvent&)
{
    if(oe_mvc_dv_changed == edit_parameters())
        {
        Run();
        }
}

/// This complete replacement for wxDocManager::OnUpdateFileSave()
/// should not call Skip().

void mec_view::UponUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(document().IsModified());
}

/// This complete replacement for wxDocManager::OnUpdateFileSaveAs()
/// should not call Skip().

void mec_view::UponUpdateFileSaveAs(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void mec_view::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

void mec_view::UponUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void mec_view::Run()
{
    input_data().RealizeAllSequenceInput();
    mec_server server(mce_emit_test_data);
    server(base_filename(), input_data());
    html_content_ = server.state().format_as_html(input_data()["Comments"].str());
    html_window_->SetPage(html_content_);
}

