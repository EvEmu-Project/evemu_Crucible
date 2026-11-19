/*
* https://github.com/ZhangJunQCC/CallStack-for-Windows-and-Linux
BSD 3-Clause License

Copyright (c) 2017, Jun Zhang
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// 
// Author: Jun Zhang
// mailto: zhangjunqcc@gmail.com
//

#include "CallStack.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#if defined(_WIN32)
#include <windows.h>
#include <imagehlp.h>
#if defined(__MINGW32__)
#include <cxxabi.h>
#endif
#else
#include <execinfo.h>
#include <cxxabi.h>
#endif

using namespace debug;
using namespace std;

const char* CallStack::Unknown_Function = "[unknown function]";
const char* CallStack::Unknown_Module = "[unknown module]";

CallStack::CallStack(void) 
{
    /* Nothing to do */
}

CallStack::~CallStack(void) 
{
    /* Nothing to do */ 
}

string CallStack::Demangle(const char* name)
{
#if defined(_MSC_VER)
    return string(name);
#else    
    string dname;
    int status = 0;
    char* pdname = abi::__cxa_demangle(name, NULL, 0, &status);     
    if(status == 0)
    {
        dname.assign(pdname);
        free(pdname);
    }
    else
    {
        dname.assign(CallStack::Unknown_Function);
    }
    return dname;
#endif
}

#if defined(_WIN32)
class noncopyable // Like the one in boost.
{
protected:
    noncopyable(void) { /* Nothing to do */ }
private:
    noncopyable(const noncopyable&);
    noncopyable& operator = (const noncopyable&);
};
// A global thread mutex class.
class mutex : noncopyable
{
public:
    mutex(void) { InitializeCriticalSection(&cs); }
    ~mutex(void) { DeleteCriticalSection(&cs); }
    void lock(void) { EnterCriticalSection(&cs); }
    void unlock(void) { LeaveCriticalSection(&cs); }
private:
    CRITICAL_SECTION cs;
} cs;

#endif // _WIN32

void CallStack::GetCalls(vector<CallStack::CallInfo>& calls)
{
#if defined(_WIN32)

    // Prepare context.
    cs.lock();
    if(!SymInitialize(GetCurrentProcess(), 0, TRUE))
    {
        fprintf(stderr, "Error: Failed to initialize symbol context.\n");
        cs.unlock();
        return;
    }
    // Prepare variables.
#if defined(_M_AMD64)
    const DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
    const DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif    
    const HANDLE process = GetCurrentProcess();
    const HANDLE thread = GetCurrentThread();
    STACKFRAME frame = { 0 };
    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);
#if defined(_M_AMD64)
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
#else
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
#endif
    int limit = 0;    
    const bool has_limit = (limit != 0);
    char symbol_buffer[sizeof(IMAGEHLP_SYMBOL)+255];
    char module_name_raw[MAX_PATH];
    
    // Go over stack.
    calls.clear();
    while(StackWalk(machine, process, thread, &frame, &context, 0, SymFunctionTableAccess, SymGetModuleBase, 0))
    {       
        if(has_limit && limit == 0) break; --limit;
        
        IMAGEHLP_SYMBOL* symbol = reinterpret_cast<IMAGEHLP_SYMBOL*>(symbol_buffer);
        symbol->SizeOfStruct = (sizeof(*symbol))+255;
        symbol->MaxNameLength = 254;

#if defined(_WIN64)
        const DWORD64 module_base = SymGetModuleBase(process, frame.AddrPC.Offset);
#else
        const DWORD module_base = SymGetModuleBase(process, frame.AddrPC.Offset);
#endif
        string module_name = Unknown_Module;
        if(module_base && GetModuleFileNameA(reinterpret_cast<HINSTANCE>(module_base), module_name_raw, MAX_PATH)) module_name = module_name_raw;

#if defined(_WIN64)
        DWORD64 dummy = 0;
#else
        DWORD dummy = 0;
#endif
        const bool got_symbol = SymGetSymFromAddr(process, frame.AddrPC.Offset, &dummy, symbol);
        const string function = (got_symbol) ? symbol->Name : Unknown_Function;

        calls.push_back(CallInfo(frame.AddrPC.Offset, function, module_name));
    }
    
    SymCleanup(GetCurrentProcess());
    cs.unlock();

#else // For Linux and Unix.

    const int BufferSize = 256;
    void* buffer[BufferSize];
    const int ncalls = backtrace(buffer, sizeof(buffer));
    char** callstrings = backtrace_symbols(buffer, ncalls);
    
    if(callstrings == NULL)
    {
        fprintf(stderr, "Error: Failed to obtain the symbol table from the executable.\n");
        return;
    }

    calls.clear();
    for(int i = 0; i < ncalls; ++i)
    {
        const string funinfo(callstrings[i]);
        const int posleftparenthesis = funinfo.rfind('(');
        const int posplus = funinfo.rfind('+');
        const int posleftbracket = funinfo.rfind('[');
        const int posrightbracket = funinfo.rfind(']');

        long long unsigned int offset = strtoull(funinfo.substr(posleftbracket+1, posrightbracket-posleftbracket-1).c_str(), NULL, 0);
        const string module_name = funinfo.substr(0, posleftparenthesis);
        const string function = Demangle(funinfo.substr(posleftparenthesis+1, posplus-posleftparenthesis-1).c_str());

        calls.push_back(CallInfo(offset, function, module_name));
     }
     free(callstrings);

#endif // _WIN32
}
