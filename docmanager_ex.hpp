// Customize implementation details of library class wxDocManager.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef docmanager_ex_hpp
#define docmanager_ex_hpp

#include "config.hpp"

#include <wx/docview.h>

#include <memory>                       // unique_ptr

class WXDLLIMPEXP_FWD_CORE wxMenuBar;
class WXDLLIMPEXP_FWD_CORE wxPageSetupDialogData;
class WXDLLIMPEXP_FWD_CORE wxPrintData;

class DocManagerEx
    :public wxDocManager
{
  public:
    explicit DocManagerEx
        (long int flags  = 0
        ,bool initialize = true
        );
    // WX !! Base class wxDocManager's dtor is virtual, but isn't
    // explicitly marked that way--though it seems that it should be,
    // for consistency with the style of the rest of the library.
    //
    ~DocManagerEx() override = default;

    void AssociateFileHistoryWithFileMenu(wxMenuBar*);
    void DissociateFileHistoryFromFileMenu(wxMenuBar*);

  private:
    DocManagerEx(DocManagerEx const&) = delete;
    DocManagerEx& operator=(DocManagerEx const&) = delete;

    void UponPageSetup(wxCommandEvent&);
    // WX !! OnPreview() and OnPrint() aren't virtual; shouldn't they be?
    void UponPreview(wxCommandEvent&);
    void UponPrint(wxCommandEvent&);

    // WX !! In msw, 'Page setup' has superseded 'Print setup', but wx
    // provides a builtin ID only for the former and not for the
    // latter. Shouldn't OnPageSetup() be added, and OnPrintSetup() be
    // deprecated unless it's wanted on other platforms?

    // wxDocManager overrides.
    wxDocTemplate* SelectDocumentType
        (wxDocTemplate** templates
        ,int             noTemplates
        ,bool            sort
        ) override;

    std::unique_ptr<wxPrintData>           const print_data_;
    std::unique_ptr<wxPageSetupDialogData> const page_setup_data_;

    DECLARE_DYNAMIC_CLASS(DocManagerEx)
    DECLARE_EVENT_TABLE()
};

#endif // docmanager_ex_hpp
