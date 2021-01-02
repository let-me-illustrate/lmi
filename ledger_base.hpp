// Ledger values: common base class.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef ledger_base_hpp
#define ledger_base_hpp

#include "config.hpp"

#include "miscellany.hpp"               // minmax
#include "so_attributes.hpp"

#include <algorithm>                    // copy()
#include <cfloat>                       // DECIMAL_DIG
#include <iomanip>                      // setprecision()
#include <iterator>                     // ostream_iterator
#include <map>
#include <ostream>
#include <string>
#include <vector>

class CRC;

/// Design notes for class LedgerBase.
///
/// Ledger statement values. This abstract base class provides a map for
/// storing vectors of values, and functions to manipulate them.
///
/// A ledger contains numerous vectors, which are inserted into the map by
/// a derived class. Iterating across the map lets us factor out operations
/// we need to apply to all vectors. For instance, instead of writing the
/// name of each vector in a copy constructor:
///   this->vector0 = object.vector0;
///   this->vector1 = object.vector1;
///   ... for thirty-five vectors at the time this was first written
/// we specify only the operation and the map to apply it to. The motivation
/// is that writing dozens of names for each of several such operations leads
/// to bulky code that is difficult to maintain and prone to error. On the
/// other hand, given an implementation of operator= that specifies only the
/// name of the map and the operation of assignment, it is trivial to
/// implement operator== by specifying the operation of comparison instead,
/// and changes to the map affect only the map itself, not the implementations
/// of such operators.
///
/// Ledgers require several special operations. We need to be able to add a
/// group of individual ledgers together to produce a group composite. To
/// avoid overflowing output fields (on large cases, composite values can far
/// exceed a billion dollars), we scale ledgers by a power of 1000. Scaling
/// means finding the number that would print the widest in any vector,
/// determining a scale factor that makes it printable, and applying that
/// scale factor to all vectors.
///
/// We prefer to store ledgers per dollar inforce, that is, without
/// multiplying them by inforce factors, and to include a vector of inforce
/// factors. At present we convert ledgers to per dollar issued, that is,
/// multiply them by inforce factors, only when producing composites. Some
/// vectors are multiplied by the inforce factor for the beginning of the
/// year, and others by that for the end; still others, such as interest
/// rates, permit no inforce adjustment. Similarly, interest rates are not
/// added together when creating a composite, while other vectors like
/// death benefits are additive. This is the reason why we have several maps.
///
/// Design decision: a few maps instead of many vectors.
///
/// We want to assemble a large number of vectors into a collection so that
/// we can iterate over the collection instead of performing operations (e.g.
/// initialize, copy, test for equality, find max/min element, multiply) for
/// each vector separately. The reason is that performing a half dozen
/// operations for each of a couple dozen vectors requires us to write about
/// a dozen squared names, getting each one exactly right. Adding a new vector
/// requires us to write its name in each of a half dozen places. This was the
/// approach originally taken, and the resulting code was sprawling, hard to
/// maintain, and difficult to validate.
///
/// We note that almost all are vector<double>, have the same size(), and
/// are initialized to zero. Treating only this majority subset of vector
/// members substantially fulfills our wishes, so we won't attempt polymorphic
/// treatment of the other vectors.
///
/// A map<> is clearly called for. Certain implementation choices must be made.
///
/// Choice 1: Who owns the data? We want only one instance of the data to
/// exist. If we have a bunch of vectors and a map<key,vector> then we have
/// two copies; that wastes memory and, worse, the copies can become
/// unsynchronized.
///
/// 1.a. Vector members own the data. Then we need a map<key, vector<>*>.
///
/// 1.b. Map owns the data. Then we want vector& members that refer to map
/// entries. Reason: we extremely often want to read or write one element
/// of a vector, and we don't want a map lookup each time. But this approach
/// appears problematic: for instance, what happens to those references when
/// we copy the map?
///
/// We choose 1.a.
///
/// Choice 2. Some vectors represent beginning-of-year data; others,
/// end-of-year data. Some are "arithmetic"; others are not. Arithmetic, an
/// adjective, here means that arithmetic can be performed upon them in the
/// sense of the following example. Payments can be multiplied by a scale
/// factor (1/1000, 1/1000000, etc.) to make them print in a given width, and
/// can be added together when combining multiple ledgers into a composite,
/// so they are arithmetic. Interest rates are not.
///
/// 2.a. Separate maps for BOY and EOY.
///
/// 2.b. Wrap vector<>* into a class and add a member BOY/EOY/non-arith indicator.
///
/// To make this choice, we need to consider wrappers further.
///
/// Choice 3. We can use vector<> pointers as such, or wrap them. Since we
/// chose to use vector<> members that own their own storage, the pointer
/// would be needed only within this class, for dealing with whole groups of
/// vectors at a time.
///
/// 3.a. Use vector<>* . Only map::data_type is a pointer; key_type is not.
/// STL algorithms like copy() would operate on the pointers themselves,
/// which is not the behavior we want. We could code such operations by hand:
/// iterate across the map, and dereference pointers to operate on the
/// data inside a loop or with transform() e.g.
///
/// 3.b. Use wrappers. The extra level of indirection is not a real concern
/// because we would intend to address all elements of a vector when going
/// through the wrapper.
///
/// A wrapper for data_type doesn't need to be very elaborate. According to
/// Matt Austern's authoritative book, the type requirements of map<> specify
/// that map::data_type be Assignable. It need not even be EqualityComparable.
///
/// The equivalence between map[k] and
///     (*((map.insert(value_type(k, data_type()))).first)).second
/// suggests that data_type must also be DefaultConstructible.
///
/// The operations we need for each vector<> are initialization, assignment,
/// scaling (multiplying by a scalar), adding, and finding the max and min
/// elements. The last four of these six are arithmetic, so we can regard
/// the necessary operations as predominantly arithmetic. So the choice
/// really amounts to
///     3.a. iterate across map, dereference vector, perform operation
///     3.b. iterate across map, perform operation; push operation into ptr class
/// The second option would mean defining a min_element operation on the ptr
/// class, which sounds like a Bad Idea.
///
/// A hybrid might be considered: 3.a. for arithmetic operations, and 3.b. for
/// other operations. This seems to secure the disadvantages of both pure
/// approaches at the cost of increased complexity.
///
/// We choose 3.a., which impels us to choose 2.a.

