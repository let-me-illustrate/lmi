// Enhanced wxView class.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: view_ex.hpp,v 1.1 2005-03-11 03:09:22 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2004 to create an abstract
// enhanced view class, and in the later years given in the copyright
// notice above.

// Enhance class wxView to encapsulate things that are naturally
// associated with each concrete derived class and implement common
// behavior that is assumed to be generally appropriate.

// Clients can use any child window type in CreateChildWindow(). That
// type could instead have been made a template parameter of this
// class. That design alternative would seem preferable if all child
// windows were created the same way, or if the child-window type were
// otherwise useful in this class, but neither of those conditions
// seems to obtain.

// Icon() and MenuBar() return the types wanted by wx functions
// SetIcon() and SetMenuBar() respectively. Because these things are
// expected to be loaded from xml resources, the functions could have
// been made nonvirtual, with a string argument to distinguish view
// types, and indeed protected functions are provided to encapsulate
// that work in this class--with the same return types as the wxxrc
// functions they call. The public functions are pure to allow
// implementation flexibility. For instance, an overrider may contain
// a static variable to cache xml resources for better responsiveness,
// or it may use a different approach altogether. It seems that wx
// wants icons to be in resource files on the msw platform, and indeed
// the wxxrc sample program doesn't use xml resources for icons, so
// IconFromXmlResource() may be of little use.

// OnCreate() and OnClose() are implemented here because it is not
// anticipated that their essential behavior would differ in client
// classes. Small differences can be factored into pure functions:
// for instance, OnCreate() calls CreateChildWindow().

// TODO ?? Revise the preceding comment.

// OnDraw() is pure in base class wxView. An (empty) implementation is
// supplied here because no view class in the application this is
// designed for actually needs to implement it.

// FrameWindow() is a convenience function that returns a reference to
// a wxFrame. The library provides a similar function, GetFrame(),
// which is less useful because it returns a wxWindow*, even though
// the wx documentation says it returns a wxFrame*. This function is
// used only in the implementation of this class and does not require
// a corresponding const version.

// TODO ?? No provision is yet made here for updating views when
// document data changes; wx provides some functions for that.

#ifndef view_ex_hpp
#define view_ex_hpp

#include "config.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT
#include <wx/docview.h>

// TODO ?? Here and elsewhere, consider losing 'WXDLLEXPORT' on
// forward declarations, depending on the disposition of
//   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20345
//
class WXDLLEXPORT wxDC;
class WXDLLEXPORT wxIcon;
class WXDLLEXPORT wxMenuBar;

class DocManagerEx;

// TODO ?? Here,
//    ,virtual private boost::noncopyable
// would elicit
//   warning: direct base `boost::noncopyable' inaccessible in
//   [derived classes] due to ambiguity
// from gcc. Consider these messages
//   http://lists.boost.org/MailArchives/boost/msg17474.php
//   http://lists.boost.org/MailArchives/boost/msg17485.php
//   http://lists.boost.org/MailArchives/boost/msg17491.php
// and write a general style rule to encompass this case.

class ViewEx
    :public wxView
    ,virtual private boost::noncopyable
{
  public:
    ViewEx();

    virtual wxWindow* CreateChildWindow() = 0;

    virtual wxIcon const& Icon() const = 0;
    virtual wxMenuBar* MenuBar() const = 0;

  protected:
    virtual ~ViewEx();

    wxIcon IconFromXmlResource(char const*) const;
    wxMenuBar* MenuBarFromXmlResource(char const*) const;

    wxFrame& FrameWindow() const;

    // TODO ?? Probably better to provide the implementation in
    // separate functions that can be called by derived classes.
    //
    // wxView overrides.
    virtual bool OnClose(bool delete_window);
    virtual bool OnCreate(wxDocument* doc, long flags);
    virtual void OnDraw(wxDC*);

  private:
    DocManagerEx& DocManager() const;

    DECLARE_ABSTRACT_CLASS(ViewEx)
};

#endif // view_ex_hpp

