#ifndef _ATUG2_H_
#define _ATUG2_H_

/*
 * @brief: atug2 (a totally understandable good - 2nd work)
 * atug2 is a C++ library to make the development easy 
 * by managing memory and 
 * by providing common features with encapsulating complicated logics
 * so that developer can ensure the code to simple & stable
 */

//@tracing
#define __FILENAME std::string(__FILE__).substr(std::string(__FILE__).find_last_of(DIV)+1).c_str()
#ifdef _DEBUG
#include <iostream>
#define BUILD_MSG(content) message("notice: (" __FUNCTION__ ") at " __FILE__ " : " content)
  
#define _TRACE(log) std::wcout << L"> " << log << L" @ " << __FILENAME << L":" << __LINE__ << std::endl;
#ifdef _WINDOWS
  #define _DBGOUT OutputDebugStringA
#endif
#else
#define BUILD_MSG(content) message()
#define _TRACE(log)
#endif

//@std::mutex lock helper
#define LOCK(target) (target).lock()
#define UNLOCK(target) (target).unlock()
#define SCOPELOCK(target) std::lock_guard<std::mutex> lock(target)

//@os specified
#if defined(_WINDOWS)
  #ifdef ATUG2_EXPORTS
  #define ATUG2_API __declspec(dllexport)
  #define ATUG2_API_EXTERN
  #else
  #define ATUG2_API __declspec(dllimport)
  #define ATUG2_API_EXTERN extern
  #endif

  #ifdef ATUG2_NO_USE_WINSOCK
    #include <windows.h>
  #else
    #include <ws2tcpip.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
  #endif

  //@tracing
  #ifdef _DEBUG
  #include <assert.h>
  #ifdef _MSVC
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
  //@File line tracing: 2 lines below to add on the top of the file 
  //#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
  //#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)

  //@Memory leakage tracing: the lines below to add on the Program main function's first line
  //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  //_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  ////_CrtSetBreakAlloc(10438);

  //@Console output window creation
  //#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

  //@Remove abnormal termination message box for RE-START automatically for watchdog's child
  //SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
  #endif // _MSVC
  #endif // _DEBUG


  #include <tchar.h>
  #include <direct.h>
  #include <io.h>
  #define DIV L'\\'
  #define DIVA '\\'
  #define access    _access_s
  typedef HANDLE Handle;

#elif defined(_LINUX)
  #include <unistd.h>
  #define DIV L'/'
  #define DIVA '/'
  typedef int Handle, FileDescriptor, Fd; // file descriptor
#endif

//@for C code using this library
#ifdef __cplusplus
extern "C" {
#endif
  
#ifdef __cplusplus
}
#endif

//@defines
#if defined(_WINDOWS)
#else
#define PURE =0
#endif
#define CONST const
#define EQ ==
#define NE !=
#define NOT !
#define REVERSE ~
#define AND &&
#define OR ||

#define TRUE 1
#define FALSE 0
#define MINUS -1

#define MAXPATH 260
#define ZERO 0
#define EMPTY ZERO
#define ONLY 1
#define END L'\0'
#define ENDA '\0'
#define EMPTY_STR L""

using std::string;
using std::wstring;

namespace atug2 {
  // typedef
  typedef void Void;
  typedef bool Bool;
  typedef short Short;
  typedef unsigned short Ushort;
  typedef int Int, Number, Error, ResultCode;
  typedef unsigned int Uint;
  typedef __int32 Int32, FlagSet;
  typedef unsigned __int32 Uint32;
  typedef __int64 Int64;
  typedef unsigned __int64 Uint64;
  typedef size_t SizeT;
  // 32bit, 64bit differ
  typedef long Long;
  typedef unsigned long Ulong, Dword;
  typedef long double LongDouble;
  //!32bit, 64bit differ
  typedef long long Longlong;

  typedef char Char;
  typedef const char* Str;
  typedef unsigned char Byte;
  typedef unsigned char* BytePtr;
  typedef wchar_t Wchar;
  typedef char16_t Char16;
  typedef char32_t Char32;
  typedef const wchar_t* Wstr;
  typedef const char16_t* Str16;
  typedef const char32_t* Str32;

  // string
  typedef std::wstring astring;

  typedef float Float;
  typedef double Double;
  typedef long double LongDouble;

}//!namespace atug2

#endif//!_ATUG2_H_
