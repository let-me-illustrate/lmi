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

default:: MakeOperators$(EXEEXT)

MakeOperators$(EXEEXT): MakeOperators.cpp ClassDescriptor.h Header.h \
	OperatorDescriptor.h Parser.h PrintFunctions.h PrintOperators.h \
	PrintList.h Options.cpp Options.h PeteOps.cpp PeteOps.h \
	PAssert.cpp PAssert.h DescriptorBase.h Join.h Join.cpp
	$(CXX) -o MakeOperators$(EXEEXT) -I.. $(CXX_OPT_APP_ARGS) \
	MakeOperators.cpp Options.cpp PAssert.cpp PeteOps.cpp Join.cpp

newpeteops: MakeOperators$(EXEEXT)
	./MakeOperators$(EXEEXT) --operators PeteOps.in --guard "" --insert-op \
	--lanl-boilerplate --o PeteOps.cpp

peteheaders: OperatorTags.h

OperatorTags.h: MakeOperators$(EXEEXT)
	./MakeOperators$(EXEEXT) --guard PETE_PETE_OPERATORTAGS_H --op-tags \
	--operators PeteOps.in --lanl-boilerplate --o OperatorTags.h

installheaders: OperatorTags.h
	mv OperatorTags.h ..

clean:
	rm -f MakeOperators$(EXEEXT)
	rm -f OperatorTags.h
	rm -f *.o
	rm -rf ti_files
	rm -rf ii_files

# ACL:rcsinfo
#  ----------------------------------------------------------------------
#  $RCSfile: makefile,v $   $Author: chicares $
#  $Revision: 1.5 $   $Date: 2008-09-09 03:15:19 $
#  ----------------------------------------------------------------------
# ACL:rcsinfo
