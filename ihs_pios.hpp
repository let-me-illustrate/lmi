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

/* pios.hh
 * Written by Joshua Rowe
 * Really cool persistent object stream library
 */

#ifndef ihs_pios_hpp
#define ihs_pios_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <cstddef> // std::size_t

//#ifndef __BORLANDC__
#if 1
#   include <string>
//using std::string;
#elif defined(__BORLANDC__) && __BORLANDC__ < 0x0550
#   include <cstring.h>
    // This BC++5.02 class uses upper case NPOS, but
    // ISO std C++ uses lower case npos .
    #define npos (std::size_t(-1))
    // This would be preferable, but the borland class doesn't
    // have string::size_type :
    // static const size_type npos = static_cast<size_type>(-1);
#elif defined(__BORLANDC__) && 0x0550 <= __BORLANDC__
#   include </bc5/include/cstring.h>
#endif

namespace JOSHUA_ROWE_PERSISTENT_STREAMS
{

// GWC: replaced with void*
//#ifndef   pvoid
//typedef   void *  pvoid;
//#endif

class JrPs_pstream;
class JrPs_opstream;
class JrPs_ipstream;
class JrPs_pstreamed;
class JrPs_pstreamable;
class JrPs_pstreamreg;
class JrPs_pstreambuf;

typedef JrPs_pstreamable * (*BUILDER)();

enum    JrPs_pstreamableInit
{
  JrPs_pstreamableinit
};

class LMI_SO JrPs_pstream
{
protected:
  JrPs_pstreambuf*  buf;
  int           curid;
  int           err;
  int           flags;
  JrPs_pstreamed* streamed;
  JrPs_pstream(const JrPs_pstream&);
  JrPs_pstream& operator= (const JrPs_pstream&);
public:
  enum  psFlags {
    xxclose = 0x0001,
    xxread  = 0x0002,
    xxwrite = 0x0004,
    xxtrunc = 0x0008,
    xxexcl  = 0x0010,
    xxcreat = 0x0020,
    xxappen = 0x0040,
    xxkill  = 0x0080
    };
  enum  ptTypes {
      ptNULL,
      ptIndex,
      ptObject
    };
  enum  peError {
      peNULL,
      peNotRegistered,
      peNoBuffer
    };
  JrPs_pstream(JrPs_pstreambuf *abuf = 0, int aflags = 0);
  virtual   ~JrPs_pstream();
  void      addstreamed(const JrPs_pstreamable* obj);
  void      close();
  void      error(int aerror);
  int       error();
  int       good(); // GWC added: compatibility with BI
  int       lookup(const void* q); // Look up an already written object by
                  // its pointer.
  const JrPs_pstreamed* lookup(int id);   // Look up an already read object
                  // by its id number
  int       operator !();     // Returns err
};

class LMI_SO JrPs_opstream  : virtual public JrPs_pstream
{
  JrPs_opstream(const JrPs_opstream&);
  JrPs_opstream& operator= (const JrPs_opstream&);
protected:
public:
  JrPs_opstream(JrPs_pstreambuf *abuf = 0, int aflags = 0);
  void      writebyte(char c);
  virtual void  writebytes(const void* d, int l);
  void      writeint(int i);
  void      writeobj(const JrPs_pstreamable* obj);
  void      writeref(const JrPs_pstreamable* obj);  // GWC added: see .cpp file

  void      writestring(const char* s);

friend  JrPs_opstream & operator <<(JrPs_opstream& os, const JrPs_pstreamable& x);
friend  JrPs_opstream & operator <<(JrPs_opstream& os, const JrPs_pstreamable* x);

    // for POD types
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     bool    );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     char    );
friend JrPs_opstream& operator<<    (JrPs_opstream& os, signed              char    );
friend JrPs_opstream& operator<<    (JrPs_opstream& os, unsigned            char    );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,             short   int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os, unsigned    short   int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os, unsigned            int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,             long    int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os, unsigned    long    int     );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     float   );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     double  );
friend JrPs_opstream& operator<<    (JrPs_opstream& os,             long    double  );

    // for C strings
