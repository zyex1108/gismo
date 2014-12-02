
#pragma once

#include <gsCore/gsConfig.h>
#include <gsCore/gsExport.h>

#include <signal.h>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #undef NOMINMAX
#endif


namespace gismo {

namespace internal {

#if defined(__GNUC__)

    /// Prints the call stack
    void gsStackWalker(void * context);

    /// Exception hook 
    void gsExceptionHook(int sig, siginfo_t * siginfo, void * context);

    /// Prints demangled GCC function names from mangled symbols
    void printGccDemangled(char * symbol);

#endif


#if defined(_WIN32)

    /// Prints the call stack
    GISMO_EXPORT void gsStackWalker(CONTEXT * context);

    /// Exception hook 
    GISMO_EXPORT LONG WINAPI gsExceptionHook(EXCEPTION_POINTERS * ExceptionInfo);

    /// Used to take control of all exceptions
    GISMO_EXPORT BOOL PreventSetUnhandledExceptionFilter();

#endif


/// Exception handler
GISMO_EXPORT bool gsExceptionHandler();


/// Initialize exception handler for stack backtrace
static const bool gismoExceptionHandler = gsExceptionHandler();


} // end namespace internal

} // end namespace gismo

//////////////////////////////////////////////////
//////////////////////////////////////////////////


#ifndef GISMO_HEADERS_ONLY
#include GISMO_HPP_HEADER(gsStackWalker.hpp)
#endif