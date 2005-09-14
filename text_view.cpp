// Document view for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: text_view.cpp,v 1.6 2005-09-14 14:16:30 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-view
// class out of a larger file, and in the later years given in the
// copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "text_view.hpp"

#include "alert.hpp"
#include "wx_new.hpp"

#include <wx/icon.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include <stdexcept>

IMPLEMENT_DYNAMIC_CLASS(TextEditView, ViewEx)

BEGIN_EVENT_TABLE(TextEditView, ViewEx)
    EVT_MENU(XRCID("test_app_status_alert"         ),TextEditView::OnTestAppStatus)
    EVT_MENU(XRCID("test_app_warning_alert"        ),TextEditView::OnTestAppWarning)
    EVT_MENU(XRCID("test_app_hobsons_choice_alert" ),TextEditView::OnTestAppHobsons)
    EVT_MENU(XRCID("test_app_fatal_error_alert"    ),TextEditView::OnTestAppFatal)
    EVT_MENU(XRCID("test_app_standard_exception"   ),TextEditView::OnTestAppStandardException)
    EVT_MENU(XRCID("test_app_arbitrary_exception"  ),TextEditView::OnTestAppArbitraryException)
    EVT_MENU(XRCID("test_lib_status_alert"         ),TextEditView::OnTestLibStatus)
    EVT_MENU(XRCID("test_lib_warning_alert"        ),TextEditView::OnTestLibWarning)
    EVT_MENU(XRCID("test_lib_hobsons_choice_alert" ),TextEditView::OnTestLibHobsons)
    EVT_MENU(XRCID("test_lib_fatal_error_alert"    ),TextEditView::OnTestLibFatal)
    EVT_MENU(XRCID("test_lib_standard_exception"   ),TextEditView::OnTestLibStandardException)
    EVT_MENU(XRCID("test_lib_arbitrary_exception"  ),TextEditView::OnTestLibArbitraryException)
END_EVENT_TABLE()

TextEditView::TextEditView()
    :ViewEx      ()
    ,text_window_(0)
{
}

TextEditView::~TextEditView()
{
}

wxWindow* TextEditView::CreateChildWindow()
{
    return text_window_ = new(wx) wxTextCtrl
        (GetFrame()     // Parent.
        ,-1             // Window ID.
        ,""             // Default contents.
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTE_MULTILINE // Style.
        );
}

wxIcon TextEditView::Icon() const
{
    return IconFromXmlResource("text_view_icon");
}

wxMenuBar* TextEditView::MenuBar() const
{
    return MenuBarFromXmlResource("text_view_menu");
}

void TextEditView::OnTestAppStatus(wxCommandEvent&)
{
    status()         << "Test status()"         << LMI_FLUSH;
}

void TextEditView::OnTestAppWarning(wxCommandEvent&)
{
    warning()        << "Test warning()"        << LMI_FLUSH;
}

void TextEditView::OnTestAppHobsons(wxCommandEvent&)
{
    hobsons_choice() << "Test hobsons_choice()" << LMI_FLUSH;
}

void TextEditView::OnTestAppFatal(wxCommandEvent&)
{
    fatal_error()    << "Test fatal_error()"    << LMI_FLUSH;
}

void TextEditView::OnTestAppStandardException(wxCommandEvent&)
{
    throw std::runtime_error("Test a standard exception.");
}

void TextEditView::OnTestAppArbitraryException(wxCommandEvent&)
{
    throw "Test an arbitrary exception.";
}

void TextEditView::OnTestLibStatus(wxCommandEvent&)
{
    test_status();
}

void TextEditView::OnTestLibWarning(wxCommandEvent&)
{
    test_warning();
}

void TextEditView::OnTestLibHobsons(wxCommandEvent&)
{
    test_hobsons_choice();
}

void TextEditView::OnTestLibFatal(wxCommandEvent&)
{
    test_fatal_error();
}

void TextEditView::OnTestLibStandardException(wxCommandEvent&)
{
    test_standard_exception();
}

void TextEditView::OnTestLibArbitraryException(wxCommandEvent&)
{
    test_arbitrary_exception();
}

