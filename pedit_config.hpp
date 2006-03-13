// Product editor custom config class.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: pedit_config.hpp,v 1.1.2.1 2006-03-13 19:56:52 etarassov Exp $

#ifndef PEDIT_CONFIG_HPP_
#define PEDIT_CONFIG_HPP_

#include "config.hpp"

#include <wx/config.h>
#include <wx/defs.h>
#include <wx/gdicmn.h>

/**
   Customized version of wxConfig for persistent values storage by LMI Product Editor.

   Simplified interface for loading/storing values of different types. The goal
   is to make an expression-compatible interface out of Read/Write methods.
 */
class PeditConfig : public wxConfig
{
public:
    PeditConfig( ) : wxConfig( _T("Lmi Product Editor"), _T("Mass Mutual") ) {}
    virtual ~PeditConfig() {}

    /// Read various objects information from from config
    /// @{
    wxPoint ReadPoint( wxString const & key, wxPoint const & defValue = wxDefaultPosition ) const;
    void WritePoint( wxString const & key, wxPoint const & value );

    wxSize ReadSize( wxString const & key, wxSize const & defValue = wxDefaultSize ) const;
    void WriteSize( wxString const & key, wxSize const & value );

    bool ReadBool( wxString const & key, bool defValue = true ) const;
    void WriteBool( wxString const & key, bool value );

    wxString ReadString( wxString const & key, wxString const & defValue = _T("") ) const;
    void WriteString( wxString const & key, wxString const & value );
    /// @}
};

#endif /*PEDIT_CONFIG_HPP_*/
