// Joshua Rowe's "Really cool persistent object stream library".
//
// Copyright (C) 2000, 2005 Gregory W. Chicares.
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

// $Id: ihs_fpios.hpp,v 1.3 2005-11-03 04:53:55 chicares Exp $

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
// and in any later years given in the copyright notice above as
// detailed in ChangeLog.

/* fpios.hh
 * Written by Joshua Rowe
 * File-based persistent object streams
 */

#ifndef ihs_fpios_hpp
#define ihs_fpios_hpp

#include "config.hpp"

#include "ihs_pios.hpp" // GWC arbitrarily changed .hh to .hpp

#include "expimp.hpp"

#include <fcntl.h>
#ifdef LMI_MSW
#   include <io.h>
#elif defined(LMI_POSIX)
#   include <sys/types.h>
#endif // LMI_ platform
#include <sys/stat.h>

namespace JOSHUA_ROWE_PERSISTENT_STREAMS
{

class   LMI_EXPIMP JrPs_fdpstreambuf;
class   LMI_EXPIMP JrPs_fdpstream;
class   LMI_EXPIMP JrPs_ifdpstream;
class   LMI_EXPIMP JrPs_ofdpstream;
class   LMI_EXPIMP JrPs_fpstreambuf;
class   LMI_EXPIMP JrPs_fpstream;
class   LMI_EXPIMP JrPs_ifpstream;
class   LMI_EXPIMP JrPs_ofpstream;
class   LMI_EXPIMP JrPs_iofpstream;

class LMI_EXPIMP JrPs_fdpstreambuf  : public JrPs_pstreambuf
{
protected:
  int       fd;
public:
  JrPs_fdpstreambuf();
  JrPs_fdpstreambuf(int afd, int aflags);
// GWC made this dtor virtual--OOPS!
//  virtual ~JrPs_fdpstreambuf();
  ~JrPs_fdpstreambuf();
  virtual void  close();
  void      open(int afd, int aflags = 0);
  virtual void* read(void* d, int l);
  virtual void  write(const void* d, int l);
};

class LMI_EXPIMP JrPs_fdpstream : virtual public JrPs_pstream
{
  JrPs_fdpstream(const JrPs_fdpstream&);
  JrPs_fdpstream& operator= (const JrPs_fdpstream&);
protected:
public:
  JrPs_fdpstream();
  JrPs_fdpstream(int afd, int aflags = 0);
  void      open(int afd, int aflags = 0);
};

class LMI_EXPIMP JrPs_ifdpstream    : virtual public JrPs_fdpstream, virtual public JrPs_ipstream
{
  JrPs_ifdpstream(const JrPs_ifdpstream&);
  JrPs_ifdpstream& operator= (const JrPs_ifdpstream&);
protected:
public:
  JrPs_ifdpstream();
  JrPs_ifdpstream(int afd, int aflags = 0);
};

class LMI_EXPIMP JrPs_ofdpstream    : virtual public JrPs_fdpstream, virtual public JrPs_opstream
{
  JrPs_ofdpstream(const JrPs_ofdpstream&);
  JrPs_ofdpstream& operator= (const JrPs_ofdpstream&);
protected:
public:
  JrPs_ofdpstream();
  JrPs_ofdpstream(int afd, int aflags = 0);
};

class LMI_EXPIMP JrPs_iofdpstream   : virtual public JrPs_ifdpstream, virtual public JrPs_ofdpstream
{
  JrPs_iofdpstream(const JrPs_iofdpstream&);
  JrPs_iofdpstream& operator= (const JrPs_iofdpstream&);
protected:
public:
  JrPs_iofdpstream();
  JrPs_iofdpstream(int afd, int aflags = 0);
};

class LMI_EXPIMP JrPs_fpstreambuf   : public JrPs_fdpstreambuf
{
protected:
public:
  JrPs_fpstreambuf();
  virtual ~JrPs_fpstreambuf();
  JrPs_fpstreambuf(const char* aname, int aflags, int amode = 0644);
  void      open(const char* aname, int aflags, int amode = 0644);
};

class LMI_EXPIMP JrPs_fpstream  : virtual public JrPs_fdpstream
{
  JrPs_fpstream(const JrPs_fpstream&);
  JrPs_fpstream& operator= (const JrPs_fpstream&);
protected:
public:
  JrPs_fpstream();
  virtual ~JrPs_fpstream();
  JrPs_fpstream(const char* aname, int aflags, int amode = 0644);
  void      open(const char* aname, int aflags, int amode = 0644);
};

class LMI_EXPIMP JrPs_ifpstream : virtual public JrPs_fpstream, virtual public JrPs_ifdpstream
{
  JrPs_ifpstream(const JrPs_ifpstream&);
  JrPs_ifpstream& operator= (const JrPs_ifpstream&);
protected:
public:
  JrPs_ifpstream();
  JrPs_ifpstream(const char* aname, int aflags = 0, int amode = 0644);
  void      open(const char* aname, int aflags, int amode = 0644);
};

class LMI_EXPIMP JrPs_ofpstream : virtual public JrPs_fpstream, virtual public JrPs_ofdpstream
{
  JrPs_ofpstream(const JrPs_ofpstream&);
  JrPs_ofpstream& operator= (const JrPs_ofpstream&);
protected:
public:
  JrPs_ofpstream();
  JrPs_ofpstream(const char* aname, int aflags = JOSHUA_ROWE_PERSISTENT_STREAMS::JrPs_pstream::xxcreat, int amode = 0644);
//  JrPs_ofpstream(const char* aname, int aflags = 0x0020, int amode = 0644);
  void      open(const char* aname, int aflags, int amode = 0644);
};

class LMI_EXPIMP JrPs_iofpstream    : virtual public JrPs_ifpstream, virtual public JrPs_ofpstream
{
  JrPs_iofpstream(const JrPs_iofpstream&);
  JrPs_iofpstream& operator= (const JrPs_iofpstream&);
protected:
public:
  JrPs_iofpstream();
  JrPs_iofpstream(const char* aname, int aflags = JOSHUA_ROWE_PERSISTENT_STREAMS::JrPs_pstream::xxcreat, int amode = 0644);
//  JrPs_iofpstream(const char* aname, int aflags = 0x0020, int amode = 0644);
};

}   // namespace JOSHUA_ROWE_PERSISTENT_STREAMS
#endif // ihs_fpios_hpp

