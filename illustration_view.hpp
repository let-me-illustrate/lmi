// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: illustration_view.hpp,v 1.17.2.3 2006-11-03 16:46:06 etarassov Exp $

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

#include "obstruct_slicing.hpp"
#include "ledger_text_formats.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <wx/event.h>

#include <string>

class IllustrationDocument;
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

    // Making these functions public so that they can be invoked by
    // class CensusView is arguably less bad than making that class a
    // friend.
    //
    void DisplaySelectedValuesAsHtml();
    void Pdf(std::string const& action) const;
    void Run(Input* = 0);
    void SetLedger(boost::shared_ptr<Ledger const>);

  private:
    IllustrationDocument& document() const;

    enum enum_copy_options
        {e_copy_values
        ,e_copy_calculation_summary
        };

    void CopyLedgerIntoClipboard(enum_copy_options);
    int EditProperties();

    enum enum_print_options
        {e_print_printer
        ,e_print_preview
        };
    // Print HTML code to the printer or preview.
    void PrintCS(enum_print_options) const;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow();
    wxIcon Icon() const;
    wxMenuBar* MenuBar() const;

    // ViewEx overrides.
    bool OnCreate              (wxDocument*, long int);

    void UponCopyLedgerValues  (wxCommandEvent&);
    void UponCopyLedgerCalculationSummary(wxCommandEvent&);
    void UponMenuOpen          (wxMenuEvent&);
    void UponPreviewCS         (wxCommandEvent&);
    void UponPreviewPdf        (wxCommandEvent&);
    void UponPrintCS           (wxCommandEvent&);
    void UponPrintPdf          (wxCommandEvent&);
    void UponProperties        (wxCommandEvent&);
    void UponUpdateFileSave    (wxUpdateUIEvent&);
    void UponUpdateFileSaveAs  (wxUpdateUIEvent&);
    void UponUpdateInapplicable(wxUpdateUIEvent&);
    void UponUpdateProperties  (wxUpdateUIEvent&);

    wxHtmlWindow* html_window_;
    bool is_phony_;
    boost::shared_ptr<Ledger const> ledger_values_;
    LedgerFormatter ledger_formatter_;
    std::string selected_values_as_html_;

    DECLARE_DYNAMIC_CLASS(IllustrationView)
    DECLARE_EVENT_TABLE()
};

IllustrationView& MakeNewIllustrationDocAndView
    (wxDocManager* dm
    ,char const*   filename
    );

bool RunSpecialInputFileIfPresent(wxDocManager*);

#endif // illustration_view_hpp

