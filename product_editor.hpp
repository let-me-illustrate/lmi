// Common code used in various product editor document/view classes.
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

#ifndef product_editor_hpp
#define product_editor_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include <wx/docview.h>

#include <string>

class MultiDimGrid;
class ProductEditorView;
class WXDLLIMPEXP_FWD_CORE wxStaticText;
class WXDLLIMPEXP_FWD_CORE wxTreeCtrl;
class WXDLLIMPEXP_FWD_CORE wxWindow;

/// Common base for all product editor document classes.
/// It transfers some responsabilities of a document to the corresponding view
/// class, since it makes more sense due to the data actually being managed
/// by the view portion of the document/view classes pair.

class ProductEditorDocument
    :public wxDocument
{
  public:
    ProductEditorDocument() = default;
    ~ProductEditorDocument() override = default;

  protected:
    virtual void ReadDocument (std::string const& filename) = 0;
    virtual void WriteDocument(std::string const& filename) = 0;

  private:
    ProductEditorDocument(ProductEditorDocument const&) = delete;
    ProductEditorDocument& operator=(ProductEditorDocument const&) = delete;

    ProductEditorView& PredominantView() const;

    // wxDocument overrides.
    bool IsModified() const override;
    void Modify(bool modified) override;
    bool DoOpenDocument(wxString const& filename) override;
    bool DoSaveDocument(wxString const& filename) override;
};

/// Common base for all product editor view classes.

class ProductEditorView
    :public ViewEx
{
    friend class ProductEditorDocument;

  public:
    ProductEditorView() = default;
    ~ProductEditorView() override = default;

  protected:
    virtual bool IsModified() const = 0;
    virtual void DiscardEdits() = 0;

  private:
    ProductEditorView(ProductEditorView const&) = delete;
    ProductEditorView& operator=(ProductEditorView const&) = delete;
};

/// This class is a common base for TierView and DatabaseView classes.
/// It contains and lays out common widgets.

class TreeGridViewBase
    :public ProductEditorView
{
  public:
    TreeGridViewBase() = default;
    ~TreeGridViewBase() override = default;

  protected:
    MultiDimGrid& grid() const;
    wxTreeCtrl&   tree() const;

    void set_grid_label_text(std::string const&);

    // Returned objects must be constructed as children of 'parent', so that
    // the returned objects ownership is managed by wx.
    virtual MultiDimGrid* CreateGridCtrl(wxWindow* parent) = 0;
    virtual wxTreeCtrl*   CreateTreeCtrl(wxWindow* parent) = 0;

    virtual void SetupControls() = 0;

  private:
    TreeGridViewBase(TreeGridViewBase const&) = delete;
    TreeGridViewBase& operator=(TreeGridViewBase const&) = delete;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;

    // These objects are held by pointer since the destruction is taken care
    // of by wx.
    MultiDimGrid* grid_       {nullptr};
    wxStaticText* grid_label_ {nullptr};
    wxTreeCtrl*   tree_       {nullptr};
};

#endif // product_editor_hpp
