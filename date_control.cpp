// Calendar-date control derived from wxTextCtrl.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: date_control.cpp,v 1.1 2005-03-11 03:19:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "date_control.hpp"

#include "calendar_date.hpp"
#include "value_cast.hpp"

#include <string>

// TODO ?? Replace this with wxDatePickerCtrl.

IMPLEMENT_DYNAMIC_CLASS(DateControl, wxTextCtrl)

namespace
{
// Namespace contents Copyright (C) 2003 Gregory W. Chicares.
// TODO ?? Move this to the calendar-date class.

    int const jdn00010301 = 1721119;
    int const days_in_four_centuries = 146097;
    int const days_in_four_years = 1461;

    int GregorianToJdn(int year, int month, int day)
    {
        if(2 < month)
            {
            month -= 3;
            }
        else
            {
            month += 9;
            --year;
            }
        int c = year / 100;
        year -= 100 * c;
        return
              jdn00010301
            + day
            + (2 + 153 * month) / 5
            + ((days_in_four_years     * year) >> 2)
            + ((days_in_four_centuries * c   ) >> 2)
            ;
    }

    void JdnToGregorian(int j, int& year, int& month, int& day)
    {
        j -= jdn00010301;
        year  = ((j   << 2) - 1) / days_in_four_centuries;
        j     = ((j   << 2) - 1) - days_in_four_centuries * year;
        day   = j >> 2;
        j     = ((day << 2) + 3) / days_in_four_years;
        day   = ((day << 2) + 3) - days_in_four_years * j;
        day   = (day + 4) >> 2;
        month = (5 * day    - 3) / 153;
        day   = (5 * day    - 3) - 153 * month;
        day   = (day + 5) / 5;
        year  = 100 * year + j;
        if(month < 10)
            {
            month += 3;
            }
        else
            {
            month -= 9;
            ++year;
            }
    }

    int YyyyMmDdToJdn(int g)
    {
        int year = g / 10000;
        g -= year * 10000;
        int month = g / 100;
        g -= month * 100;
        int day = g;
        return GregorianToJdn(year, month, day);
    }

    int JdnToYyyyMmDd(int j)
    {
        int year;
        int month;
        int day;
        JdnToGregorian(j, year, month, day);
        return day + 100 * month + 10000 * year;
    }
} // Unnamed namespace.

DateControl::DateControl()
    :wxTextCtrl()
{
}

DateControl::~DateControl()
{
}

DateControl::DateControl
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxString    const& value
    ,wxPoint     const& pos
    ,wxSize      const& size
    ,long               style
    ,wxValidator const& validator
    ,wxString    const& name
    )
    :wxTextCtrl
        (parent
        ,id
        ,value
        ,pos
        ,size
        ,style
        ,validator
        ,name
        )
{
}

// Translate YYYYMMDD to jdn.
wxString DateControl::GetValue() const
{
    std::string s = wxTextCtrl::GetValue().c_str();

    // If an empty string is transferred, pass it through unchanged.
    // Apparently this occurs at startup.
    //
    if(s.empty())
        {
        return s.c_str();
        }

    s = value_cast<std::string>(YyyyMmDdToJdn(value_cast<int>(s)));

    return s.c_str();
}

// Translate jdn to YYYYMMDD.
void DateControl::SetValue(const wxString& value)
{
    std::string s = value.c_str();

    s = value_cast<std::string>(JdnToYyyyMmDd(value_cast<int>(s)));

    wxTextCtrl::SetValue(s.c_str());
}

