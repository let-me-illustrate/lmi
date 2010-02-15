// Joshua Rowe's "Really cool persistent object stream library".
//
// Copyright (C) 2000, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// This is a derived work based on Joshua Rowe's
//   "Really cool persistent object stream library"
// which he released this under GNU General Public License version 2.
//
// The original, version 0.0.3, was downloaded from:
//   http://metalab.unc.edu/pub/Linux/devel/lang/c++/pstream-0.0.3.tar.gz
// and was modified as follows:
//
//   20000528 Gregory W. Chicares made changes marked inline 'GWC',
//   and released the modified version it under the same version of
//   the same license as the original. Any defect in this modified
//   version should not reflect on Joshua Rowe's reputation.
//
//   20050108 Gregory W. Chicares substituted 'LMI_MSW' for a
//   proprietary macro.
//
// and in any later years given in the copyright notice above as
// detailed in ChangeLog.

/* fpios.cc
 * Written by Joshua Rowe
 * file-based persistent object streams
 */

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_fpios.hpp" // GWC arbitrarily changed .hh to .hpp

// Begin GWC mods
// Neither of these is in the language standard
//#include  <unistd.h>
#include "platform_dependent.hpp" // unistd.h and other platforms too
// End GWC mods

#include <errno.h>
#include <fcntl.h>

// TODO ?? Probably anything here that's actually useful should be
// moved to 'platform_dependent.hpp'.
#if defined LMI_POSIX
#   include <sys/types.h>
#   include <sys/stat.h>
#elif defined LMI_MSW
#   if defined __GNUC__ && defined __STRICT_ANSI__
#       define GNU_STRICT_ANSI_ORIGINALLY_NOT_DEFINED
#       undef __STRICT_ANSI__
#   endif // !(defined(__GNUC__) && defined(__STRICT_ANSI__))
#   include <io.h>
#   if defined __GNUC__ && defined GNU_STRICT_ANSI_ORIGINALLY_NOT_DEFINED
#       define __STRICT_ANSI__ ""
#   endif // !(defined(__GNUC__) && defined(__STRICT_ANSI__))
#else  // Unknown OS.
#   error Unknown operating system. Consider contributing support.
#endif // Unknown OS.

