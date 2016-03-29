// Customize implementation details of library class wxDocMDIChildFrame.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

// Enhance class wxDocMDIChildFrame to support MDI without requiring
// the parent MDI frame window to display a 'Window' menu when it has
// no MDI child, as recommended in msw platform documentation. This
// implementation is rather specific to that platform.

// The wx implementation of OnMenuHighlight() assumes that MDI child
// frames have statusbars, but that practice is uncommon, at least for
// msw, where typically only the MDI parent frame has a statusbar.
// Instead of copying implementation details of OnMenuHighlight()
// here, GetStatusBar() is overridden to change its behavior when it
// is called in the menu-highlighting context. The new behavior shunts
// menu highlighting to the parent frame's statusbar iff the child
// frame has none. 'status_bar_sought_from_menu_highlight_handler_' is
// a mode flag used to accomplish this. A more straightforward patch
// to the library could be devised.

// WX !! Even without these changes, handling enablement of child-frame
// menuitems takes a bit of work. For example, change 'samples/mdi' by
// adding EVT_MENU_OPEN handlers for both parent and child that call
//   event.GetMenu()
//   event.GetEventObject()
// and report their values. All menus on the menubar report the parent
// as the event object, but the 'window menu' and child menus return
// null from GetMenu(). The child's system menu, however, reports the
// child as its event object. Now add EVT_UPDATE_UI handlers for the
// MDI_ABOUT item, for both parent and child: they are triggered only
// by activating the respective system menu. Alternatively, add
// EVT_UPDATE_UI handlers for MDI_CHILD_QUIT: only the child's handler
// is triggered, and only when its system menu is activated. None of
// these EVT_UPDATE_UI handlers can be triggered by manipulating the
// menubars that own the menuitems corresponding to these events. At
// least in these cases, the wx documentation for wxUpdateUIEvent:
//   "On Windows and GTK+, events for menubar items are only sent when
//   the menu is about to be shown, and not in idle time"
// could be clarified.
//
// It seems satisfactory, however, to catch EVT_MENU_OPEN, in the
// parent class only, with a handler that manages either parent or
// child menuitems depending on whether GetActiveChild() returns null.

#ifndef docmdichildframe_ex_hpp
#define docmdichildframe_ex_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <wx/docmdi.h>

class DocMDIChildFrameEx
    :public  wxDocMDIChildFrame
    ,private lmi::uncopyable<DocMDIChildFrameEx>
{
  public:
    DocMDIChildFrameEx
        (wxDocument*       doc
        ,wxView*           view
        ,wxMDIParentFrame* parent
        ,wxWindowID        id     = wxID_ANY
        ,wxString   const& title  = "Loading..."
        ,wxPoint    const& pos    = wxDefaultPosition
        ,wxSize     const& size   = wxDefaultSize
        ,long int          style  = wxDEFAULT_FRAME_STYLE
        ,wxString   const& name   = "child frame"
        );

  private:
    DECLARE_CLASS(DocMDIChildFrameEx)
    DECLARE_EVENT_TABLE()
};

#endif // docmdichildframe_ex_hpp

