
// memory macros
// 

#ifndef _ATUG2_MEM_MACRO_H_
#define _ATUG2_MEM_MACRO_H_
#include <new>
#include "atug2.h"
using namespace atug2;

/**
 * @brief for heap tracking 
 * 
 */

#define anew(type, amount) static_cast<type*>(atug2::AManagedHeap::Allocate(sizeof(type)*amount, __FILENAME, __LINE__))
#define adelete(target) atug2::AManagedHeap::Free(target);

/**
 * @brief prevent copy, assign 
 * 
 */
#define DONT_ALLOW_COPY(typename) \
  typename(const typename& _copy) = delete; // classname x = y; OR classname x{y} OR ...;
#define DONT_ALLOW_ASSIGN(typename) \
  typename& operator=(const typename& _assign) = delete; // classname x; x = y;
#define DONT_ALLOW_COPY_AND_ASSIGN(typename) \
  typename(const typename& _copy) = delete; \
  typename& operator=(const typename& _assign) = delete;
#define DONT_ALLOW_MOVECOPY(typename) \
  typename(typename&& _mcopy) = delete;
#define DONT_ALLOVE_MOVEASSIGN(typename) \
  typename& operator=(typename&& _massign) = delete;
#define DONT_ALLOW_MOVECOPYASSIGN(typename) \
  typename(typename&& _mcopy) = delete; \
  typename& operator=(typename&& _massign) = delete;

/**
 * @brief prevent using new
 * new & delete, nothrow new 
 */
#define DONT_ALLOW_NEW_DELETE  \
void* operator new(size_t _size) = delete; \
void* operator new[](size_t _size) = delete; \
void* operator new(size_t _len, const std::nothrow_t&) noexcept = delete; \
void* operator new[](size_t _len, const std::nothrow_t&) noexcept = delete; \
void operator delete(void* _ptr) = delete; \
void operator delete[](void* _ptr) = delete; \
void operator delete(void* _ptr, const std::nothrow_t&) noexcept = delete; \
void operator delete[](void* _ptr, const std::nothrow_t&) noexcept = delete; \
void operator delete(void* _ptr, void*) noexcept = delete; \
void operator delete[](void* _ptr, void*) noexcept = delete;

/**
 * @brief prevent using pointer
 * 
 */
#define DONT_ALLOW_ACCESS_AS_PTR \
  const void operator&() = delete; \
  const void operator*() = delete; \
  const void operator->() = delete

/**
 * @brief for C like C++ usage
 * We don't recommend the default new & delete or malloc(calloc) & free
 * those macros are prepared just for special cases
 */
#ifdef _DEBUG
#ifdef _MSVC
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
#define SAFE_FREE(ptr) if(nullptr != ptr) { free(ptr); ptr = nullptr; }
#define ALLOC(target, type, len) target = (type*)malloc(sizeof(type)*(len)); memset(target, 0, sizeof(type)*(len))
#define CREATE_VAR(type, name) type name; memset(&name, 0, sizeof(type))
#define CREATE_ARR(type, name, len) type name[len] = { 0, }
#define ALLOC_VAR(type, name) type* name = (type*)malloc(sizeof(type)); memset(name, 0, sizeof(type))
#define ALLOC_ARR(type, name, len) type* name = (type*)malloc(sizeof(type)*(len)); memset(name, 0, sizeof(type)*(len))
#define SET_VAL(ptr, val, type, len) memset(ptr, val, sizeof(type)*len)
#define CLEAR_VAL(ptr, type, len) SET_VAL(ptr, 0, type, len)

#define SAFE_DELETE(ptr) delete ptr; ptr = nullptr
#define SAFE_DELETE_ARR(ptr) delete[] ptr; ptr = nullptr

#endif//!_ATUG2_MEM_MACRO_H_
