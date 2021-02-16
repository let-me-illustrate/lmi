// C++ exception unwinder for pc-linux-gnu.
//
// Copyright (C) 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "unwind.hpp"

#if defined LMI_X86_64 && defined LMI_POSIX

#define UNW_LOCAL_ONLY

#include <cstddef>                      // size_t
#include <cstdio>                       // fprintf()
#include <cstdlib>                      // free()
#include <cxxabi.h>
#include <dlfcn.h>
#include <elfutils/libdwfl.h>
#include <exception>
#include <execinfo.h>
#include <libunwind.h>
#include <typeinfo>                     // type_info

#if defined __GNUC__
#   pragma GCC diagnostic push
    // Calls to low-level C functions may as well use "0" for
    // terseness instead of "nullptr".
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    // Casting dlsym objects to function pointers is allowed
    // only as a conditional extension.
#   pragma GCC diagnostic ignored "-Wconditionally-supported"
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#endif // defined __GNUC__

// ABI:
extern "C" void __cxa_throw
    (void*                  thrown_exception // exception address
    ,struct std::type_info* tinfo            // exception type
    ,void                 (*dest)(void*)     // exception destructor
    );

using cxa_throw_t   = void (*)(void*, std::type_info*, void (*)(void*));
using cxa_rethrow_t = void (*)();

cxa_throw_t   original_cxa_throw   = (cxa_throw_t)   dlsym(RTLD_NEXT, "__cxa_throw");
// Not yet used:
cxa_rethrow_t original_cxa_rethrow = (cxa_rethrow_t) dlsym(RTLD_NEXT, "__cxa_rethrow");

// ABI:
extern "C" char* __cxa_demangle
    (char const * mangled_name  // mangled name, NUL-terminated
    ,char       * output_buffer // just use 0
    ,std::size_t* length        // just use 0
    ,int        * status        // zero --> success
    );

/// Print type of exception, and what() if it's a std::exception.
///
/// Use fprintf() rather than iostreams because the former can't
/// throw C++ exceptions.

void identify_exception(void* thrown_exception, std::type_info* tinfo)
{
    if(0 == tinfo)
        {
        std::fprintf(stderr, "Exception type_info is null.\n");
        return;
        }

    char const* mangled_name = tinfo->name();
    int status = 0;

    char* demangled_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
    char const* exception_name = (0 == status) ? demangled_name : mangled_name;
    std::fprintf(stderr, "Exception type: '%s'\n", exception_name);
    std::free(demangled_name);

    using abi_ti = abi::__class_type_info;
    abi_ti const* src = dynamic_cast<abi_ti const*>(&typeid(std::exception));
    abi_ti const* dst = dynamic_cast<abi_ti*>(tinfo);

    if(dst && src)
        {
        std::exception* x = reinterpret_cast<std::exception*>
            (abi::__dynamic_cast
                (thrown_exception
                ,src // static type of exception object
                ,dst // desired type of target
                ,-1  // unspecified relationship between src and dst
                )
            );
        if(x)
            {
            std::fprintf(stderr, "what(): '%s'\n", x->what());
            }
        }
}

void fail(char const* msg)
{
    std::fprintf(stderr, "%s\n", msg);
}

void print_backtrace()
{
    // Initialize libdw.
    char* debuginfo_path     = 0;
    Dwfl_Callbacks callbacks = {};
    callbacks.find_elf       = dwfl_linux_proc_find_elf;
    callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
    callbacks.debuginfo_path = &debuginfo_path;
    Dwfl* dwfl = dwfl_begin(&callbacks);
    if
        (dwfl == 0
        || 0 != dwfl_linux_proc_report(dwfl, getpid())
        || 0 != dwfl_report_end(dwfl, 0, 0)
        )
        {fail("Error initializing DWARF."); return;}

    // Initialize libunwind.
    unw_context_t context;
    if(0 != unw_getcontext(&context))
        {fail("Failed to get machine state"); return;}

    unw_cursor_t cursor;
    if(0 != unw_init_local(&cursor, &context))
        {fail("Failed to initialize cursor"); return;}

    // IP is in this function, so first frame could be skipped.
    int frame_number = 0;
    while(0 < unw_step(&cursor))
        {
        unw_word_t ip;
        if(0 != unw_get_reg(&cursor, UNW_REG_IP, &ip))
            {fail("Failed to read IP register"); return;}

        char symbol[4096]; // Should be plenty.
        unw_word_t offset;
        if(0 == unw_get_proc_name(&cursor, symbol, sizeof(symbol), &offset))
            {
            int status = 0;
            char* demangled_name = abi::__cxa_demangle(symbol, 0, 0, &status);
            char* function_name = (0 == status) ? demangled_name : symbol;

            char const* source_file = "?""?""?";
            int line_number = 0;

            Dwarf_Addr addr = ip - 4;
            Dwfl_Line* obj_line = dwfl_getsrc(dwfl, addr);
            if (obj_line != 0)
                {
                source_file = dwfl_lineinfo(obj_line, &addr, &line_number, 0, 0, 0);
                }

            std::fprintf(stderr, "#%2d"      , frame_number);
            std::fprintf(stderr, " 0x%lx:"   , ip);
            std::fprintf(stderr, " %s:%d"    , source_file, line_number);
// 'offset' doesn't seem very useful
//          std::fprintf(stderr, " %s +0x%lx", function_name, offset);
            std::fprintf(stderr, " %s"       , function_name);
            std::fprintf(stderr, "\n");

            std::free(demangled_name);
            }
        else
            {
            std::fprintf(stderr, "Failed to get symbol name.\n");
            }
        ++frame_number;
        }
}

extern "C"
void __cxa_throw(void* thrown_exception, std::type_info* tinfo, void (*dest)(void*))
{
    identify_exception(thrown_exception, tinfo);
    print_backtrace();
    original_cxa_throw(thrown_exception, tinfo, dest);
}

#endif // defined LMI_X86_64 && defined LMI_POSIX
