// Document view for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: text_view.cpp,v 1.8 2006-03-09 01:58:18 chicares Exp $

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
    EVT_MENU(XRCID("test_app_status_alert"         ),TextEditView::UponTestAppStatus)
    EVT_MENU(XRCID("test_app_warning_alert"        ),TextEditView::UponTestAppWarning)
    EVT_MENU(XRCID("test_app_hobsons_choice_alert" ),TextEditView::UponTestAppHobsons)
    EVT_MENU(XRCID("test_app_fatal_error_alert"    ),TextEditView::UponTestAppFatal)
    EVT_MENU(XRCID("test_app_standard_exception"   ),TextEditView::UponTestAppStandardException)
    EVT_MENU(XRCID("test_app_arbitrary_exception"  ),TextEditView::UponTestAppArbitraryException)
    EVT_MENU(XRCID("test_lib_status_alert"         ),TextEditView::UponTestLibStatus)
    EVT_MENU(XRCID("test_lib_warning_alert"        ),TextEditView::UponTestLibWarning)
    EVT_MENU(XRCID("test_lib_hobsons_choice_alert" ),TextEditView::UponTestLibHobsons)
    EVT_MENU(XRCID("test_lib_fatal_error_alert"    ),TextEditView::UponTestLibFatal)
    EVT_MENU(XRCID("test_lib_standard_exception"   ),TextEditView::UponTestLibStandardException)
    EVT_MENU(XRCID("test_lib_arbitrary_exception"  ),TextEditView::UponTestLibArbitraryException)
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

void TextEditView::UponTestAppStatus(wxCommandEvent&)
{
    status()         << "Test status()"         << LMI_FLUSH;
}

void TextEditView::UponTestAppWarning(wxCommandEvent&)
{
    warning()        << "Test warning()"        << LMI_FLUSH;
}

void TextEditView::UponTestAppHobsons(wxCommandEvent&)
{
    hobsons_choice() << "Test hobsons_choice()" << LMI_FLUSH;
}

void TextEditView::UponTestAppFatal(wxCommandEvent&)
{
    fatal_error()    << "Test fatal_error()"    << LMI_FLUSH;
}

void TextEditView::UponTestAppStandardException(wxCommandEvent&)
{
    throw std::runtime_error("Test a standard exception.");
}

void TextEditView::UponTestAppArbitraryException(wxCommandEvent&)
{
    throw "Test an arbitrary exception.";
}

void TextEditView::UponTestLibStatus(wxCommandEvent&)
{
    test_status();
}

void TextEditView::UponTestLibWarning(wxCommandEvent&)
{
    test_warning();
}

void TextEditView::UponTestLibHobsons(wxCommandEvent&)
{
    test_hobsons_choice();
}

void TextEditView::UponTestLibFatal(wxCommandEvent&)
{
    test_fatal_error();
}

void TextEditView::UponTestLibStandardException(wxCommandEvent&)
{
    test_standard_exception();
}

void TextEditView::UponTestLibArbitraryException(wxCommandEvent&)
{
    test_arbitrary_exception();
}

