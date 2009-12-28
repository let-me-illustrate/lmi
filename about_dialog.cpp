// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: about_dialog.cpp,v 1.14 2009-03-31 19:33:09 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "about_dialog.hpp"

#include "license.hpp"
#include "version.hpp"

#include <wx/button.h>
#include <wx/html/htmlwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>

#include <string>

BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
    EVT_BUTTON(wxID_ABOUT, AboutDialog::UponReadLicense)
END_EVENT_TABLE()

AboutDialog::AboutDialog(wxWindow* parent)
    :wxDialog
        (parent
        ,wxID_ANY
        ,"About 'Let me illustrate...' version " + std::string(LMI_VERSION)
        )
{
}

AboutDialog::~AboutDialog()
{
}

/// This virtual function calls its base-class namesake explicitly.

int AboutDialog::ShowModal()
{
    wxHtmlWindow* html_window = new wxHtmlWindow
        (this
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION
        );
    html_window->SetBorders(0);
    html_window->SetPage(license_notices_as_html());
    int width =
            html_window->GetInternalRepresentation()->GetWidth()
        +   wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)
        ;
    int height = html_window->GetInternalRepresentation()->GetHeight();
    html_window->SetSize(width, height);

    wxButton* license_button = new wxButton
        (this
        ,wxID_ABOUT
        ,"Read the GNU General Public License"
        );
    license_button->SetDefault();
    wxButton* cancel_button = new wxButton
        (this
        ,wxID_CANCEL
        ,"Let me illustrate"
        );

    wxFlexGridSizer* sizer1 = new wxFlexGridSizer(2);
    sizer1->AddGrowableCol(0);
    sizer1->AddGrowableCol(1);
    sizer1->Add(license_button, 1, wxALL|wxALIGN_LEFT , 5);
    sizer1->Add(cancel_button , 1, wxALL|wxALIGN_RIGHT, 5);

    wxFlexGridSizer* sizer0 = new wxFlexGridSizer(1);
    sizer0->AddGrowableRow(0);
    sizer0->Add(html_window, 1, wxALL, 10);
    sizer0->Add(sizer1     , 1, wxALL, 10);

    SetAutoLayout(true);
    SetSizer(sizer0);
    sizer0->Fit(this);
    Center();
    return wxDialog::ShowModal();
}

void AboutDialog::UponReadLicense(wxCommandEvent&)
{
    wxDialog dialog(this, wxID_ANY, std::string("GNU General Public License"));
    wxHtmlWindow* html_window = new wxHtmlWindow
        (&dialog
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION
        );
    html_window->SetBorders(0);
    html_window->SetPage(license_as_html());
    html_window->GetInternalRepresentation()->Layout(1);
    int width =
            html_window->GetInternalRepresentation()->GetWidth()
        +   wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)
        ;
    int height = html_window->GetInternalRepresentation()->GetHeight();
    html_window->SetSize(width, height);

    wxButton* button = new wxButton(&dialog, wxID_CANCEL, "Close");
    button->SetDefault();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(html_window, 1, wxALL              , 10);
    sizer->Add(button     , 0, wxALL|wxALIGN_RIGHT, 10);

    dialog.SetAutoLayout(true);
    dialog.SetSizer(sizer);
    sizer->Fit(&dialog);
    dialog.Center();
    dialog.ShowModal();
}

