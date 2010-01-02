// MEC-testing server.
//
// Copyright (C) 2009 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mec_server.hpp,v 1.1 2009-07-30 21:46:13 chicares Exp $

#ifndef mec_server_hpp
#define mec_server_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp" // enum mcenum_emission
#include "mec_state.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <functional>

class mec_input;

/// MEC-testing server.
///
/// operator() returns bool only to follow class illustrator, not
/// because the return value is useful. SOMEDAY !! Reconsider that.
///
/// Some 'mcenum_emission' enumerators don't make sense here. However,
/// a distinct enumeration seems unwarranted, especially because
/// explaining another one in '--help' would be too complicated.
/// Enumerators that don't make sense can be reported at run time.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO mec_server
    :public std::unary_function<fs::path const&, bool>
    ,virtual private obstruct_slicing<mec_server>
{
  public:
    explicit mec_server(mcenum_emission);
    ~mec_server();

    bool operator()(fs::path const&);
    bool operator()(fs::path const&, mec_input const&);

    void conditionally_show_timings_on_stdout() const;

    mec_state state() const;

    double usec_for_input       () const;
    double usec_for_calculations() const;
    double usec_for_output      () const;

  private:
    mcenum_emission emission_;
    mec_state state_;
    double usec_for_input_;
    double usec_for_calculations_;
    double usec_for_output_;
};

#endif // mec_server_hpp

