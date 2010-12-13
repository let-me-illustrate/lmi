#!/bin/sh

# Install como C++ on msw, with MinGW gcc-2.95.3-5 as its C compiler.

# Copyright (C) 2010 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id$

set -v

  # Prerequisites: a normal lmi installation, with 'unzip' and 'cpio'
  # added to the usual Cygwin packages; and the Comeau compiler.

cache_dir=/cache_for_lmi/downloads

mkdir --parents /opt/lmi/como
cd /opt/lmi/como

  # Archive name is personalized by vendor. A copy of it is assumed
  # to have been saved in $cache_dir .

cp --preserve $cache_dir/200043101* .
unzip -u 200043101beta*

  # According to this page:
  #   http://comeaucomputing.com/4.3.0/minor/win95+/como43101.html
  # configuration is simpler when "libcomo is installed into the
  # Comeau C++ directory tree in a directory named libcomo".
  #
  # As with redhat, don't use the '-c' flag for 'cpio' with Cygwin;
  # see:
  #   http://www.comeaucomputing.com/libcomo/36.html

como_server=ftp://ftp.comeaucomputing.com/pub/
libcomo_archive=libcomo36.Z
cd $cache_dir && [ -e $libcomo_archive ] || wget --timestamping $como_server/$libcomo_archive

mkdir --parents /opt/lmi/como/libcomo
cd /opt/lmi/como/libcomo

cp --preserve $cache_dir/$libcomo_archive .
uncompress --force $libcomo_archive
cpio -ivdIlibcomo36
rm libcomo36

  # Use MinGW gcc-2.95.3-5 as the underlying C compiler. This page:
  #   http://comeaucomputing.com/4.3.0/minor/win95+/como43101.html
  # says a suitable gcc is available here:
  #   http://prdownloads.sourceforge.net/mingw/mingw-1.0.1-20010726.tar.gz
  # However, sourceforge has been redisorganized, and that file has
  # been either hidden or removed. The closest thing available on
  # sourceforge is only slightly different:
  #   http://sourceforge.net/projects/mingw/files/MinGW/BaseSystem/GCC/Version2/Current%20Release_%20gcc-2.95.3/
  # The original tarball is still available here:
  #   http://www2.cs.uidaho.edu/~jeffery/%77in32/mingw-1.0.1-20010726.tar.gz
  # and its md5sum:
  #   99b3cf9fb4a1b82654a4caf11ef1a429 *mingw-1.0.1-20010726.tar.gz
  # matches that of another copy that I downloaded from sourceforge
  # in 2004, so it's authentic. However, it's even better to use lmi's
  # own MinGW installer, which was validated against that tarball.
  #
  # It ought to be possible to use MinGW-3.4.5 instead with a few
  # changes to this script, viz.:
  #   make ... version=MinGW-20010726
  #   touch /opt/lmi/como/default.min345
  #   cd /opt/lmi/como/libcomo && ./makeit.bat min345
  # but 'invalid option: --min345' is unexpectedly observed.

cd /opt/lmi/como
make \
       prefix=/opt/lmi/como/mingw_for_como \
    cache_dir=/cache_for_lmi/downloads \
      version=MinGW-20010726 \
  -f /lmi/src/lmi/install_mingw.make

  # Create patch for shadow headers. See:
  #   http://comeaucomputing.com/4.3.0/minor/win95+/como43101.html

cat >MinGW-20010726.patch <<\EOF
diff -U3 older/float.h newer/float.h
--- older/float.h	2001-07-26 01:51:11.000000000 +0000
+++ newer/float.h	2010-12-11 01:05:05.868059600 +0000
@@ -82,7 +82,7 @@
  * The characteristics of double.
  */
 #define DBL_DIG		15
-#define DBL_EPSILON	1.1102230246251568e-16
+#define DBL_EPSILON	2.2204460492503131e-16
 #define DBL_MANT_DIG	53
 #define DBL_MAX		1.7976931348623157e+308
 #define DBL_MAX_EXP	1024
@@ -97,7 +97,7 @@
  * NOTE: long double is the same as double.
  */
 #define LDBL_DIG	15
-#define LDBL_EPSILON	1.1102230246251568e-16L
+#define LDBL_EPSILON	1.08420217248550443401e-19L
 #define LDBL_MANT_DIG	53
 #define LDBL_MAX	1.7976931348623157e+308L
 #define LDBL_MAX_EXP	1024
diff -U3 older/rpcdcep.h newer/rpcdcep.h
--- older/rpcdcep.h	2001-07-26 01:46:06.000000000 +0000
+++ newer/rpcdcep.h	2010-12-11 01:05:05.914936700 +0000
@@ -81,7 +81,7 @@
 void __stdcall I_RpcRequestMutex(I_RPC_MUTEX*);
 void __stdcall I_RpcClearMutex(I_RPC_MUTEX);
 void __stdcall I_RpcDeleteMutex(I_RPC_MUTEX);
