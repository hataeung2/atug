#ifndef _ATUG2_MEMORY_H_
#define _ATUG2_MEMORY_H_
#include "atug2.h"
#include "amem_macro.h"

namespace atug2 {
  class ATUG2_API IHeapTracker {
  public:
    IHeapTracker();
    virtual ~IHeapTracker();

    static void* operator new(size_t _size);
    static void operator delete(void* _ptr);
    static void* operator new[](size_t _size);
    static void operator delete[](void* _ptr);

    const bool IsOnHeap() const;

    static void* operator new(size_t _size, const char* _file, int _line);
    static void* operator new[](size_t _size, const char* _file, int _line);
  };

  class ATUG2_API AManagedHeap {
  public:
    static void* Allocate(const size_t _size);
    static void* Allocate(const size_t _size, const char* _file, const int _line);
    static void Free(void* _ptr);
    static const bool IsOnHeap(const void* _ptr);
    static const bool IsEmpty();
    static const size_t Count();
  };

  struct RefCntProp;
  class ATUG2_API IRefCounter {
    DONT_ALLOW_ASSIGN(IRefCounter);
  public:
    IRefCounter();
    virtual ~IRefCounter();
    IRefCounter(const IRefCounter& _copy);
  public:
    IRefCounter& operator++();
    IRefCounter& operator--();
    const Bool IsReferenced() const;
    const Number RefCnt() const;
  protected:
    RefCntProp* refp_;
  };

}//!namespace atug2 {
#endif//!_ATUG2_MEMORY_H_