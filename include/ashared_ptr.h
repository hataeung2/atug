#ifndef _ATUG2_SHARED_PTR_H_
#define _ATUG2_SHARED_PTR_H_
#include "atug2.h"
#include "amem_macro.h"

namespace atug2 {
  struct SPtrErr {
    SPtrErr(const Char* _msg) : err_msg(_msg) {}
    const std::string& what() { return err_msg; }
    std::string err_msg;
  };

  class AFile;
  class ALog;
  class AThread;
  class ARoutine;
  class ASocket;
  class AObj;

  struct SPtrProp;
  template <typename T>
  class ATUG2_API ASharedPtr {
    DONT_ALLOW_NEW_DELETE;
  public:
    explicit ASharedPtr();
    ASharedPtr(T* _ptr);
    ASharedPtr(const ASharedPtr<T>& _copy);
    ASharedPtr<T>& operator=(const ASharedPtr<T>& _assign);
    //ASharedPtr(const ASharedPtr<T>&& _movecopy) = delete;
    //ASharedPtr<T>& operator=(const ASharedPtr<T>&& _moveassign) = delete;
    ~ASharedPtr();
    Void Release();
  public:
    T* const operator&() const;
    T* const operator*() const;
    T* const operator->() const;
  public:
    const Int GetRef() const;
    const Bool IsEmpty() const;

  private:
    SPtrProp* sptrp_;
    T* ptr_;
  };

  // explicit template instance
  // declare types you want to use.
  // at ashared_ptr.cc

}//!namespace atug2

#endif//!_ATUG2_SHARED_PTR_H_
