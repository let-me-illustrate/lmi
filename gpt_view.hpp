// Document view for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifndef gpt_view_hpp
#define gpt_view_hpp

#include "config.hpp"

#include "mvc_view.hpp"
#include "view_ex.hpp"

#include "obstruct_slicing.hpp"
#include "oecumenic_enumerations.hpp"
#include "uncopyable_lmi.hpp"

#include <wx/event.h>

#include <string>

class gpt_document;
class gpt_input;
class WXDLLIMPEXP_FWD_CORE wxHtmlWindow;

/// MVC View for GPT.
///
/// This class has external linkage so that it can be used to preload
/// XRC resources at startup in order to diagnose their absence early.
///
/// Implicitly-declared special member functions do the right thing.

class gpt_mvc_view
    :public MvcView
{
  public:
    gpt_mvc_view();
    virtual ~gpt_mvc_view();

  private:
    // MvcView required implementation.
    virtual char const* DoBookControlName () const;
    virtual char const* DoMainDialogName  () const;
    virtual char const* DoResourceFileName() const;
};

class gpt_view
    :        public  ViewEx
    ,        private lmi::uncopyable <gpt_view>
    ,virtual private obstruct_slicing<gpt_view>
{
    friend class gpt_document;

  public:
    gpt_view();
    virtual ~gpt_view();

  private:
    gpt_document& document() const;

    oenum_mvc_dv_rc edit_parameters();
    void Run();

    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();
    virtual wxIcon Icon() const;
    virtual wxMenuBar* MenuBar() const;

    // ViewEx overrides.
    virtual bool OnCreate(wxDocument*, long int);

    // wxView overrides.
    virtual wxPrintout* OnCreatePrintout();

    void UponProperties        (wxCommandEvent&);
    void UponUpdateFileSave    (wxUpdateUIEvent&);
    void UponUpdateFileSaveAs  (wxUpdateUIEvent&);
    void UponUpdateInapplicable(wxUpdateUIEvent&);
    void UponUpdateProperties  (wxUpdateUIEvent&);

    gpt_input& input_data();

    std::string html_content_;
    wxHtmlWindow* html_window_;

    DECLARE_DYNAMIC_CLASS(gpt_view)
    DECLARE_EVENT_TABLE()
};

#endif // gpt_view_hpp

