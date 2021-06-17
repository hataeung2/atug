#ifdef _WINDOWS
#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
/**
* @brief file line tracking.
* two lines into each file's top line
*/
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)

/**
* @brief memory leak tracking
* at the top line of the "main" function,
* 
* add two lines
* _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
* _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
* //_CrtSetBreakAlloc(10438); // find allocated position 
*/

/**
* @brief console output window 
* #pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
*/

#endif//!_DEBUG
#endif//!_WINDOWS