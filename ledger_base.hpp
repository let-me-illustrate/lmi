// Ledger values: common base class.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_base.hpp,v 1.1 2005-02-12 12:59:31 chicares Exp $

#ifndef ledger_base_hpp
#define ledger_base_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <algorithm>
#include <cfloat>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>
#include <vector>

class CRC;

// Ledger statement values. This abstract base class provides a map for
// storing vectors of values, and functions to manipulate them.
//
// A ledger contains numerous vectors, which are inserted into the map by
// a derived class. Iterating across the map lets us factor out operations
// we need to apply to all vectors. For instance, instead of writing the
// name of each vector in a copy constructor:
//   this->vector0 = object.vector0;
//   this->vector1 = object.vector1;
//   ... for thirty-five vectors at the time this was first written
// we specify only the operation and the map to apply it to. The motivation
// is that writing dozens of names for each of several such operations leads
// to bulky code that is difficult to maintain and prone to error. On the
// other hand, given an implementation of operator= that specifies only the
// name of the map and the operation of assignment, it is trivial to
// implement operator== by specifying the operation of comparison instead,
// and changes to the map affect only the map itself, not the implementations
// of such operators.
//
// Ledgers require several special operations. We need to be able to add a
// group of individual ledgers together to produce a group composite. To
// avoid overflowing output fields (on large cases, composite values can far
// exceed a billion dollars), we scale ledgers by a power of 1000. Scaling
// means finding the number that would print the widest in any vector,
// determining a scale factor that makes it printable, and applying that
// scale factor to all vectors.
//
// We prefer to store ledgers per dollar inforce, that is, without
// multiplying them by inforce factors, and to include a vector of inforce
// factors. At present we convert ledgers to per dollar issued, that is,
// multiply them by inforce factors, only when producing composites. Some
// vectors are multiplied by the inforce factor for the beginning of the
// year, and others by that for the end; still others, such as interest
// rates, permit no inforce adjustment. Similarly, interest rates are not
// added together when creating a composite, while other vectors like
// death benefits are additive. This is the reason why we have several maps.
//
// See the design discussion in the accompanying .cpp file.

typedef std::map<std::string, std::vector<double>*, std::less<std::string> > double_vector_map;
typedef std::map<std::string, std::string*, std::less<std::string> > string_map;
typedef std::map<std::string, double*, std::less<std::string> > scalar_map;

class LMI_EXPIMP LedgerBase
{
    friend class Ledger;

  public:
    virtual ~LedgerBase();

    void               ApplyScaleFactor(double a_Mult);

    double             DetermineScaleFactor() const;
    std::string const& ScaleUnit() const;
    double             ScaleFactor() const;
    std::string        value_str(std::string const& map_key, int index) const;
    std::string        value_str(std::string const& map_key) const;

  protected:
    explicit LedgerBase(int a_Length);
    LedgerBase(LedgerBase const&);
    LedgerBase& operator=(LedgerBase const&);

    void Alloc();   // Merge certain maps together.
    void Copy(LedgerBase const&);
    void Initialize(int a_Length);

    LedgerBase& PlusEq
        (LedgerBase const&         a_Addend
        ,std::vector<double> const& a_Inforce
        );

    virtual int     GetLength() const = 0;
    virtual void    UpdateCRC(CRC& crc) const;
    virtual void    Spew(std::ostream& os) const;

    // Pointers to std::vector<double> members are stored in these maps for
    // reasons discussed in the .cpp file.
    //
    // "Arithmetic" vectors representing BOY quantities.
    double_vector_map   BegYearVectors;
    // "Arithmetic" vectors representing EOY quantities.
    double_vector_map   EndYearVectors;
    // "Arithmetic" vectors unaffected by survivorship.
    double_vector_map   ForborneVectors;
    // Vectors that are not "arithmetic"--i.e. not additive.
    double_vector_map   OtherVectors;
    // All four of the above merged together.
    double_vector_map   AllVectors;
    // All "arithmetic" vectors merged together: scaled to avoid overflow.
    double_vector_map   ScalableVectors;

    // "Arithmetic" scalars
    scalar_map          ScalableScalars;
    // Scalars that are not "arithmetic"--i.e. not additive
    scalar_map          OtherScalars;
    // Both of the above merged together
    scalar_map          AllScalars;

    string_map          Strings;

  private:
    double              m_scaling_factor;
    std::string         m_scale_unit; // E.g. "thousands", "millions".
};

template<typename T> void SpewVector
    (std::ostream&          os
    ,std::string const&     name
    ,std::vector<T> const&  elements
    )
{
    std::ostream_iterator<T> osi(os, "\n");

    static int const prec =
//      std::numeric_limits<double>::digits10;
//      DBL_DIG;
//      LDBL_DIG;
        18; // TODO ?? gcc has this wrong (?)
    // Somehow BC++ gives at least 19 digits;
    // it prolly emits long doubles
// TODO ?? Consider using max_stream_precision.hpp instead.
    os << name << '\n';
//      os << setiosflags(std::ios_base::fixed) << std::setprecision(prec);
    os << std::setprecision(prec);
    std::copy(elements.begin(), elements.end(), osi);
}

#endif // ledger_base_hpp

