// Input 'notebook' (tabbed dialog) driven by xml resources.
//
// Copyright (C) 2003, 2004 Gregory W. Chicares.
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

// $Id: xml_notebook.hpp,v 1.1.1.1 2004-05-15 19:59:44 chicares Exp $

#ifndef xml_notebook_hpp
#define xml_notebook_hpp

#include "config.hpp"

#include <wx/dialog.h>

#include <string>

// Unlike other event classes used here, class wxNotebookEvent is not
// defined by including class wxDialog's header.
class WXDLLEXPORT wxNotebookEvent;

// Bind(), Validate(): want const, but library doesn't allow

class XmlNotebook
    :public wxDialog
{
  public:
    XmlNotebook(wxWindow* parent);
    ~XmlNotebook();

  private:
    XmlNotebook(XmlNotebook const&);
    XmlNotebook const& operator=(XmlNotebook const&);

    void Bind(std::string const& name, std::string& data);
    
    void ConditionallyEnable
        (std::string const& input_name
        ,wxWindow&          control
        );
    void ConditionallyEnableItems
        (std::string const& input_name
        ,wxWindow&          control
        );

    void OnOK                (wxCommandEvent& );
    void OnPageChanging      (wxNotebookEvent&);
    void OnUpdateGUI         (wxUpdateUIEvent&);
    bool Validate();

    DECLARE_EVENT_TABLE()
};

#endif // xml_notebook_hpp

