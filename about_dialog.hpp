// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#ifndef about_dialog_hpp
#define about_dialog_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <wx/dialog.h>

/// Implementation notes: class AboutDialog.
///
/// The 'about' dialog displays a wxHtmlWindow, and the license is
/// optionally shown in another wxHtmlWindow. The first does not need
/// scrolling; the second does. How should those windows interact with
/// the keyboard for scrolling and text selection?
///
/// Perhaps it would be ideal to let the html text be selected, and
/// the selection be copied, at least for the primary wxHtmlWindow,
/// which eventually will display a precise version number.
///
/// Until selectable text is needed, it would seem sensible to disable
/// the primary wxHtmlWindow; but that would make the two wxHtmlWindow
/// objects behave differently.
///
/// If both wxHtmlWindows are to behave similarly, then they cannot be
/// disabled, because that would prevent scrolling. And at least the
/// second wxHtmlWindow must receive initial focus (otherwise, tabbing
/// between it and the buttons might look better, but the normal
/// keystrokes for scrolling would, astonishingly, fail to work), and,
/// for consistency, so does the first.

class AboutDialog
    :public  wxDialog
    ,private lmi::uncopyable<AboutDialog>
{
  public:
    AboutDialog(wxWindow* parent);
    virtual ~AboutDialog();

    // wxDialog overrides.
    virtual int ShowModal();

  private:
    void UponReadLicense(wxCommandEvent&);

    DECLARE_EVENT_TABLE()
};

#endif // about_dialog_hpp

