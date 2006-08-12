// Enhanced wxView class.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: view_ex.hpp,v 1.6 2006-08-12 17:16:33 chicares Exp $

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
// types, and indeed protected functions are provided to encapsulate
// that work in this class--with the same return types as the wxxrc
// functions they call. The public functions are pure to allow
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

#include <string>

// TODO ?? Here and elsewhere, consider losing 'WXDLLEXPORT' on
// forward declarations, depending on the disposition of
//   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20345
//
class WXDLLEXPORT wxDC;
class WXDLLEXPORT wxIcon;
class WXDLLEXPORT wxMenuBar;

class DocManagerEx;

// INELEGANT !! If both a base and a derived class derive nonvirtually
// from boost::noncopyable
//    ,private boost::noncopyable
// then gcc will complain
//   warning: direct base `boost::noncopyable' inaccessible in
//   [derived classes] due to ambiguity
// Cacciola's noncopyable<T> solution here
//   http://lists.boost.org/MailArchives/boost/msg17485.php
//   http://lists.boost.org/MailArchives/boost/msg17012.php
// is best; no good reason for dismissing it was given on the boost
// mailing list, but presumably they didn't want to require typing
// the template argument. The workaround used here is rightfully
// criticized by Cacciola:
//   http://lists.boost.org/MailArchives/boost/msg17491.php
// That criticism matters little in this particular case, but perhaps
// it would be better to use Cacciola's idea instead of boost's.

class ViewEx
    :public wxView
    ,virtual private boost::noncopyable
{
  public:
    ViewEx();

    virtual wxWindow* CreateChildWindow() = 0;

    virtual wxIcon Icon() const = 0;
    virtual wxMenuBar* MenuBar() const = 0;

  protected:
    virtual ~ViewEx();

    wxIcon IconFromXmlResource(char const*) const;
    wxMenuBar* MenuBarFromXmlResource(char const*) const;

    wxFrame& FrameWindow() const;

    std::string base_filename() const;

    std::string serial_filename
        (int                serial_number
        ,std::string const& extension
        ) const;

    // TODO ?? Probably better to provide the implementation in
    // separate functions that can be called by derived classes.
    //
    // wxView overrides.
    virtual bool OnClose(bool delete_window);
    virtual bool OnCreate(wxDocument* doc, long int flags);
    virtual void OnDraw(wxDC*);

  private:
    DocManagerEx& DocManager() const;

    DECLARE_ABSTRACT_CLASS(ViewEx)
};

#endif // view_ex_hpp

