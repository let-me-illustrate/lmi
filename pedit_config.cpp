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
