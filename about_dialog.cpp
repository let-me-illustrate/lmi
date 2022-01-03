// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

/// This virtual function calls its base-class namesake explicitly.

int AboutDialog::ShowModal()
{
    wxHtmlWindow* html_window = new(wx) wxHtmlWindow
        (this
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxHW_SCROLLBAR_NEVER | wxHW_NO_SELECTION
        );
    html_window->SetBorders(0);
    html_window->SetPage(license_notices_as_html());
    html_window->GetInternalRepresentation()->Layout(65 * html_window->GetCharWidth());
    int const width  = html_window->GetInternalRepresentation()->GetWidth ();
    int const height = html_window->GetInternalRepresentation()->GetHeight();
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

    auto const borderFlags = wxSizerFlags().Border(wxALL, 3);

    wxBoxSizer* sizer1 = new(wx) wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(license_button, borderFlags);
    sizer1->Add(cancel_button , borderFlags);

    wxBoxSizer* sizer0 = new(wx) wxBoxSizer(wxVERTICAL);
    sizer0->Add(html_window, wxSizerFlags().Proportion(1)     );
    sizer0->Add(sizer1     , wxSizerFlags(borderFlags).Right());

    SetSizerAndFit(sizer0);
    Center();
    return wxDialog::ShowModal();
}

void AboutDialog::UponReadLicense(wxCommandEvent&)
{
    wxDialog dialog
        (this
        ,wxID_ANY
        ,std::string("GNU General Public License")
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX
        );
    wxHtmlWindow* html_window = new(wx) wxHtmlWindow
        (&dialog
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION
        );
    html_window->SetBorders(0);
    html_window->SetPage(license_as_html());

    wxButton* button = new(wx) wxButton(&dialog, wxID_CANCEL, "Close");
    button->SetDefault();

    wxBoxSizer* sizer = new(wx) wxBoxSizer(wxVERTICAL);
    sizer->Add(html_window, wxSizerFlags().Proportion(1).Expand()  );
    sizer->Add(button     , wxSizerFlags().Right().Border(wxALL, 6));
    dialog.SetSizerAndFit(sizer);

    wxRect r = wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea();
    int const minimum_width  = 60 * dialog.GetCharWidth();
    int const default_width  = r.GetWidth () * 4 / 5;
    int const default_height = r.GetHeight() * 4 / 5;
    dialog.SetInitialSize(wxSize(minimum_width, default_height));
    dialog.SetSize       (wxSize(default_width, default_height));
    dialog.Center();
    dialog.ShowModal();
}
