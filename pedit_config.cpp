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

// $Id: pedit_config.cpp,v 1.1.2.2 2006-03-27 13:02:05 etarassov Exp $

#include "pedit_config.hpp"

// -----------------------------
// PeditConfig implementation
// -----------------------------
wxPoint PeditConfig::ReadPoint( wxString const & key, wxPoint const & defValue ) const
{
    wxPoint res = defValue;
    {
        long tmp;
        if( Read( key + _T(".x"), &tmp ) )
            res.x = tmp;
        if( Read( key + _T(".y"), &tmp ) )
            res.y = tmp;
    }
    return res;
}
void PeditConfig::WritePoint( wxString const & key, wxPoint const & point )
{
    Write( key + _T(".x"), static_cast<long>( point.x ) );
    Write( key + _T(".y"), static_cast<long>( point.y ) );
}

wxSize PeditConfig::ReadSize( wxString const & key, wxSize const & defValue ) const
{
    wxSize res = defValue;
    {
        long tmp;
        if( Read( key + _T(".width"), &tmp ) )
            res.SetWidth( tmp );
        if( Read( key + _T(".height"), &tmp ) )
            res.SetHeight( tmp );
    }
    return res;
}
void PeditConfig::WriteSize( wxString const & key, wxSize const & size )
{
    Write( key + _T(".width"), static_cast<long>( size.GetWidth() ) );
    Write( key + _T(".height"), static_cast<long>( size.GetHeight() ) );
}

bool PeditConfig::ReadBool( wxString const & key, bool defValue ) const
{
    bool value;
    Read( key, &value, defValue );
    return value;
}
void PeditConfig::WriteBool( wxString const & key, bool value )
{
    Write( key, value );
}
wxString PeditConfig::ReadString( wxString const & key, wxString const & defValue ) const
{
    wxString value;
    Read( key, &value, defValue );
    return value;
}
void PeditConfig::WriteString( wxString const & key, wxString const & value )
{
    Write( key, value );
}
