// Sample input class for wx data-transfer demonstration.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: input.hpp,v 1.1.1.1 2004-05-15 19:58:43 chicares Exp $

#ifndef input_hpp
#define input_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "input_datum.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"

#include <string>

// These sample input classes are part of a demonstration program that
// uses wxWindows and its xml resource library to implement dialogs
// that can be maintained through xml resources instead of C++ code.
//
// At least for now, two separate input classes are used. One contains
// only strings that capture user input exactly. The other holds the
// data of various types that a program would need to capture from GUI
// input. These classes are interconvertible, with a bijective mapping
// between their members. They're distinct because conversion may not
// perfectly preserve value. For example, "1.07" in a text control may
// correspond to (double)(1.07), but the latter converted to a string
// with the maximum precision the machine is capable of would differ
// from the original "1.07". A user who reloads saved input from a
// file would likely protest "but I didn't say 1.0700000000001".
// Truncating to a 'reasonable' precision merely engenders complaints
// from other users who may enter pi to machine precision and expect
// more than "3.1416": there is no universally reasonable way to
// truncate numbers.
//
// [Note: that example impedes interconvertibility. Adding floating-
// point text controls later will force us to grapple with that.]
//
// The second of this pair of classes is intended to express dependent
// relationships among controls in the custom types of its members.
// For instance, discrete-valued controls like wxControlWithItems and
// wxRadioBox are mapped to an enumerative type that constrains
// assignment to values that are permitted in the overall context of
// the input object. For example, a radiobox might offer three choices
// but allow only the first two if the input object is in a particular
// state determined by the contents of other controls.

class TransferData
{
    friend class Input;
    friend class XmlNotebook;

  private:
    TransferData const& operator=(Input const& z);
    bool operator==(TransferData const& z);

    std::string comments;
    std::string check0;
    std::string check1;
    std::string check2;
    std::string option0;
    std::string option1;
    std::string option2;
    std::string option3;
};

class Input
    :public MemberSymbolTable<Input>
{
    friend class TransferData;
    friend class XmlNotebook;

  public:
    Input();
    Input const& operator=(TransferData const&);

    void Harmonize();

  private:
    Input(Input const&);

    std::string comments;
    input_datum check0;
    input_datum check1;
    input_datum check2;
    e_option    option0;
    e_option    option1;
    e_option    option2;
    e_option    option3;
};

#endif // input_hpp

