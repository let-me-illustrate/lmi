// Common code used in various product editor document/view classes.
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

// $Id: product_editor.hpp,v 1.3 2007-02-25 19:28:18 chicares Exp $

#ifndef profile_editor_hpp
#define profile_editor_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include <boost/utility.hpp>

#include <wx/docview.h>

// forward declaration
class ProductEditorView;
class MultiDimGrid;

class WXDLLEXPORT wxStaticText;
class WXDLLEXPORT wxTreeCtrl;
class WXDLLEXPORT wxWindow;

/// Common base for all product editor document classes.
/// It transfers some responsabilities of a document to the corresponding view
/// class, since it makes more sense due to the data actually being managed
/// by the view portion of the document/view classes pair.

class ProductEditorDocument
    :public wxDocument
    ,private boost::noncopyable
{
  public:
    ProductEditorDocument();
    virtual ~ProductEditorDocument();

  protected:
    virtual void ReadDocument(wxString const& filename) = 0;
    virtual void WriteDocument(wxString const& filename) = 0;

  private:
    ProductEditorView& PredominantView() const;

    // wxDocument overrides.
    virtual bool IsModified() const;
    virtual void Modify(bool modified);
    virtual bool OnOpenDocument(wxString const& filename);
    virtual bool OnSaveDocument(wxString const& filename);
};

/// Common base for all product editor view classes.

class ProductEditorView
    :public ViewEx
    ,virtual private boost::noncopyable
{
    friend class ProductEditorDocument;

  public:
    ProductEditorView();
    virtual ~ProductEditorView();

  protected:
    virtual bool IsModified() const = 0;
    virtual void DiscardEdits() = 0;
};

/// This class is a common base for TierView and DatabaseView classes.
/// It contains and lays out common widgets.

class TreeGridViewBase
    :public ProductEditorView
    ,virtual private boost::noncopyable
{
  public:
    TreeGridViewBase();
    virtual ~TreeGridViewBase();

  protected:
    wxTreeCtrl&   GetTreeCtrl() const;
    MultiDimGrid& GetGridCtrl() const;
    void SetLabel(std::string const&);

    virtual wxTreeCtrl*   CreateTreeCtrl(wxWindow* parent) = 0;
    virtual MultiDimGrid* CreateGridCtrl(wxWindow* parent) = 0;
    virtual void SetupControls() = 0;

  private:
    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();

    wxTreeCtrl*   tree_;
    MultiDimGrid* grid_;
    wxStaticText* grid_label_;
};

inline wxTreeCtrl& TreeGridViewBase::GetTreeCtrl() const
{
    return *tree_;
}

inline MultiDimGrid& TreeGridViewBase::GetGridCtrl() const
{
    return *grid_;
}

#endif // profile_editor_hpp

