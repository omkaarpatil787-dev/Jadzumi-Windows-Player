#include "CrashDiagnostics.h"
#include <windows.h>
#include <dbghelp.h>
#include <string>
#include <cstdio>

static std::wstring diagnosticDirectory;
static LONG WINAPI writeCrash(EXCEPTION_POINTERS *exception) {
    const auto process=GetCurrentProcess();
    const auto dump=CreateFileW((diagnosticDirectory+L"/startup.dmp").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(dump!=INVALID_HANDLE_VALUE){
        MINIDUMP_EXCEPTION_INFORMATION info{GetCurrentThreadId(),exception,FALSE};
        MiniDumpWriteDump(process,GetCurrentProcessId(),dump,MiniDumpNormal,&info,nullptr,nullptr);CloseHandle(dump);
    }
    const auto trace=CreateFileW((diagnosticDirectory+L"/startup-stack.txt").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(trace!=INVALID_HANDLE_VALUE){
        auto write=[trace](const char *line){DWORD count;WriteFile(trace,line,static_cast<DWORD>(std::char_traits<char>::length(line)),&count,nullptr);};
        char line[4096];std::snprintf(line,sizeof(line),"Exception %08lx at %p\n",exception->ExceptionRecord->ExceptionCode,exception->ExceptionRecord->ExceptionAddress);write(line);
        SymSetOptions(SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS);
        SymInitialize(process,nullptr,TRUE);
        CONTEXT context=*exception->ContextRecord;
        STACKFRAME64 frame{};frame.AddrPC.Offset=context.Rip;frame.AddrStack.Offset=context.Rsp;frame.AddrFrame.Offset=context.Rbp;
        frame.AddrPC.Mode=frame.AddrStack.Mode=frame.AddrFrame.Mode=AddrModeFlat;
        for(int i=0;i<50;++i){
            const DWORD64 address=frame.AddrPC.Offset;
            const auto base=SymGetModuleBase64(process,address);
            char module[MAX_PATH]{};if(base)GetModuleFileNameA(reinterpret_cast<HMODULE>(base),module,MAX_PATH);
            alignas(SYMBOL_INFO) char storage[sizeof(SYMBOL_INFO)+MAX_SYM_NAME]{};
            auto *symbol=reinterpret_cast<SYMBOL_INFO*>(storage);symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=MAX_SYM_NAME;
            DWORD64 displacement=0;const bool found=SymFromAddr(process,address,&displacement,symbol)!=FALSE;
            std::snprintf(line,sizeof(line),"%02d %s +0x%llx %s +0x%llx\n",i,module,static_cast<unsigned long long>(address-base),found?symbol->Name:"?",static_cast<unsigned long long>(displacement));write(line);
            if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;
        }
        SymCleanup(process);CloseHandle(trace);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
void installCrashDiagnosticsFromEnvironment(){
    wchar_t directory[32768]{};
    const auto length=GetEnvironmentVariableW(L"JADZUMI_CRASH_DIR",directory,32768);
    if(!length || length>=32768)return;
    diagnosticDirectory=directory;
    SetUnhandledExceptionFilter(writeCrash);
}
