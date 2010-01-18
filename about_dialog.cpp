// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "about_dialog.hpp"

#include "license.hpp"
#include "version.hpp"
#include "wx_new.hpp"

#include <wx/button.h>
#include <wx/display.h>
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
    wxHtmlWindow* html_window = new(wx) wxHtmlWindow
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
    html_window->SetMinSize(wxSize(width, height));

    wxButton* license_button = new(wx) wxButton
        (this
        ,wxID_ABOUT
        ,"Read the GNU General Public License"
        );
    license_button->SetDefault();
    wxButton* cancel_button = new(wx) wxButton
        (this
        ,wxID_CANCEL
        ,"Let me illustrate"
        );

    wxBoxSizer* sizer1 = new(wx) wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(license_button, 0, wxALL, 3);
    sizer1->Add(cancel_button , 0, wxALL, 3);

    wxBoxSizer* sizer0 = new(wx) wxBoxSizer(wxVERTICAL);
    sizer0->Add(html_window, 1, wxALL                , 0);
    sizer0->Add(sizer1     , 0, wxALL | wxALIGN_RIGHT, 3);

    SetSizerAndFit(sizer0);
    Center();
    return wxDialog::ShowModal();
}

void AboutDialog::UponReadLicense(wxCommandEvent&)
{
    wxDialog dialog(this, wxID_ANY, std::string("GNU General Public License"));
    wxHtmlWindow* html_window = new(wx) wxHtmlWindow
        (&dialog
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION
        );
    html_window->SetBorders(0);
    html_window->SetPage(license_as_html());
    html_window->GetInternalRepresentation()->Layout(1);

    wxRect r = wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea();
    // Using the whole client area would seem unnatural. Pushbuttons
    // can't plausibly take more than twenty percent of the vertical
    // space.
    int width  = r.GetWidth () * 4 / 5;
    int height = r.GetHeight() * 4 / 5;
    html_window->SetMinSize(wxSize(width, height));

    wxButton* button = new(wx) wxButton(&dialog, wxID_CANCEL, "Close");
    button->SetDefault();

    wxBoxSizer* sizer = new(wx) wxBoxSizer(wxVERTICAL);
    sizer->Add(html_window, 1, wxALL                , 0);
    sizer->Add(button     , 0, wxALL | wxALIGN_RIGHT, 6);

    dialog.SetSizerAndFit(sizer);
    dialog.Center();
    dialog.ShowModal();
}

