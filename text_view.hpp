// Document view for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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
#include "uncopyable_lmi.hpp"

#include <wx/defs.h> // wx shared-library 'attributes'

class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

/// This class isn't actually used in production: it's exposed for use
/// only with a special command-line option that enables "experimental
/// or perilous" features. There's no intention to make lmi a general-
/// purpose text editor.
///
/// It's left in the repository, though, because it might be a useful
/// model for some other feature someday.

class TextEditView
    :        public  ViewEx
    ,        private lmi::uncopyable <TextEditView>
    ,virtual private obstruct_slicing<TextEditView>
{
    friend class TextEditDocument;

  public:
    TextEditView();
    virtual ~TextEditView();

  private:
    // ViewEx required implementation.
    virtual bool CanBePrinted() const { return false; }
    virtual wxWindow* CreateChildWindow();
    virtual char const* icon_xrc_resource   () const;
    virtual char const* menubar_xrc_resource() const;

    wxTextCtrl* text_window_;

    DECLARE_DYNAMIC_CLASS(TextEditView)
};

#endif // text_view_hpp

