# Copyright (C) 1998, 1999, 2000, 2002  Los Alamos National Laboratory,
# Copyright (C) 1998, 1999, 2000, 2002  CodeSourcery, LLC
#
# This file is part of FreePOOMA.
#
# FreePOOMA is free software; you can redistribute it and/or modify it
# under the terms of the Expat license.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Expat
# license for more details.
#
# You should have received a copy of the Expat license along with
# FreePOOMA; see the file LICENSE.

default:: MakeOperators

MakeOperators: MakeOperators.cpp ClassDescriptor.h Header.h \
	OperatorDescriptor.h Parser.h PrintFunctions.h PrintOperators.h \
	PrintList.h Options.cpp Options.h PeteOps.cpp PeteOps.h \
	PAssert.cpp PAssert.h DescriptorBase.h Join.h Join.cpp
	$(CXX) -o MakeOperators -I.. $(CXX_OPT_APP_ARGS) \
	MakeOperators.cpp Options.cpp PAssert.cpp PeteOps.cpp Join.cpp

newpeteops: MakeOperators
	MakeOperators --operators PeteOps.in --guard "" --insert-op \
	--lanl-boilerplate --o PeteOps.cpp

peteheaders: OperatorTags.h

OperatorTags.h: MakeOperators
	MakeOperators --guard PETE_PETE_OPERATORTAGS_H --op-tags \
	--operators PeteOps.in --lanl-boilerplate --o OperatorTags.h

installheaders: OperatorTags.h
	mv OperatorTags.h ..

clean:
	rm -f MakeOperators
	rm -f OperatorTags.h
	rm -f *.o
	rm -rf ti_files
	rm -rf ii_files

# ACL:rcsinfo
#  ----------------------------------------------------------------------
#  $RCSfile: makefile,v $   $Author: chicares $
#  $Revision: 1.3 $   $Date: 2008-09-06 00:19:22 $
#  ----------------------------------------------------------------------
# ACL:rcsinfo
