#ifndef JACY_PLATFORM_SIGNALS_H
#define JACY_PLATFORM_SIGNALS_H

#include <iostream>
#include <stdio.h>

#define MAX_STACK_FRAMES 64

static const char * programName;

int addr2line(const char * const programName, const void * const addr) {
    char addr2lineCmd[512] = {0};

    #ifdef __APPLE__
    sprintf(addr2lineCmd, "atos -o %.256s %p", programName, addr);
    #else
    sprintf(addr2lineCmd,"addr2line -f -p -e %.256s %p", programName, addr);
    #endif

    return system(addr2lineCmd);
}

#if defined(_WIN32) || \
    defined(_WIN64) || \
    defined(__WIN32) || \
    defined(__WIN32__) || \
    defined(__WIN64) || \
    defined(__WIN64__)
#define WIN 1
#else
#define WIN 0
#endif // WIN checks

#if defined(_WIN64) || defined(__WIN64) || defined(__WIN64__)
#define X64
#else
#define X86
#endif

//#if defined(X64)
//#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_AMD64
//#elif defined(X86)
//#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_I386
//#endif


#if WIN
#include <windows.h>
#include <imagehlp.h>

void winPrintStacktrace(CONTEXT * ctx) {
    SymInitialize(GetCurrentProcess(), 0, true);

    // Note: Maybe check for x64 and use `STACKFRAME64`
    STACKFRAME frame = {0};

    #ifdef _AMD64_
    frame.AddrPC.Offset = ctx->Rip;
    frame.AddrStack.Offset = ctx->Rsp;
    frame.AddrFrame.Offset = ctx->Rbp;
    #else
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrFrame.Offset = ctx->Ebp;
    #endif

    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;

    while (
        StackWalk(
            IMAGE_FILE_MACHINE_I386,
            GetCurrentProcess(),
            GetCurrentThread(),
            &frame,
            ctx,
            0,
            SymFunctionTableAccess,
            SymGetModuleBase,
            0
        )
    ) {
        addr2line(programName, (void*)frame.AddrPC.Offset);
    }

    SymCleanup(GetCurrentProcess());
}

LONG WINAPI winExceptionHandler(EXCEPTION_POINTERS * info) {
    std::string msg;
    switch (info->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION: {
            msg = "Error: EXCEPTION_ACCESS_VIOLATION";
            break;
        }
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            msg = "Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
            break;
        case EXCEPTION_BREAKPOINT:
            msg = "Error: EXCEPTION_BREAKPOINT";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            msg = "Error: EXCEPTION_DATATYPE_MISALIGNMENT";
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            msg = "Error: EXCEPTION_FLT_DENORMAL_OPERAND";
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            msg = "Error: EXCEPTION_FLT_DIVIDE_BY_ZERO";
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            msg = "Error: EXCEPTION_FLT_INEXACT_RESULT";
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            msg = "Error: EXCEPTION_FLT_INVALID_OPERATION";
            break;
        case EXCEPTION_FLT_OVERFLOW:
            msg = "Error: EXCEPTION_FLT_OVERFLOW";
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            msg = "Error: EXCEPTION_FLT_STACK_CHECK";
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            msg = "Error: EXCEPTION_FLT_UNDERFLOW";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            msg = "Error: EXCEPTION_ILLEGAL_INSTRUCTION";
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            msg = "Error: EXCEPTION_IN_PAGE_ERROR";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            msg = "Error: EXCEPTION_INT_DIVIDE_BY_ZERO";
            break;
        case EXCEPTION_INT_OVERFLOW:
            msg = "Error: EXCEPTION_INT_OVERFLOW";
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            msg = "Error: EXCEPTION_INVALID_DISPOSITION";
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            msg = "Error: EXCEPTION_NONCONTINUABLE_EXCEPTION";
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            msg = "Error: EXCEPTION_PRIV_INSTRUCTION";
            break;
        case EXCEPTION_SINGLE_STEP:
            msg = "Error: EXCEPTION_SINGLE_STEP";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            msg = "Error: EXCEPTION_STACK_OVERFLOW";
            break;
        default:
            msg = "Error: Unrecognized Exception";
            break;
    }

    std::cerr << msg << std::endl;

    if (EXCEPTION_STACK_OVERFLOW != info->ExceptionRecord->ExceptionCode) {
        winPrintStacktrace(info->ContextRecord);
    } else {
        #ifdef _AMD64_
        addr2line(programName, (void*)info->ContextRecord->Rip);
        #else
        addr2line(programName, (void*)info->ContextRecord->Eip);
        #endif
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif // WIN backtrace

void setSignals(int argc, const char ** argv) {
    programName = argv[0];

    #if WIN
    SetUnhandledExceptionFilter(winExceptionHandler);
    #endif
}

// TODO: msvc

#endif // JACY_PLATFORM_SIGNALS_H
