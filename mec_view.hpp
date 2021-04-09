// Document view for MEC testing.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef mec_view_hpp
#define mec_view_hpp

#include "config.hpp"

#include "mvc_view.hpp"
#include "view_ex.hpp"

#include "oecumenic_enumerations.hpp"

#include <wx/event.h>

#include <string>

class mec_document;
class mec_input;
class WXDLLIMPEXP_FWD_CORE wxHtmlWindow;

/// MVC View for MEC testing.
///
/// This class has external linkage so that it can be used to preload
/// XRC resources at startup in order to diagnose their absence early.
///
/// Implicitly-declared special member functions do the right thing.

class mec_mvc_view
    :public MvcView
{
  public:
    mec_mvc_view() = default;
    ~mec_mvc_view() override = default;

  private:
    // MvcView required implementation.
    char const* DoBookControlName () const override;
    char const* DoMainDialogName  () const override;
    char const* DoResourceFileName() const override;
};

class mec_view final
    :public ViewEx
{
    friend class mec_document;

  public:
    mec_view() = default;
    ~mec_view() override = default;

  private:
    mec_view(mec_view const&) = delete;
    mec_view& operator=(mec_view const&) = delete;

    mec_document& document() const;

    oenum_mvc_dv_rc edit_parameters();
    void Run();

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // ViewEx overrides.
    bool OnCreate(wxDocument*, long int) override;

    // wxView overrides.
    wxPrintout* OnCreatePrintout() override;

    void UponProperties        (wxCommandEvent&);
    void UponUpdateFileSave    (wxUpdateUIEvent&);
    void UponUpdateFileSaveAs  (wxUpdateUIEvent&);
    void UponUpdateInapplicable(wxUpdateUIEvent&);
    void UponUpdateProperties  (wxUpdateUIEvent&);

    mec_input& input_data();

    std::string html_content_  {"Unable to display results."};
    wxHtmlWindow* html_window_ {nullptr};

    DECLARE_DYNAMIC_CLASS(mec_view)
    DECLARE_EVENT_TABLE()
};

#endif // mec_view_hpp
