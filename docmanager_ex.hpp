// Customize implementation details of library class wxDocManager.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: docmanager_ex.hpp,v 1.4 2006-01-29 13:52:00 chicares Exp $

#ifndef docmanager_ex_hpp
#define docmanager_ex_hpp

#include "config.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT
#include <wx/docview.h>

#include <memory>

class WXDLLEXPORT wxMenuBar;

class DocManagerEx
    :public wxDocManager
    ,private boost::noncopyable
{
  public:
    explicit DocManagerEx
        (long flags      = wxDEFAULT_DOCMAN_FLAGS
        ,bool initialize = true
        );
    // WX !! Base class wxDocManager's dtor is virtual, but isn't
    // explicitly marked that way--though it seems that it should be,
    // for consistency with the style of the rest of the library.
    //
    virtual ~DocManagerEx();

    void AssociateFileHistoryWithFileMenu(wxMenuBar*);
    void DissociateFileHistoryFromFileMenu(wxMenuBar*);

  private:
    void OnPageSetup(wxCommandEvent&);
    // WX !! These functions don't seem to be virtual; shouldn't they be?
    void OnPreview(wxCommandEvent&);
    void OnPrint(wxCommandEvent&);

    // WX !! In msw, 'Page setup' has superseded 'Print setup', but wx
    // provides a builtin ID only for the former and not for the
    // latter. Shouldn't OnPageSetup() be added, and OnPrintSetup() be
    // deprecated unless it's wanted on other platforms?
//    void OnPrintSetup(wxCommandEvent&);

    // wxDocManager overrides.
    virtual wxDocTemplate* SelectDocumentType
        (wxDocTemplate** templates
        ,int             noTemplates
        ,bool            sort
        );

    std::auto_ptr<wxPrintData> print_data_;
    std::auto_ptr<wxPageSetupDialogData> page_setup_data_;

    DECLARE_DYNAMIC_CLASS(DocManagerEx)
    DECLARE_EVENT_TABLE()
};

#endif // docmanager_ex_hpp

