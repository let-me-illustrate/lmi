// Server for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// $Id$

#ifndef gpt_server_hpp
#define gpt_server_hpp

#include "config.hpp"

#include "gpt_state.hpp"
#include "mc_enum_type_enums.hpp" // enum mcenum_emission
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <functional>

class gpt_input;

/// Guideline premium test server.
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

class LMI_SO gpt_server
    :public std::unary_function<fs::path const&, bool>
    ,virtual private obstruct_slicing<gpt_server>
{
  public:
    explicit gpt_server(mcenum_emission);
    ~gpt_server();

    bool operator()(fs::path const&);
    bool operator()(fs::path const&, gpt_input const&);

    void conditionally_show_timings_on_stdout() const;

    gpt_state state() const;

    double seconds_for_input       () const;
    double seconds_for_calculations() const;
    double seconds_for_output      () const;

  private:
    mcenum_emission emission_;
    gpt_state state_;
    double seconds_for_input_;
    double seconds_for_calculations_;
    double seconds_for_output_;
};

#endif // gpt_server_hpp

