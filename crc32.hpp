// 32-bit cyclic redundancy check.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef crc32_hpp
#define crc32_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "ssize_lmi.hpp"                // sstrlen()

#include <string>
#include <vector>

class LMI_SO CRC
{
  public:
    CRC();

    unsigned int value() const;

    CRC& operator+=(                    bool    );
    CRC& operator+=(                    char    );
    CRC& operator+=(  signed            char    );
    CRC& operator+=(unsigned            char    );
    CRC& operator+=(            short   int     );
    CRC& operator+=(unsigned    short   int     );
    CRC& operator+=(                    int     );
    CRC& operator+=(unsigned            int     );
    CRC& operator+=(            long    int     );
    CRC& operator+=(unsigned    long    int     );
    CRC& operator+=(                    float   );
    CRC& operator+=(                    double  );
    CRC& operator+=(            long    double  );

    CRC& operator+=(                    char const*);
    CRC& operator+=(  signed            char const*);
    CRC& operator+=(unsigned            char const*);

    CRC& operator+=(std::string const&);

  private:
    CRC& update
        (unsigned char const*  buf
        ,int                   len
        );

    unsigned int value_;
};

// operator+=() inline implementations.
// SOMEDAY !! Use one template for the bodies of these implementations
// (C++11 will make that easier).

inline CRC& CRC::operator+=(                    bool    z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(                    char    z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(signed              char    z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(unsigned            char    z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(            short   int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(unsigned    short   int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(                    int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(unsigned            int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(            long    int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(unsigned    long    int     z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(                    float   z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(                    double  z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}
inline CRC& CRC::operator+=(            long    double  z)
    {return update(reinterpret_cast<unsigned char const*>(&z), sizeof z);}

// Forward char const* and signed char const* to the unsigned char const* function.
inline CRC& CRC::operator+=(                    char const* z)
    {
    return operator+=(reinterpret_cast<unsigned char const*>(z));
    }
inline CRC& CRC::operator+=(signed              char const* z)
    {
    return operator+=(reinterpret_cast<unsigned char const*>(z));
    }
inline CRC& CRC::operator+=(unsigned            char const* z)
    {
    // std::strlen() is defined only for char const* arguments, so the
    // cast is required.
    return update(z, lmi::sstrlen(reinterpret_cast<char const*>(z)));
    }

template<typename T>
CRC& operator+=
    (CRC&                   crc
    ,std::vector<T> const&  v
    )
{
    typename std::vector<T>::const_iterator i = v.begin();
    while(i != v.end())
        {
        crc += *i++;
        }
    return crc;
}

#endif // crc32_hpp
