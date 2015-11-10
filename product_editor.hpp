// Common code used in various product editor document/view classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifndef product_editor_hpp
#define product_editor_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "uncopyable_lmi.hpp"

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
    :public  wxDocument
    ,private lmi::uncopyable<ProductEditorDocument>
{
  public:
    ProductEditorDocument();
    virtual ~ProductEditorDocument();

  protected:
    virtual void ReadDocument (std::string const& filename) = 0;
    virtual void WriteDocument(std::string const& filename) = 0;

  private:
    ProductEditorView& PredominantView() const;

    // wxDocument overrides.
    virtual bool IsModified() const;
    virtual void Modify(bool modified);
    virtual bool DoOpenDocument(wxString const& filename);
    virtual bool DoSaveDocument(wxString const& filename);
};

/// Common base for all product editor view classes.

class ProductEditorView
    :public  ViewEx
    ,private lmi::uncopyable<ProductEditorView>
{
    friend class ProductEditorDocument;

  public:
    ProductEditorView();
    virtual ~ProductEditorView();

    // None of the product documents is currently printable, so provide a
    // common implementation of this ViewEx method for all our subclasses.
    virtual bool CanBePrinted() const { return false; }

  protected:
    virtual bool IsModified() const = 0;
    virtual void DiscardEdits() = 0;
};

/// This class is a common base for TierView and DatabaseView classes.
/// It contains and lays out common widgets.

class TreeGridViewBase
    :public  ProductEditorView
    ,private lmi::uncopyable<TreeGridViewBase>
{
  public:
    TreeGridViewBase();
    virtual ~TreeGridViewBase();

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
    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();

    // These objects are held by pointer since the destruction is taken care
    // of by wx.
    MultiDimGrid* grid_;
    wxStaticText* grid_label_;
    wxTreeCtrl*   tree_;
};

#endif // product_editor_hpp

