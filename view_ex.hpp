// Enhanced wxView class.
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

// Icon() and MenuBar() return types suitable for passing to wx
// functions SetIcon() and SetMenuBar() respectively. Because they are
// expected to be loaded from xml resources, the functions could have
// been made nonvirtual, with a string argument to distinguish view
// types, and indeed protected functions IconFromXmlResource() and
// MenuBarFromXmlResource() are provided to encapsulate that work in
// this class--with the same return types as the wxxrc functions they
// call. Thus, notably, MenuBarFromXmlResource() returns a pointer,
// but it never returns a null pointer, which would cause the program
// to crash--instead, it returns a default object if no menubar can be
// read from an xml resource. The public functions are pure to allow
// implementation flexibility. For instance, an overrider may contain
// a static variable to cache xml resources for better responsiveness,
// or it may use a different approach altogether.

// OnCreate() and OnClose() are implemented here because it is not
// anticipated that their essential behavior would differ in client
// classes. Small differences can be factored into pure functions:
// for instance, OnCreate() calls CreateChildWindow().

// TODO ?? Revise the preceding comment.

// OnDraw() is pure in base class wxView. An (empty) implementation is
// supplied here because no view class in the application this is
// designed for actually needs to implement it.

// FrameWindow() is a convenience function that returns a reference to
// a wxFrame. Member functions specific to wxFrame cannot be called
// through wxView::GetFrame() without casting because it returns a
// wxWindow*.

#ifndef view_ex_hpp
#define view_ex_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <wx/docview.h>

#include <string>

class DocManagerEx;

class WXDLLIMPEXP_FWD_CORE wxDC;
class WXDLLIMPEXP_FWD_CORE wxIcon;
class WXDLLIMPEXP_FWD_CORE wxMenuBar;

class ViewEx
    :public  wxView
    ,private lmi::uncopyable<ViewEx>
{
  public:
    ViewEx();

    // TODO ?? Consider making virtuals nonpublic and public functions
    // nonvirtual.
    virtual wxIcon Icon() const = 0;
    virtual wxMenuBar* MenuBar() const = 0;

  protected:
    virtual ~ViewEx();

    wxIcon IconFromXmlResource(char const*) const;
    wxMenuBar* MenuBarFromXmlResource(char const*) const;

    wxFrame& FrameWindow() const;

    std::string base_filename() const;

    // TODO ?? Probably better to provide the implementation in
    // separate functions that can be called by derived classes.
    //
    // wxView overrides.
    virtual bool OnClose(bool delete_window);
    virtual bool OnCreate(wxDocument* doc, long int flags);
    virtual void OnDraw(wxDC*);

  private:
    virtual wxWindow* CreateChildWindow() = 0;

    DocManagerEx& DocManager() const;

    DECLARE_ABSTRACT_CLASS(ViewEx)
};

#endif // view_ex_hpp

