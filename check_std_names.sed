# Check for standard names referenced without 'std::' qualification.
#
# Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
#
# grep -n ^ *.?pp | sed -nf check_std_names.sed
#
# First, don't complain about the 'free' in 'free software'
/free software/d
# or other common phrases
/FILE:/d

# Expect many occurrences of these common words to be flagged:
# free puts gets time remove system signal rename exit raise log labs

# ctime is the name of a C++ system header as well as a C rtl function

# C++98 C.2.3 standard macros

# assert
# HUGE_VAL
# NULL
# SIGILL
# va_arg
# BUFSIZ
# LC_ALL
# NULL
# SIGINT
# va_end
# CLOCKS_PER_SEC
# LC_COLLATE
# NULL
# SIGSEGV
# va_start
# EDOM
# LC_CTYPE
# offsetof
# SIGTERM
# WCHAR_MAX
# EOF
# LC_MONETARY
# RAND_MAX
# SIG_DFL
# WCHAR_MIN
# ERANGE
# LC_NUMERIC
# SEEK_CUR
# SIG_ERR
# WEOF
# errno
# LC_TIME
# SEEK_END
# SIG_IGN
# WEOF
# EXIT_FAILURE
# L_tmpnam
# SEEK_SET
# stderr
# _IOFBF
# EXIT_SUCCESS
# MB_CUR_MAX
# setjmp
# stdin
# _IOLBF
# FILENAME_MAX
# NULL
# SIGABRT
# stdout
# _IONBF
# FOPEN_MAX
# NULL
# SIGFPE
# TMP_MAX

# C++98 C.2.4 standard values

# CHAR_BIT
# FLT_DIG
# INT_MIN
# MB_LEN_MAX
# CHAR_MAX
# FLT_EPSILON
# LDBL_DIG
# SCHAR_MAX
# CHAR_MIN
# FLT_MANT_DIG
# LDBL_EPSILON
# SCHAR_MIN
# DBL_DIG
# FLT_MAX
# LDBL_MANT_DIG
# SHRT_MAX
# DBL_EPSILON
# FLT_MAX_10_EXP
# LDBL_MAX
# SHRT_MIN
# DBL_MANT_DIG
# FLT_MAX_EXP
# LDBL_MAX_10_EXP
# UCHAR_MAX
# DBL_MAX
# FLT_MIN
# LDBL_MAX_EXP
# UINT_MAX
# DBL_MAX_10_EXP
# FLT_MIN_10_EXP
# LDBL_MIN
# ULONG_MAX
# DBL_MAX_EXP
# FLT_MIN_EXP
# LDBL_MIN_10_EXP
# USHRT_MAX
# DBL_MIN
# FLT_RADIX
# LDBL_MIN_EXP
# DBL_MIN_10_EXP
# FLT_ROUNDS
# LONG_MAX
# DBL_MIN_EXP
# INT_MAX
# LONG_MIN

# C++98 C.2.5 standard types

/[^:A-Za-z_]clock_t[^A-Za-z_]/p
/[^:A-Za-z_]ldiv_t[^A-Za-z_]/p
/[^:A-Za-z_]size_t[^A-Za-z_]/p
/[^:A-Za-z_]wctrans_t[^A-Za-z_]/p
/[^:A-Za-z_]div_t[^A-Za-z_]/p
/[^:A-Za-z_]mbstate_t[^A-Za-z_]/p
/[^:A-Za-z_]wctype_t[^A-Za-z_]/p
/[^:A-Za-z_]FILE[^A-Za-z_]/p
/[^:A-Za-z_]ptrdiff_t[^A-Za-z_]/p
/[^:A-Za-z_]fpos_t[^A-Za-z_]/p
/[^:A-Za-z_]sig_atomic_t[^A-Za-z_]/p
/[^:A-Za-z_]time_t[^A-Za-z_]/p
/[^:A-Za-z_]wint_t[^A-Za-z_]/p
/[^:A-Za-z_]jmp_buf[^A-Za-z_]/p
/[^:A-Za-z_]va_list[^A-Za-z_]/p

