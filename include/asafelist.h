#ifndef _ATUG2_SAFELIST_H_
#define _ATUG2_SAFELIST_H_
#include "atug2.h"
#include "amem_macro.h"
#include "adata.h"
#include "ashared_ptr.h"

namespace atug2 {
  template <typename T> struct SafeListProp;

  template <typename T = adata::ADataBucket>
  class ATUG2_API ASafeList {
    DONT_ALLOW_COPY_AND_ASSIGN(ASafeList<T>);
  public:
    ASafeList();
    ~ASafeList();
    Void Clear();

  public:
    const Bool IsEmpty();
    const SizeT Size();

  public:
    const SizeT PushBack(T _data);
    const SizeT PushFront(T _data);
    const T PopBack();
    const T PopFront();

  private:
    SafeListProp<T>* slistp_;
  };

}//! namespace atug2

#endif//!_ATUG2_SAFELIST_H_