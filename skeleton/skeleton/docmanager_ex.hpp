// Customize implementation details of library class wxDocManager.
//
// Copyright (C) 2002, 2003, 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: docmanager_ex.hpp,v 1.1.1.1 2004-05-15 19:58:27 chicares Exp $

#ifndef docmanager_ex_hpp
#define docmanager_ex_hpp

#include "config.hpp"

#include <wx/defs.h> // WXDLLEXPORT
#include <wx/docview.h>

#include <memory>

class WXDLLEXPORT wxMenuBar;

class DocManagerEx
    :public wxDocManager
{
  public:
    explicit DocManagerEx
        (long flags      = wxDEFAULT_DOCMAN_FLAGS
        ,bool initialize = true
        );
    ~DocManagerEx();

    void AssociateFileHistoryWithFileMenu(wxMenuBar*);
    void DissociateFileHistoryFromFileMenu(wxMenuBar*);

  private:
    DocManagerEx(DocManagerEx const&);
    DocManagerEx& operator=(DocManagerEx const&);

    void OnPageSetup(wxCommandEvent&);
    void OnPreview(wxCommandEvent&);
    void OnPrint(wxCommandEvent&);
    void OnPrintSetup(wxCommandEvent&);

    std::auto_ptr<wxPrintData> print_data_;
    std::auto_ptr<wxPageSetupDialogData> page_setup_data_;

    DECLARE_DYNAMIC_CLASS(DocManagerEx)
    DECLARE_EVENT_TABLE();
};

#endif // docmanager_ex_hpp

