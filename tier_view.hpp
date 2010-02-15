// Stratified charges manager.
//
// Copyright (C) 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef tier_view_hpp
#define tier_view_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include "obstruct_slicing.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

class TierDocument;
class TierTableAdapter;
class WXDLLIMPEXP_FWD_CORE wxTreeEvent;

class TierView
    :public TreeGridViewBase
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<TierView>
{
  public:
    TierView();
    virtual ~TierView();

  private:
    // ViewEx required implementation.
    virtual wxIcon Icon() const;
    virtual wxMenuBar* MenuBar() const;

    // ProductEditorView required implementation.
    virtual bool IsModified() const;
    virtual void DiscardEdits();

    // TreeGridViewBase required implementation.
    virtual wxTreeCtrl*   CreateTreeCtrl(wxWindow* parent);
    virtual MultiDimGrid* CreateGridCtrl(wxWindow* parent);
    virtual void SetupControls();

    TierDocument& document() const;

    void UponTreeSelectionChange(wxTreeEvent&);

    boost::shared_ptr<TierTableAdapter> table_adapter_;

    DECLARE_DYNAMIC_CLASS(TierView)
    DECLARE_EVENT_TABLE()
};

#endif // tier_view_hpp