typedef std::map<std::string,std::vector<double>*> double_vector_map;
typedef std::map<std::string,std::string*> string_map;
typedef std::map<std::string,double*> scalar_map;

class LMI_SO LedgerBase
{
    friend class Ledger;

  public:
    virtual ~LedgerBase() = default;

    void               apply_scale_factor(int decimal_power);

    minmax<double>     scalable_extrema() const;
    std::string const& scale_unit() const;
    std::string        value_str(std::string const& map_key, int index) const;
    std::string        value_str(std::string const& map_key) const;

    double_vector_map const& all_vectors() const;

  protected:
    explicit LedgerBase(int a_Length);
    LedgerBase(LedgerBase const&);
    LedgerBase& operator=(LedgerBase const&);

    void Alloc();   // Merge certain maps together.
    void Copy(LedgerBase const&);
    void Initialize(int a_Length);

    LedgerBase& PlusEq
        (LedgerBase          const& a_Addend
        ,std::vector<double> const& a_Inforce
        );

    virtual int     GetLength() const = 0;
    virtual void    UpdateCRC(CRC&) const;
    virtual void    Spew(std::ostream&) const;

    // TODO ?? A priori, protected data is a defect.

    // Pointers to std::vector<double> members are stored in these maps for
    // reasons discussed in the design notes above.
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
    int                 scale_power_; // E.g., for (000,000): 6
    std::string         scale_unit_;  // E.g., for (000,000): "millions"
};

template<typename T> void SpewVector
    (std::ostream        &  os
    ,std::string    const&  name
    ,std::vector<T> const&  elements
    )
{
    std::ostream_iterator<T> osi(os, "\n");

    os << name << '\n';
    os << std::setprecision(DECIMAL_DIG);
    std::copy(elements.begin(), elements.end(), osi);
}

#endif // ledger_base_hpp