friend JrPs_opstream& operator<<    (JrPs_opstream& os,                     char const*);
friend JrPs_opstream& operator<<    (JrPs_opstream& os, signed              char const*);
friend JrPs_opstream& operator<<    (JrPs_opstream& os, unsigned            char const*);

    // for C++ strings
friend JrPs_opstream& operator<<    (JrPs_opstream& os, std::string const&);

};

class LMI_SO JrPs_ipstream  : virtual public JrPs_pstream
{
  JrPs_ipstream(const JrPs_ipstream&);
  JrPs_ipstream& operator= (const JrPs_ipstream&);
protected:
public:
  JrPs_ipstream(JrPs_pstreambuf *abuf = 0, int aflags = 0);
  char      readbyte();
  virtual const void*   readbytes(void* d, int l);
  int       readint();
  void*     readobj(JrPs_pstreamreg *d, JrPs_pstreamable *m);
  char* readstring();

friend  JrPs_ipstream & operator >>(JrPs_ipstream& is, JrPs_pstreamable &x);
friend  JrPs_ipstream & operator >>(JrPs_ipstream& is, const void* &x);

    // for POD types
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     bool    &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     char    &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, signed          char    &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, unsigned            char    &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,             short   int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, unsigned    short   int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, unsigned            int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,             long    int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, unsigned    long    int     &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     float   &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     double  &);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,             long    double  &);

    // for C strings
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is,                     char*&);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, signed          char*&);
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, unsigned            char*&);

    // for C++ strings
friend JrPs_ipstream& operator>>    (JrPs_ipstream& is, std::string&);

};

class   JrPs_pstreamed
{
friend  JrPs_opstream & operator <<(JrPs_opstream& os, const JrPs_pstreamable& x);
friend  JrPs_opstream & operator <<(JrPs_opstream& os, const JrPs_pstreamable* x);
friend  JrPs_ipstream & operator >>(JrPs_ipstream& is, JrPs_pstreamable &x);
friend  JrPs_ipstream & operator >>(JrPs_ipstream& is, void* &x);
friend class    JrPs_pstream;
friend class    JrPs_ipstream;
friend class    JrPs_opstream;
  JrPs_pstreamed(const JrPs_pstreamed&);
  JrPs_pstreamed& operator= (const JrPs_pstreamed&);
protected:
  int       id;
  const JrPs_pstreamed* next;
  JrPs_pstreamed(int aid, const JrPs_pstreamed* anext, const void* ap);
public:
  const void*       p;
};

class   JrPs_pstreamable
{
  friend class  JrPs_opstream;
  friend class  JrPs_ipstream;
  virtual const char*   streamableName() const  = 0;
protected:
  virtual void*     read(JrPs_ipstream& is) = 0;
  virtual void  write(JrPs_opstream& os)    const = 0;
public:
  JrPs_pstreamable();
  JrPs_pstreamable(JrPs_pstreamableInit);
  virtual ~JrPs_pstreamable();
};

class   JrPs_pstreamreg
{
  friend class  JrPs_opstream;
  friend class  JrPs_ipstream;
  JrPs_pstreamreg(const JrPs_pstreamreg&);
  JrPs_pstreamreg& operator= (const JrPs_pstreamreg&);
protected:
  const char*   name;
  JrPs_pstreamreg * next;
  BUILDER   builder;
  std::size_t    delta;
public:
  JrPs_pstreamreg(const char* aname, BUILDER abuilder, std::size_t adelta);
  static JrPs_pstreamreg *  findclass(const char* aname);
};

class   JrPs_pstreambuf
{
friend class    JrPs_pstream;
friend class    JrPs_opstream;
friend class    JrPs_ipstream;
protected:
  int       err;
  int       flags;
public:
  JrPs_pstreambuf(int aflags = 0);
  virtual   ~JrPs_pstreambuf();
  virtual void  close();
  int       error();
  void      error(int aerror);
  virtual void* read(void* d, int l)    = 0;
  virtual void  write(const void* d, int l) = 0;
};

#define JRPS_PSTREAM_DELTA(d) \
(std::size_t((JOSHUA_ROWE_PERSISTENT_STREAMS::JrPs_pstreamable *)(d *)1) - 1)

}   // namespace JOSHUA_ROWE_PERSISTENT_STREAMS

#endif // ihs_pios_hpp

