// Product data manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef policy_view_hpp
#define policy_view_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include <map>
#include <string>

class PolicyDocument;
class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

class PolicyView final
    :public ProductEditorView
{
  public:
    PolicyView() = default;
    ~PolicyView() override = default;

    typedef std::map<std::string, wxTextCtrl*> controls_type;
    controls_type&       controls();
    controls_type const& controls() const;

    static char const* resource_file_name();

  private:
    PolicyView(PolicyView const&) = delete;
    PolicyView& operator=(PolicyView const&) = delete;

    PolicyDocument& document() const;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // ProductEditorView required implementation.
    bool IsModified() const override;
    void DiscardEdits() override;

    controls_type controls_;

    DECLARE_DYNAMIC_CLASS(PolicyView)
};

inline PolicyView::controls_type& PolicyView::controls()
{
    return controls_;
}

inline PolicyView::controls_type const& PolicyView::controls() const
{
    return controls_;
}

inline char const* PolicyView::resource_file_name()
{
    return "policy_view.xrc";
}

#endif // policy_view_hpp