-__stdcall void *I_RpcAllocate(unsigned int);
+void __stdcall *I_RpcAllocate(unsigned int);
 void __stdcall I_RpcFree(void*);
 void __stdcall I_RpcPauseExecution(unsigned long);
 typedef void(__stdcall *PRPC_RUNDOWN) (void*);
diff -U3 older/winnt.h newer/winnt.h
--- older/winnt.h	2001-07-26 01:46:12.000000000 +0000
+++ newer/winnt.h	2010-12-11 01:05:05.946188100 +0000
@@ -2509,24 +2509,15 @@
 } REPARSE_POINT_INFORMATION, *PREPARSE_POINT_INFORMATION;
 extern __inline PVOID GetCurrentFiber(void)
 {
-    void* ret;
-    __asm__ volatile ("\n\
-	      movl	%%fs:0x10,%0\n\
-	      movl	(%0),%0\n\
-	      " : "=r" (ret) /* allow use of reg eax, ebx, ecx, edx, esi, edi */
-	        :
-	      );
-    return ret;
+    __asm mov eax, fs:[0x10]
+    return 0;
 }
 extern __inline PVOID GetFiberData(void)
 {
-    void* ret;
-    __asm__ volatile ("\n\
-	      movl	%%fs:0x10,%0\n\
-	      " : "=r" (ret) /* allow use of reg eax,ebx,ecx,edx,esi,edi */
-	        :
-		);
-    return ret;
+    __asm { mov eax, fs:[0x10]
+            mov eax,[eax]
+          }
+    return 0;
 }
 #endif
 #ifdef __cplusplus
diff -U3 older/winsock.h newer/winsock.h
--- older/winsock.h	2001-07-26 01:46:13.000000000 +0000
+++ newer/winsock.h	2010-12-11 01:05:05.946188100 +0000
@@ -429,7 +429,7 @@
 int PASCAL getsockname(SOCKET,struct sockaddr*,int*);
 int PASCAL getsockopt(SOCKET,int,int,char*,int*);
 unsigned long PASCAL inet_addr(const char*);
-PASCAL char *inet_ntoa(struct in_addr);
+char * PASCAL inet_ntoa(struct in_addr);
 int PASCAL listen(SOCKET,int);
 int PASCAL recv(SOCKET,char*,int,int);
 int PASCAL recvfrom(SOCKET,char*,int,int,struct sockaddr*,int*);
@@ -438,12 +438,12 @@
 int PASCAL setsockopt(SOCKET,int,int,const char*,int);
 int PASCAL shutdown(SOCKET,int);
 SOCKET PASCAL socket(int,int,int);
-PASCAL struct hostent *gethostbyaddr(const char*,int,int);
-PASCAL struct hostent *gethostbyname(const char*);
-PASCAL struct servent *getservbyport(int,const char*);
-PASCAL struct servent *getservbyname(const char*,const char*);
-PASCAL struct protoent *getprotobynumber(int);
-PASCAL struct protoent *getprotobyname(const char*);
+struct hostent * PASCAL gethostbyaddr(const char*,int,int);
+struct hostent * PASCAL gethostbyname(const char*);
+struct servent * PASCAL getservbyport(int,const char*);
+struct servent * PASCAL getservbyname(const char*,const char*);
+struct protoent * PASCAL getprotobynumber(int);
+struct protoent * PASCAL getprotobyname(const char*);
 int PASCAL WSAStartup(WORD,LPWSADATA);
 int PASCAL WSACleanup(void);
 void PASCAL WSASetLastError(int);
EOF

  # Create shadow headers.

cd /opt/lmi/como/include/min
for z in *; do cp --preserve /opt/lmi/como/mingw_for_como/include/$z .; done
patch </opt/lmi/como/MinGW-20010726.patch

  # Configure como, in order to build its C++ standard library.

touch /opt/lmi/como/default.min
PATH=/opt/lmi/como/bin:/opt/lmi/como/mingw_for_como/bin:$PATH
export COMO_MIN_INCLUDE=/opt/lmi/como/mingw_for_como/include

  # Build libcomo.

cd /opt/lmi/como/libcomo && ./makeit.bat min
grep 'warning:\|error:' makeout

  # Patch libcomo.
  #
  # This patch must be applied after building libcomo, and before
  # building lmi. The code commented out here may be a kludge for
  # getting <istream> to work without including <algorithm>.

cd /opt/lmi/como/libcomo
cat >libcomo-36.patch <<\EOF
diff -U3 older/stl_config.h newer/stl_config.h
--- older/stl_config.h	2008-05-29 10:48:28.000000000 +0000
+++ newer/stl_config.h	2010-12-10 14:40:28.000000000 +0000
@@ -608,9 +608,9 @@
      inline long double log10l(long double arg) { return log10(arg); }
 #endif

-namespace std {
- inline int min(int a, int b) { return a>b ? b : a; }
-}
+//namespace std {
+// inline int min(int a, int b) { return a>b ? b : a; }
+//}
 #endif

 #ifdef __MWERKS__
EOF

patch <libcomo-36.patch

echo Finished installing como. >/dev/tty

