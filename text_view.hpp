// Document view for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: text_view.hpp,v 1.13 2008-03-09 12:13:26 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.h (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-view
// class out of a larger file, and in the later years given in the
// copyright notice above.

#ifndef text_view_hpp
#define text_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT

class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

/// This class isn't actually used in production: it's exposed for use
/// only with a special command-line option that enables "experimental
/// or perilous" features. There's no intention to make lmi a general-
/// purpose text editor.
///
/// It's left in cvs, though, because it might be a valuable model for
/// some other feature someday.

class TextEditView
    :public ViewEx
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<TextEditView>
{
    friend class TextEditDocument;

  public:
    TextEditView();
    virtual ~TextEditView();

  private:
    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();
    virtual wxIcon Icon() const;
    virtual wxMenuBar* MenuBar() const;

    wxTextCtrl* text_window_;

    DECLARE_DYNAMIC_CLASS(TextEditView)
};

#endif // text_view_hpp

