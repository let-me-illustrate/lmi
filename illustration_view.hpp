// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: illustration_view.hpp,v 1.5 2005-04-30 18:15:33 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2003 to create a standalone
// view class customized for illustration documents, and in the later
// years given in the copyright notice above.

#ifndef illustration_view_hpp
#define illustration_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "illustration_document.hpp"
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <wx/event.h>

#include <memory>
#include <string>

class Input;
class Ledger;
class WXDLLEXPORT wxHtmlWindow;

// TODO ?? Consider adding an input reference member. Here, it's used
// only for edit and run; in the census view class, it's used widely.

class IllustrationView
    :public ViewEx
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<IllustrationView>
{
    friend class IllustrationDocument;

  public:
    IllustrationView();
    virtual ~IllustrationView();

  private:
    IllustrationDocument& document() const;

    int EditProperties();

    // ViewEx required implementation.
    wxWindow* CreateChildWindow();
    wxIcon Icon() const;
    wxMenuBar* MenuBar() const;

  public: // TODO ?? Or make class CensusView a friend; or make these free functions.
    void DisplaySelectedValuesAsHtml();
    void Pdf(std::string action);
    void Run(Input* = 0);
    void SetLedger(Ledger const&);

  private:
    // ViewEx overrides.
    bool OnCreate            (wxDocument*, long);

    // wxView overrides.
// TODO ?? expunge    wxPrintout* OnCreatePrintout();

    void OnMenuOpen          (wxMenuEvent&);
    void OnPreviewPdf        (wxCommandEvent&);
    void OnPrintPdf          (wxCommandEvent&);
    void OnProperties        (wxCommandEvent&);
    void OnUpdateFileSave    (wxUpdateUIEvent&);
    void OnUpdateFileSaveAs  (wxUpdateUIEvent&);
    void OnUpdateInapplicable(wxUpdateUIEvent&);
    void OnUpdateProperties  (wxUpdateUIEvent&);

// The following function probably should be factored out into
// a utility module. The original base_filename() is
// Copyright (C) 1998, 1999, 2000, 2002, 2003, 2004 Gregory W. Chicares.
//
    // Extract the base portion of the filename, excluding path and extension.
    std::string const base_filename() const;

    wxHtmlWindow* html_window_;
    bool is_phony_;
    std::auto_ptr<Ledger> ledger_values_;
    std::string selected_values_as_html_;

    DECLARE_DYNAMIC_CLASS(IllustrationView)
    DECLARE_EVENT_TABLE()
};

#endif // illustration_view_hpp

