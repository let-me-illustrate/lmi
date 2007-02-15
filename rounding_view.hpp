// Rounding rules manager.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: rounding_view.hpp,v 1.1.2.2 2007-02-15 14:42:18 etarassov Exp $

#ifndef rounding_view_hpp
#define rounding_view_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "product_editor.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <wx/icon.h>

#include <map>

class WXDLLEXPORT wxMenuBar;
class WXDLLEXPORT wxSpinCtrl;
class WXDLLEXPORT wxWindow;

// forward declaration
class RoundingButtons;
class RoundingDocument;

class RoundingView
    :public ProductEditorView
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<RoundingView>
{
  public:
    RoundingView();
    virtual ~RoundingView();

    typedef std::map<std::string, RoundingButtons*> controls_type;
    controls_type&       controls();
    controls_type const& controls() const;

    static char const* resource_file_name();

  private:
    RoundingDocument& document() const;

    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();
    virtual wxIcon Icon() const;
    virtual wxMenuBar* MenuBar() const;
    // ProductEditorView required implementation.
    virtual bool IsModified() const;
    virtual void DiscardEdits();

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