# C++98 C.2.6 standard structs

/[^:A-Za-z_]lconv[^A-Za-z_]/p
/[^:A-Za-z_]tm[^A-Za-z_]/p

# C++98 C.2.7 standard functions

/[^:A-Za-z_]abort[^A-Za-z_]/p
/[^:A-Za-z_]fmod[^A-Za-z_]/p
/[^:A-Za-z_]isupper[^A-Za-z_]/p
/[^:A-Za-z_]mktime[^A-Za-z_]/p
/[^:A-Za-z_]strftime[^A-Za-z_]/p
/[^:A-Za-z_]wcrtomb[^A-Za-z_]/p
/[^:A-Za-z_]abs[^A-Za-z_]/p
/[^:A-Za-z_]fopen[^A-Za-z_]/p
/[^:A-Za-z_]iswalnum[^A-Za-z_]/p
/[^:A-Za-z_]modf[^A-Za-z_]/p
/[^:A-Za-z_]strlen[^A-Za-z_]/p
/[^:A-Za-z_]wcscat[^A-Za-z_]/p
/[^:A-Za-z_]acos[^A-Za-z_]/p
/[^:A-Za-z_]fprintf[^A-Za-z_]/p
/[^:A-Za-z_]iswalpha[^A-Za-z_]/p
/[^:A-Za-z_]perror[^A-Za-z_]/p
/[^:A-Za-z_]strncat[^A-Za-z_]/p
/[^:A-Za-z_]wcschr[^A-Za-z_]/p
/[^:A-Za-z_]asctime[^A-Za-z_]/p
/[^:A-Za-z_]fputc[^A-Za-z_]/p
/[^:A-Za-z_]iswcntrl[^A-Za-z_]/p
/[^:A-Za-z_]pow[^A-Za-z_]/p
/[^:A-Za-z_]strncmp[^A-Za-z_]/p
/[^:A-Za-z_]wcscmp[^A-Za-z_]/p
/[^:A-Za-z_]asin[^A-Za-z_]/p
/[^:A-Za-z_]fputs[^A-Za-z_]/p
/[^:A-Za-z_]iswctype[^A-Za-z_]/p
/[^:A-Za-z_]printf[^A-Za-z_]/p
/[^:A-Za-z_]strncpy[^A-Za-z_]/p
/[^:A-Za-z_]wcscoll[^A-Za-z_]/p
/[^:A-Za-z_]atan[^A-Za-z_]/p
/[^:A-Za-z_]fputwc[^A-Za-z_]/p
/[^:A-Za-z_]iswdigit[^A-Za-z_]/p
/[^:A-Za-z_]putc[^A-Za-z_]/p
/[^:A-Za-z_]strpbrk[^A-Za-z_]/p
/[^:A-Za-z_]wcscpy[^A-Za-z_]/p
/[^:A-Za-z_]atan2[^A-Za-z_]/p
/[^:A-Za-z_]fputws[^A-Za-z_]/p
/[^:A-Za-z_]iswgraph[^A-Za-z_]/p
/[^:A-Za-z_]putchar[^A-Za-z_]/p
/[^:A-Za-z_]strrchr[^A-Za-z_]/p
/[^:A-Za-z_]wcscspn[^A-Za-z_]/p
/[^:A-Za-z_]atexit[^A-Za-z_]/p
/[^:A-Za-z_]fread[^A-Za-z_]/p
/[^:A-Za-z_]iswlower[^A-Za-z_]/p
/[^:A-Za-z_]puts[^A-Za-z_]/p
/[^:A-Za-z_]strspn[^A-Za-z_]/p
/[^:A-Za-z_]wcsftime[^A-Za-z_]/p
/[^:A-Za-z_]atof[^A-Za-z_]/p
/[^:A-Za-z_]free[^A-Za-z_]/p
/[^:A-Za-z_]iswprint[^A-Za-z_]/p
/[^:A-Za-z_]putwc[^A-Za-z_]/p
/[^:A-Za-z_]strstr[^A-Za-z_]/p
/[^:A-Za-z_]wcslen[^A-Za-z_]/p
/[^:A-Za-z_]atoi[^A-Za-z_]/p
/[^:A-Za-z_]freopen[^A-Za-z_]/p
/[^:A-Za-z_]iswpunct[^A-Za-z_]/p
/[^:A-Za-z_]putwchar[^A-Za-z_]/p
/[^:A-Za-z_]strtod[^A-Za-z_]/p
/[^:A-Za-z_]wcsncat[^A-Za-z_]/p
/[^:A-Za-z_]atol[^A-Za-z_]/p
/[^:A-Za-z_]frexp[^A-Za-z_]/p
/[^:A-Za-z_]iswspace[^A-Za-z_]/p
/[^:A-Za-z_]qsort[^A-Za-z_]/p
/[^:A-Za-z_]strtok[^A-Za-z_]/p
/[^:A-Za-z_]wcsncmp[^A-Za-z_]/p
/[^:A-Za-z_]bsearch[^A-Za-z_]/p
/[^:A-Za-z_]fscanf[^A-Za-z_]/p
/[^:A-Za-z_]iswupper[^A-Za-z_]/p
/[^:A-Za-z_]raise[^A-Za-z_]/p
/[^:A-Za-z_]strtol[^A-Za-z_]/p
/[^:A-Za-z_]wcsncpy[^A-Za-z_]/p
/[^:A-Za-z_]btowc[^A-Za-z_]/p
/[^:A-Za-z_]fseek[^A-Za-z_]/p
/[^:A-Za-z_]iswxdigit[^A-Za-z_]/p
/[^:A-Za-z_]rand[^A-Za-z_]/p
/[^:A-Za-z_]strtoul[^A-Za-z_]/p
/[^:A-Za-z_]wcspbrk[^A-Za-z_]/p
/[^:A-Za-z_]calloc[^A-Za-z_]/p
/[^:A-Za-z_]fsetpos[^A-Za-z_]/p
/[^:A-Za-z_]isxdigit[^A-Za-z_]/p
/[^:A-Za-z_]realloc[^A-Za-z_]/p
/[^:A-Za-z_]strxfrm[^A-Za-z_]/p
/[^:A-Za-z_]wcsrchr[^A-Za-z_]/p
/[^:A-Za-z_]ceil[^A-Za-z_]/p
/[^:A-Za-z_]ftell[^A-Za-z_]/p
/[^:A-Za-z_]labs[^A-Za-z_]/p
/[^:A-Za-z_]remove[^A-Za-z_]/p
/[^:A-Za-z_]swprintf[^A-Za-z_]/p
/[^:A-Za-z_]wcsrtombs[^A-Za-z_]/p
/[^:A-Za-z_]clearerr[^A-Za-z_]/p
/[^:A-Za-z_]fwide[^A-Za-z_]/p
/[^:A-Za-z_]ldexp[^A-Za-z_]/p
/[^:A-Za-z_]rename[^A-Za-z_]/p
/[^:A-Za-z_]swscanf[^A-Za-z_]/p
/[^:A-Za-z_]wcsspn[^A-Za-z_]/p
/[^:A-Za-z_]clock[^A-Za-z_]/p
/[^:A-Za-z_]fwprintf[^A-Za-z_]/p
/[^:A-Za-z_]ldiv[^A-Za-z_]/p
/[^:A-Za-z_]rewind[^A-Za-z_]/p
/[^:A-Za-z_]system[^A-Za-z_]/p
/[^:A-Za-z_]wcsstr[^A-Za-z_]/p
/[^:A-Za-z_]cos[^A-Za-z_]/p
/[^:A-Za-z_]fwrite[^A-Za-z_]/p
/[^:A-Za-z_]localeconv[^A-Za-z_]/p
/[^:A-Za-z_]scanf[^A-Za-z_]/p
/[^:A-Za-z_]tan[^A-Za-z_]/p
/[^:A-Za-z_]wcstod[^A-Za-z_]/p
/[^:A-Za-z_]cosh[^A-Za-z_]/p
/[^:A-Za-z_]fwscanf[^A-Za-z_]/p
/[^:A-Za-z_]localtime[^A-Za-z_]/p
/[^:A-Za-z_]setbuf[^A-Za-z_]/p
/[^:A-Za-z_]tanh[^A-Za-z_]/p
/[^:A-Za-z_]wcstok[^A-Za-z_]/p
/[^:A-Za-z_]ctime[^A-Za-z_]/p
/[^:A-Za-z_]getc[^A-Za-z_]/p
/[^:A-Za-z_]log[^A-Za-z_]/p
/[^:A-Za-z_]setlocale[^A-Za-z_]/p
/[^:A-Za-z_]time[^A-Za-z_]/p
/[^:A-Za-z_]wcstol[^A-Za-z_]/p
/[^:A-Za-z_]difftime[^A-Za-z_]/p
/[^:A-Za-z_]getchar[^A-Za-z_]/p
/[^:A-Za-z_]log10[^A-Za-z_]/p
/[^:A-Za-z_]setvbuf[^A-Za-z_]/p
/[^:A-Za-z_]tmpfile[^A-Za-z_]/p
/[^:A-Za-z_]wcstombs[^A-Za-z_]/p
/[^:A-Za-z_]div[^A-Za-z_]/p
/[^:A-Za-z_]getenv[^A-Za-z_]/p
/[^:A-Za-z_]longjmp[^A-Za-z_]/p
/[^:A-Za-z_]signal[^A-Za-z_]/p
/[^:A-Za-z_]tmpnam[^A-Za-z_]/p
/[^:A-Za-z_]wcstoul[^A-Za-z_]/p
/[^:A-Za-z_]exit[^A-Za-z_]/p
/[^:A-Za-z_]gets[^A-Za-z_]/p
/[^:A-Za-z_]malloc[^A-Za-z_]/p
/[^:A-Za-z_]sin[^A-Za-z_]/p
/[^:A-Za-z_]tolower[^A-Za-z_]/p
/[^:A-Za-z_]wcsxfrm[^A-Za-z_]/p
/[^:A-Za-z_]exp[^A-Za-z_]/p
/[^:A-Za-z_]getwc[^A-Za-z_]/p
/[^:A-Za-z_]mblen[^A-Za-z_]/p
/[^:A-Za-z_]sinh[^A-Za-z_]/p
/[^:A-Za-z_]toupper[^A-Za-z_]/p
/[^:A-Za-z_]wctob[^A-Za-z_]/p
/[^:A-Za-z_]fabs[^A-Za-z_]/p
/[^:A-Za-z_]getwchar[^A-Za-z_]/p
/[^:A-Za-z_]mbrlen[^A-Za-z_]/p
/[^:A-Za-z_]sprintf[^A-Za-z_]/p
/[^:A-Za-z_]towctrans[^A-Za-z_]/p
/[^:A-Za-z_]wctomb[^A-Za-z_]/p
/[^:A-Za-z_]fclose[^A-Za-z_]/p
/[^:A-Za-z_]gmtime[^A-Za-z_]/p
/[^:A-Za-z_]mbrtowc[^A-Za-z_]/p
/[^:A-Za-z_]sqrt[^A-Za-z_]/p
/[^:A-Za-z_]towlower[^A-Za-z_]/p
/[^:A-Za-z_]wctrans[^A-Za-z_]/p
/[^:A-Za-z_]feof[^A-Za-z_]/p
/[^:A-Za-z_]isalnum[^A-Za-z_]/p
/[^:A-Za-z_]mbsinit[^A-Za-z_]/p
/[^:A-Za-z_]srand[^A-Za-z_]/p
/[^:A-Za-z_]towupper[^A-Za-z_]/p
/[^:A-Za-z_]wctype[^A-Za-z_]/p
/[^:A-Za-z_]ferror[^A-Za-z_]/p
/[^:A-Za-z_]isalpha[^A-Za-z_]/p
/[^:A-Za-z_]mbsrtowcs[^A-Za-z_]/p
/[^:A-Za-z_]sscanf[^A-Za-z_]/p
/[^:A-Za-z_]ungetc[^A-Za-z_]/p
/[^:A-Za-z_]wmemchr[^A-Za-z_]/p
/[^:A-Za-z_]fflush[^A-Za-z_]/p
/[^:A-Za-z_]iscntrl[^A-Za-z_]/p
/[^:A-Za-z_]mbstowcs[^A-Za-z_]/p
/[^:A-Za-z_]strcat[^A-Za-z_]/p
/[^:A-Za-z_]ungetwc[^A-Za-z_]/p
/[^:A-Za-z_]wmemcmp[^A-Za-z_]/p
/[^:A-Za-z_]fgetc[^A-Za-z_]/p
/[^:A-Za-z_]isdigit[^A-Za-z_]/p
/[^:A-Za-z_]mbtowc[^A-Za-z_]/p
/[^:A-Za-z_]strchr[^A-Za-z_]/p
/[^:A-Za-z_]vfprintf[^A-Za-z_]/p
/[^:A-Za-z_]wmemcpy[^A-Za-z_]/p
/[^:A-Za-z_]fgetpos[^A-Za-z_]/p
/[^:A-Za-z_]isgraph[^A-Za-z_]/p
/[^:A-Za-z_]memchr[^A-Za-z_]/p
/[^:A-Za-z_]strcmp[^A-Za-z_]/p
/[^:A-Za-z_]vfwprintf[^A-Za-z_]/p
/[^:A-Za-z_]wmemmove[^A-Za-z_]/p
/[^:A-Za-z_]fgets[^A-Za-z_]/p
/[^:A-Za-z_]islower[^A-Za-z_]/p
/[^:A-Za-z_]memcmp[^A-Za-z_]/p
/[^:A-Za-z_]strcoll[^A-Za-z_]/p
/[^:A-Za-z_]vprintf[^A-Za-z_]/p
/[^:A-Za-z_]wmemset[^A-Za-z_]/p
/[^:A-Za-z_]fgetwc[^A-Za-z_]/p
/[^:A-Za-z_]isprint[^A-Za-z_]/p
/[^:A-Za-z_]memcpy[^A-Za-z_]/p
/[^:A-Za-z_]strcpy[^A-Za-z_]/p
/[^:A-Za-z_]vsprintf[^A-Za-z_]/p
/[^:A-Za-z_]wprintf[^A-Za-z_]/p
/[^:A-Za-z_]fgetws[^A-Za-z_]/p
/[^:A-Za-z_]ispunct[^A-Za-z_]/p
/[^:A-Za-z_]memmove[^A-Za-z_]/p
/[^:A-Za-z_]strcspn[^A-Za-z_]/p
/[^:A-Za-z_]vswprintf[^A-Za-z_]/p
/[^:A-Za-z_]wscanf[^A-Za-z_]/p
/[^:A-Za-z_]floor[^A-Za-z_]/p
/[^:A-Za-z_]isspace[^A-Za-z_]/p
/[^:A-Za-z_]memset[^A-Za-z_]/p
/[^:A-Za-z_]strerror[^A-Za-z_]/p
/[^:A-Za-z_]vwprintf[^A-Za-z_]/p
