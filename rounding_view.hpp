// Rounding rules manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef rounding_view_hpp
#define rounding_view_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include <map>

class RoundingButtons;
class RoundingDocument;

class RoundingView final
    :public ProductEditorView
{
  public:
    RoundingView() = default;
    ~RoundingView() override = default;

    typedef std::map<std::string, RoundingButtons*> controls_type;
    controls_type&       controls();
    controls_type const& controls() const;

    static char const* resource_file_name();

  private:
    RoundingView(RoundingView const&) = delete;
    RoundingView& operator=(RoundingView const&) = delete;

    RoundingDocument& document() const;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // ProductEditorView required implementation.
    bool IsModified() const override;
    void DiscardEdits() override;

    controls_type controls_;

    DECLARE_DYNAMIC_CLASS(RoundingView)
};

inline RoundingView::controls_type& RoundingView::controls()
{
    return controls_;
}

inline RoundingView::controls_type const& RoundingView::controls() const
{
    return controls_;
}

inline char const* RoundingView::resource_file_name()
{
    return "rounding_view.xrc";
}

#endif // rounding_view_hpp
