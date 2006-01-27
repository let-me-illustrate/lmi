// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: about_dialog.cpp,v 1.3 2006-01-27 11:21:56 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "about_dialog.hpp"

#include "license.hpp"
#include "version.hpp"

#include <wx/button.h>
#include <wx/html/htmlwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>

// The 'about' dialog displays a wxHtmlWindow, and the license is
// optionally shown in another wxHtmlWindow. The first does not need
// scrolling; the second does. How should those windows interact with
// the keyboard?
//
// Perhaps it would be ideal to let the html text be selected, and the
// selection be copied, at least for the primary wxHtmlWindow, which
// eventually will display a precise version number.
//
// Until selectable text is needed, it would seem sensible to disable
// the primary wxHtmlWindow; but that would make the two wxHtmlWindows
// behave differently.
//
// If both wxHtmlWindows are to behave similarly, then they cannot be
// disabled, because that would prevent scrolling. And at least the
// second wxHtmlWindow must receive initial focus (otherwise, tabbing
// between it and the buttons might look better, but the normal
// keystrokes for scrolling would, astonishingly, fail to work), and,
// for consistency, so does the first.

BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
    EVT_BUTTON(wxID_OK, AboutDialog::OnOK)
END_EVENT_TABLE()

AboutDialog::AboutDialog(wxWindow* parent)
    :wxDialog
        (parent
        ,-1
        ,wxString
            (   "About 'Let me illustrate...' version "
            +   std::string(LMI_VERSION)
            )
        )
{
}

AboutDialog::~AboutDialog()
{
}

int AboutDialog::ShowModal()
{
// WX !! Help for wxHtmlWindow mentions
//   wxHP_SCROLLBAR_AUTO
//   wxHP_NO_SELECTION
// but apparently both should begin 'wxHW'.

// WX !! Style wxHW_NO_SELECTION must be set at creation. Trying to
// change it later, e.g.
//    html_window->SetWindowStyle(html_window->GetWindowStyle() | wxHW_NO_SELECTION);
//    html_window->SetExtraStyle (html_window->GetExtraStyle()  | wxHW_NO_SELECTION);
// doesn't work. Is that reasonable?

    wxHtmlWindow* html_window = new wxHtmlWindow
        (this
        ,-1
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
        ,wxID_OK
        ,"Read the GNU General Public License"
        );
    wxButton* cancel_button = new wxButton
        (this
        ,wxID_CANCEL
        ,"Let me illustrate"
        );
    license_button->SetDefault();

    wxFlexGridSizer* sizer1 = new wxFlexGridSizer(0, 1);
    sizer1->AddGrowableCol(0);
    sizer1->AddGrowableCol(1);
    sizer1->Add(license_button, 1, wxALL|wxALIGN_LEFT , 5);
    sizer1->Add(cancel_button , 1, wxALL|wxALIGN_RIGHT, 5);

    wxFlexGridSizer* sizer0 = new wxFlexGridSizer(1, 0);
    sizer0->AddGrowableRow(0);
    sizer0->Add(html_window, 1, wxALL, 10);
    sizer0->Add(sizer1, 1, wxALL, 10);

    SetAutoLayout(true);
    SetSizer(sizer0);
    sizer0->Fit(this);
    Center();
    return wxDialog::ShowModal();
}

void AboutDialog::OnOK(wxCommandEvent&)
{
    wxDialog dialog(this, -1, wxString("GNU General Public License"));
    wxHtmlWindow* html_window = new wxHtmlWindow
        (&dialog
        ,-1
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
    sizer->Add(html_window, 1, wxALL, 10);
    sizer->Add(button, 0, wxALL|wxALIGN_RIGHT, 10);

    dialog.SetAutoLayout(true);
    dialog.SetSizer(sizer);
    sizer->Fit(&dialog);
    dialog.Center();
    dialog.ShowModal();
}

