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
// and in any later years given in the copyright notice above as
// detailed in ChangeLog.

/* pios.cc
 * Written by Joshua Rowe
 * Definitions for the streamable class library
 */

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_pios.hpp" // GWC arbitrarily changed .hh to .hpp

#include <cstring>

namespace JOSHUA_ROWE_PERSISTENT_STREAMS
{

JrPs_pstream::JrPs_pstream(JrPs_pstreambuf *abuf, int aflags)
    :buf        (abuf)
    ,curid      (1)
    ,err        (0)
    ,flags      (aflags)
    ,streamed   (0)
{
}

JrPs_pstream::~JrPs_pstream()
{
  close();
  // GWC: Perhaps the intention was to pass creation flags including
  // xxkill to ctors of intermediate classes; that does not occur,
  // so close() never deletes buf.
  delete    buf;
  buf       = 0;
}

void    JrPs_pstream::addstreamed(const JrPs_pstreamable* obj)
{
  streamed  = new JrPs_pstreamed(curid++, streamed, static_cast<const void*>(obj));
}

void    JrPs_pstream::close()
{
  const JrPs_pstreamed *    p   = streamed;
  const JrPs_pstreamed *    q;
  for(; p != 0; p = q)
    {
      q = p->next;
      delete    p;
    }
  if (buf != 0)
    {
      if (flags & xxkill)
    {
      delete    buf;
      buf       = 0;
    }
      else if (flags & xxclose)
    {
      buf->close();
      error(buf->error());
    }
    }
  else
    error(peNoBuffer);
  curid     = 1;
  streamed  = 0;
  err       = 0;
}

void    JrPs_pstream::error(int aerror)
{
  err   = aerror;
}

int JrPs_pstream::error()
{
  return    err;
}

// GWC added: compatibility with BI
int JrPs_pstream::good()
{
  return 0 == err;
}

int JrPs_pstream::lookup(const void* q)
{
  const JrPs_pstreamed *    p   = streamed;
  for (; p != 0; p = p->next)
    if (p->p == q)
      break;
  if (p)
    return          p->id;  // p = 0 if not found, otherwise
                // it points to the JrPs_pstreamreg
  return    0;
}

const JrPs_pstreamed *  JrPs_pstream::lookup(int aid)
{
  const JrPs_pstreamed *    p   = streamed;
  for (; p != 0; p = p->next)
    if (p->id == aid)
      break;
  return    p;
}

int JrPs_pstream::operator!()
{
  return    err;
}

JrPs_opstream::JrPs_opstream(JrPs_pstreambuf *abuf, int aflags)
:   JrPs_pstream(abuf, aflags)
{
}

void    JrPs_opstream::writebyte(char c)
{
  writebytes((const void*) &c, sizeof(c));
}

void    JrPs_opstream::writebytes(const void* d, int l)
{
  if (error())
    return;
  if (buf != 0)
    buf->write(d, l);
  else
    error(peNoBuffer);
  error(buf->error());
}

void    JrPs_opstream::writeint(int i)
{
  writebytes(&i, sizeof(i));
}

void    JrPs_opstream::writeobj(const JrPs_pstreamable * obj)
{
  int   index;
  if (obj == 0)
    writebyte(JrPs_pstream::ptNULL);
  else if ((index = lookup(static_cast<const void*>(obj))) != 0)
    {
      writebyte(JrPs_pstream::ptIndex);
      writeint(index);
    }
  else
    {
      writebyte(JrPs_pstream::ptObject);
                // Write the object Prefix
      writebyte('[');
      writestring(obj->streamableName());
                // Write the object data
      addstreamed(obj);
      obj->write(*this);
                // Write the object suffix
      writebyte(']');
    }
}

// GWC added: see comment under
//   JrPs_opstream& operator<< (JrPs_opstream & os, const JrPs_pstreamable& x)
void    JrPs_opstream::writeref(const JrPs_pstreamable * obj)
{
                // Write the object Prefix
      writebyte('[');
      writestring(obj->streamableName());
                // Write the object data
      addstreamed(obj);
      obj->write(*this);
                // Write the object suffix
      writebyte(']');
}

void    JrPs_opstream::writestring(const char* s)
{
  int   l   = std::strlen(s);
  writeint(l);
  char* temp = new char[1 + l];
  std::strcpy(temp, s);
  writebytes(reinterpret_cast<void*>(temp), l);
  delete[]temp;
}

JrPs_opstream& operator<< (JrPs_opstream & os, bool c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, char c)
{
  os.writebyte(c);
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, signed char c)
{
  os.writebyte(c);
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, unsigned char c)
{
  os.writebyte(c);
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, const char* s)
{
  os.writestring(s);
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, signed char const* s)
{
  os.writestring(reinterpret_cast<const char*>(s));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, unsigned char const* s)
{
  os.writestring(reinterpret_cast<const char*>(s));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, std::string const& s)
{
  os.writestring(const_cast<char*>(s.c_str()));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, short int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, unsigned short int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, unsigned int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, long int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, unsigned long int c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, float c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, double c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, long double c)
{
  os.writebytes(&c, sizeof(c));
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, const JrPs_pstreamable& x)
{
//  os.writeobj(&x);    // GWC replaced this with the following
// GWC: In the original code, this function did the same thing as
//  JrPs_opstream& operator<< (JrPs_opstream & os, const JrPs_pstreamable* x)
// although the corresponding operator>> implementations differ
// for reference vs. pointer arguments.
  os.writeref(&x);
  return    os;
}

JrPs_opstream& operator<< (JrPs_opstream & os, const JrPs_pstreamable* x)
{
  os.writeobj(x);
  return    os;
}

JrPs_ipstream::JrPs_ipstream(JrPs_pstreambuf *abuf, int aflags)
:   JrPs_pstream(abuf, aflags)
{
}

char    JrPs_ipstream::readbyte()
{
  char      c;
  readbytes(&c, sizeof(c));
  return    c;
}

const void* JrPs_ipstream::readbytes(void* d, int l)
{
  if (error())
    return  0;
  const void*   x   = 0;
  if (buf != 0)
    x   = buf->read(d, l);
  else
    error(peNoBuffer);
  return    x;
}

int JrPs_ipstream::readint()
{
  int       i;
  readbytes(&i, sizeof(i));
  return    i;
}

void*   JrPs_ipstream::readobj(JrPs_pstreamreg *d, JrPs_pstreamable *m)
{
  if (error())
    return  0;
  if (m == 0)
    m   = d->builder();
  streamed  = new JrPs_pstreamed(curid++, streamed, ((char *)m - d->delta));
  void* q   = m->read(*this);
  readbyte();
  if (error())
    return  0;
  return    q;
}

char *  JrPs_ipstream::readstring()
{
  int       l   = readint();
  char *    s   = new char[l+1];
  readbytes(s, l);
  s[l]      = 0;
  if (error())
    {
      delete[]  s;
      s     = 0;
    }
  return    s;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, bool &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, char &c)
{
  c = is.readbyte();
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, signed char &c)
{
  c = is.readbyte();
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, unsigned char &c)
{
  c = is.readbyte();
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, char * &s)
{
  s = is.readstring();
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, signed char * &s)
{
  s = reinterpret_cast<signed char*>(is.readstring());
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, unsigned char * &s)
{
  s = reinterpret_cast<unsigned char*>(is.readstring());
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, std::string& s)
{
  char *    c   = is.readstring();
  s = c;
  delete[]  c;

  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, short int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, unsigned short int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, unsigned int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, long int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, unsigned long int &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, float &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, double &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, long double &c)
{
  is.readbytes(&c, sizeof(c));
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, JrPs_pstreamable & x)
{
  is.readbyte();
//  is.readbyte();  // GWC: TODO ?? Isn't this second call erroneous?
  char *    c   = is.readstring();
  JrPs_pstreamreg * d   = JrPs_pstreamreg::findclass(c);
  delete[]  c;
  is.readobj(d, &x);
  return    is;
}

JrPs_ipstream& operator>> (JrPs_ipstream & is, const void*& x)
{
  char      c   = is.readbyte();
  switch (c)
    {
    default:
      break;
    case JrPs_pstream::ptNULL:
      x     = 0;
      return    is;
      // break;
    case JrPs_pstream::ptIndex:
      x     = static_cast<const void*>(is.lookup(is.readint())->p);
      return    is;
      // break;
    case JrPs_pstream::ptObject:
      is.readbyte();
    case 91:
      char *        d   = is.readstring();
      JrPs_pstreamreg * r   = JrPs_pstreamreg::findclass(d);
      delete[]  d;
      x = is.readobj(r, 0);
      break;
    }
  return    is;
}

JrPs_pstreamed::JrPs_pstreamed(int aid, const JrPs_pstreamed* anext, const void* ap)
    :id     (aid)
    ,next   (anext)
    ,p      (ap)
{
}

JrPs_pstreamable::JrPs_pstreamable()
{
}

JrPs_pstreamable::JrPs_pstreamable(JrPs_pstreamableInit)
{
}

JrPs_pstreamable::~JrPs_pstreamable()
{
}

JrPs_pstreamreg *   pstreamreged    = 0;

JrPs_pstreamreg::JrPs_pstreamreg(const char* aname, BUILDER abuilder, std::size_t adelta)
    :name       (aname)
    ,next       (pstreamreged)  // TODO ??
    ,builder    (abuilder)
    ,delta      (adelta)
{
  // GWC TODO ?? Is pstreamreged static so that it holds all streamable classes?
//  next        = pstreamreged; // TODO ??
  pstreamreged  = this;
}

JrPs_pstreamreg *   JrPs_pstreamreg::findclass(const char* aname)
{
  JrPs_pstreamreg * p   = pstreamreged;
  for (; p != 0; p = p->next)
    if (std::strcmp(aname, p->name) == 0)
      break;
  return    p;      // p = 0 if not found, otherwise
                // it points to the JrPs_pstreamreg
}

JrPs_pstreambuf::JrPs_pstreambuf(int aflags)
    :err    (0) // GWC: presumably this is to be initialized to zero
    ,flags  (aflags)
{
  flags = aflags;
}

JrPs_pstreambuf::~JrPs_pstreambuf()
{
  if (flags & JrPs_pstream::xxclose)
    close();
}

void    JrPs_pstreambuf::close()
{
}

int JrPs_pstreambuf::error()
{
  return    err;
}

void    JrPs_pstreambuf::error(int aerror)
{
  err   = aerror;
}

}   // namespace JOSHUA_ROWE_PERSISTENT_STREAMS

