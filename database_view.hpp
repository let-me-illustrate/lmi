// Database dictionary manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef database_view_hpp
#define database_view_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include <memory>                       // shared_ptr

class DatabaseDocument;
class DatabaseTableAdapter;
class WXDLLIMPEXP_FWD_CORE wxTreeEvent;

class DatabaseView final
    :public TreeGridViewBase
{
  public:
    DatabaseView();
    ~DatabaseView() override = default;

  private:
    DatabaseView(DatabaseView const&) = delete;
    DatabaseView& operator=(DatabaseView const&) = delete;

    // ViewEx required implementation.
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // ProductEditorView required implementation.
    bool IsModified() const override;
    void DiscardEdits() override;

    // TreeGridViewBase required implementation.
    wxTreeCtrl*   CreateTreeCtrl(wxWindow* parent) override;
    MultiDimGrid* CreateGridCtrl(wxWindow* parent) override;
    void          SetupControls() override;

    DatabaseDocument&           document() const;
    DatabaseTableAdapter      & table_adapter()      ;
    DatabaseTableAdapter const& table_adapter() const;

    void UponTreeSelectionChange(wxTreeEvent&);

    std::shared_ptr<DatabaseTableAdapter> const table_adapter_;

    DECLARE_DYNAMIC_CLASS(DatabaseView)
    DECLARE_EVENT_TABLE()
};

#endif // database_view_hpp