namespace JOSHUA_ROWE_PERSISTENT_STREAMS
{

JrPs_fdpstreambuf::JrPs_fdpstreambuf()
    :fd(-1)
{
}

JrPs_fdpstreambuf::JrPs_fdpstreambuf(int afd, int aflags)
    :JrPs_pstreambuf    (aflags)
    ,fd         (afd)
{
}

JrPs_fdpstreambuf::~JrPs_fdpstreambuf()
{
// GWC suppressed the next line under the assumption it should always be closed
//  if (flags & JrPs_pstream::xxclose)
    close();
}

void    JrPs_fdpstreambuf::close()
{
  if (::close(fd) == -1)
    err = errno;
  else
    err = 0;
  fd    = -1;
}

void    JrPs_fdpstreambuf::open(int afd, int aflags)
{
  if (flags & JrPs_pstream::xxclose)
    close();
  fd    = afd;
  flags = aflags;
}

void*   JrPs_fdpstreambuf::read(void* d, int l)
{
  if (error())
    return  0;
  if (::read(fd, d, l) == -1)
    err = errno;
  else
    err = 0;
  if (error())
    return  0;
  else
    return  d;
}

void    JrPs_fdpstreambuf::write(const void* d, int l)
{
  if (error())
    return;
  if (::write(fd, d, l) == -1)
    err = errno;
  else
    err = 0;
}

JrPs_fdpstream::JrPs_fdpstream()
:   JrPs_pstream(0, 0)
{
}

JrPs_fdpstream::JrPs_fdpstream(int afd, int aflags)
:   JrPs_pstream(0, flags)
{
  open(afd, aflags);
}

void    JrPs_fdpstream::open(int afd, int aflags)
{
//  if ((buf != 0) & (flags & xxkill))  // pretty sure this is incorrect
  if ((buf != 0) && (flags & xxkill))
    delete  buf;
  buf   = new JrPs_fdpstreambuf(afd, aflags);
  flags = aflags;
  flags |= xxkill;
}

JrPs_ifdpstream::JrPs_ifdpstream()
:   JrPs_fdpstream(0, 0)
{
}

JrPs_ifdpstream::JrPs_ifdpstream(int afd, int aflags)
:   JrPs_fdpstream(afd, aflags)
{
}

JrPs_ofdpstream::JrPs_ofdpstream()
:   JrPs_fdpstream(0, 0)
{
}

JrPs_ofdpstream::JrPs_ofdpstream(int afd, int aflags)
:   JrPs_fdpstream(afd, aflags)
{
}

JrPs_iofdpstream::JrPs_iofdpstream()
:   JrPs_fdpstream(0, 0)
{
}

JrPs_iofdpstream::JrPs_iofdpstream(int afd, int aflags)
:   JrPs_fdpstream(afd, aflags | xxread | xxwrite)
{
}

JrPs_fpstreambuf::JrPs_fpstreambuf(const char* aname, int aflags, int amode)
:   JrPs_fdpstreambuf()
{
  open(aname, aflags, amode);
}

JrPs_fpstreambuf::~JrPs_fpstreambuf()
{
}

// TODO ?? KLUDGE

#if 0 && defined __GNUC__
#   define O_BINARY _O_BINARY
#   define O_WRONLY _O_WRONLY
#   define O_RDONLY _O_RDONLY
#   define O_APPEND _O_APPEND
#   define O_TRUNC  _O_TRUNC
#   define O_EXCL   _O_EXCL
#   define O_CREAT  _O_CREAT
#endif // __GNUC__

// COMPILER !! (all compilers) Make certain nonstandard functions available.
// TODO ?? We're going to a lot of trouble for three crummy symbols.
// Maybe we should instead define _O_BINARY and prototype _fileno() and
// _setmode() with simpler conditions--the way _setmode() is treated
// for borland below.
// INELEGANT !! Why not do this in "platform_dependent.hpp"?

#ifdef LMI_MSW
#   if defined __GNUC__ && defined __STRICT_ANSI__
#       define GNU_STRICT_ANSI_ORIGINALLY_NOT_DEFINED
#       undef __STRICT_ANSI__
#   endif // !(defined(__GNUC__) && defined(__STRICT_ANSI__))
//  for _O_BINARY, _fileno(), _setmode()
#   include <io.h>
#   include <fcntl.h>
#   if defined __GNUC__ && defined GNU_STRICT_ANSI_ORIGINALLY_NOT_DEFINED
#       define __STRICT_ANSI__ ""
#   endif // !(defined(__GNUC__) && defined(__STRICT_ANSI__))
#   if defined __BORLANDC__
#       define _setmode setmode
#   endif // __BORLANDC__
#   include <stdio.h>
#endif

void    JrPs_fpstreambuf::open(const char* aname, int aflags, int amode)
{

  // GWC: changed from
  //    ind fd = ::open(...
  // which I assume is an oversight because it shadows a class member
  unsigned int inspectable_flags = // OOPS aflags | // GWC added aflags |
#ifdef O_BINARY
             O_BINARY |
#endif
             ((aflags & JrPs_pstream::xxwrite) ? O_WRONLY : 0) |
             ((aflags & JrPs_pstream::xxread)  ? O_RDONLY : 0) |
             ((aflags & JrPs_pstream::xxappen) ? O_APPEND : 0) |
             ((aflags & JrPs_pstream::xxtrunc) ? O_TRUNC  : 0) |
             ((aflags & JrPs_pstream::xxexcl)  ? O_EXCL   : 0) |
             ((aflags & JrPs_pstream::xxcreat) ? O_CREAT  : 0)
             ;
  fd    = ::open(aname,
             inspectable_flags,
/*
             O_BINARY |
             ((aflags & JrPs_pstream::xxwrite) ? O_WRONLY : 0) |
             ((aflags & JrPs_pstream::xxread)  ? O_RDONLY : 0) |
             ((aflags & JrPs_pstream::xxappen) ? O_APPEND : 0) |
             ((aflags & JrPs_pstream::xxtrunc) ? O_TRUNC  : 0) |
             ((aflags & JrPs_pstream::xxexcl)  ? O_EXCL   : 0) |
             ((aflags & JrPs_pstream::xxcreat) ? O_CREAT  : 0),
*/
             amode);
  if (fd == -1)
    error(errno);
  else
    error(0);
  JrPs_fdpstreambuf::open(fd, aflags | JrPs_pstream::xxclose);
}

JrPs_fpstream::JrPs_fpstream()
{
}

JrPs_fpstream::JrPs_fpstream(const char* aname, int aflags, int amode)
{
  open(aname, aflags, amode);
}

JrPs_fpstream::~JrPs_fpstream()
{
}

void    JrPs_fpstream::open(const char* aname, int aflags, int amode)
{
  if (buf != 0)
    delete  buf;
  buf   = new JrPs_fpstreambuf(aname, aflags, amode);
  flags = aflags;
}

JrPs_ifpstream::JrPs_ifpstream()
{
}

JrPs_ifpstream::JrPs_ifpstream(const char* aname, int aflags, int amode)
{
  open(aname, aflags | xxread, amode);
}

void JrPs_ifpstream::open(const char* aname, int aflags, int amode)
{
    JrPs_fpstream::open(aname, aflags, amode);
    // GWC: add this header for compatibility with borland pstreams
    char bi_header[5];
    readbytes(bi_header, 5);
}

JrPs_ofpstream::JrPs_ofpstream()
{
}

JrPs_ofpstream::JrPs_ofpstream(const char* aname, int aflags, int amode)
{
  open(aname, aflags | xxwrite, amode);
}

void JrPs_ofpstream::open(const char* aname, int aflags, int amode)
{
    JrPs_fpstream::open(aname, aflags, amode);
    // GWC: add this header for compatibility with borland pstreams
    const char bi_header[5] = {0x03a, 0x01, 0x01, 0x0, 0x0};
    writebytes((const void*)bi_header, 5);
}

JrPs_iofpstream::JrPs_iofpstream()
{
}

JrPs_iofpstream::JrPs_iofpstream(const char* aname, int aflags, int amode)
{
  JrPs_fpstream::open(aname, aflags | xxwrite | xxappen | xxread, amode);
  // GWC: TODO ?? What would we do about the borland header here?
}

}   // namespace JOSHUA_ROWE_PERSISTENT_STREAMS

