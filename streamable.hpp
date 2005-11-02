// Mixin for xml streaming operators.
//
// Copyright (C) 2001, 2002, 2005 Gregory W. Chicares.
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

// $Id: streamable.hpp,v 1.3 2005-11-02 07:26:50 chicares Exp $

// This was originally designed to write data serially to iostreams,
// in a format
//   string-representation-0 \n string-representation-1 \n ...
// While compact and portable across different machines, this format
// depended on an implicit order of data variables. Thus, it was
// not readily readable, and adding a new variable required special
// code for backward compatibility.
//
// To eliminate this problem, we changed to xml, which gives name-
// value pairs instead of just implicitly ordered values. This makes
// input files human readable. Unknown names can be ignored (perhaps
// after an informational message, and a default value can be given
// to any variable for which no xml entity is found in the input file.
// This provides backward compatibility automatically (new versions of
// the program can read old input files), and also provides forward
// compatibility (old versions of the program can read new input
// files, ignoring new entities) to the greatest extent possible.
//
// Furthermore, xml is a standard for sharing data among programs.
// An insurance industry group has proposed a standard xml dtd for
// insurance data, but we don't use it because it does not appear to
// be freely distributed.
//
// TODO ?? Since xml does most of the work that this mixin originally
// did, do we want this mixin any more?
//
// TODO ?? At any rate, remove irrelevant history from the
// foregoing discussion.

#ifndef streamable_hpp
#define streamable_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <xmlwrapp/node.h>

#include <string>

class LMI_EXPIMP streamable
{
  public:
    virtual ~streamable() {}

    virtual void read(xml::node&) = 0;
    virtual void write(xml::node&) const = 0;

    virtual int class_version() const = 0;
    virtual std::string xml_root_name() const = 0;
};

inline xml::node& LMI_EXPIMP operator>>(xml::node& x, streamable& z)
{
    z.read(x);
    return x;
}

inline xml::node& LMI_EXPIMP operator<<(xml::node& x, streamable const& z)
{
    z.write(x);
    return x;
}

#endif // streamable_hpp

