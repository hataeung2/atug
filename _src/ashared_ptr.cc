#include "ashared_ptr.h"
#include <mutex>
#include <set>
#include <map>

#include "aexception.h"
#include "afile.h"
#include "alog.h"
#include "athread.h"
#include "aroutine.h"
#include "asocket.h"
#include "asafelist.h"


namespace atug2 {
  template class ASharedPtr<AFile>;
  template class ASharedPtr<ALog>;
  template class ASharedPtr<AThread>;
  template class ASharedPtr<ARoutine>;
  template class ASharedPtr<ASocket>;


#define DOOR (sptrp_->door)
  typedef const void* RawAddr;
  static std::set<RawAddr> addrs_;


  struct SPtrProp {
    SPtrProp() : refcnt{ 1 } {}
    ~SPtrProp() { Release(); }
    Void Release() { refcnt = 0; }
    const Int& IncRef() { return ++refcnt; }
    const Int& DecRef() { return --refcnt; }
    std::mutex door;
    Int refcnt;
  };

  template<typename T>
  ASharedPtr<T>::ASharedPtr()
    : sptrp_(new SPtrProp), ptr_(nullptr) {
  }
  template<typename T>
  ASharedPtr<T>::ASharedPtr(T* _ptr)
    : sptrp_(new SPtrProp), ptr_(_ptr) {
    if (false EQ AManagedHeap::IsOnHeap(_ptr)) {
      SAFE_DELETE(sptrp_);
      throw AException("ASharedPtr MUST contain a memory block on AManagedHeap.");
    }
    if (addrs_.end() NE addrs_.find(ptr_)) {
      SAFE_DELETE(sptrp_);
      throw AException("ASharedPtr MUST contain ONE individual memory block at ONE ASharedPtr.");
    } else {
      addrs_.insert(ptr_);
    }
    assert(nullptr NE _ptr);
  }
  template<typename T>
  ASharedPtr<T>::ASharedPtr(const ASharedPtr<T>& _copy)
    : sptrp_(nullptr), ptr_(nullptr) {
    _TRACE("ASharedPtr copied");
    *this = _copy; // using assign
  }
  template<typename T>
  ASharedPtr<T>& ASharedPtr<T>::operator=(const ASharedPtr<T>& _assign) {
    SCOPELOCK((_assign.sptrp_->door));
    _assign.sptrp_->IncRef();

    // old ptr released
    if (ptr_ NE _assign.ptr_) { 
      this->Release(); 
    }
    _TRACE("ASharedPtr assigned");
    // assign
    sptrp_ = _assign.sptrp_;
    ptr_ = _assign.ptr_;
    return *this;
  }
  //// move ctor
  //template<typename T>
  //ASharedPtr<T>::ASharedPtr(const ASharedPtr<T>&& _move)
  //  : sptrp_(std::move(_move.sptrp_)), ptr_(std::move(_move.ptr_)) {
  //  _TRACE("ASharedPtr move contructed");
  //  return *this;
  //}
  //// move assigner
  //template<typename T>
  //ASharedPtr<T>& ASharedPtr<T>::operator=(const ASharedPtr<T>&& _massign) {
  //  sptrp_ = std::move(_massign.sptrp_);
  //  ptr_ = std::move(_massign.ptr_);
  //  _TRACE("ASharedPtr move assigned");
  //  return *this;
  //}


  template<typename T>
  ASharedPtr<T>::~ASharedPtr() {
    Release();
  }
  template<typename T>
  Void ASharedPtr<T>::Release() {
    if (nullptr NE this->sptrp_) {
      LOCK(DOOR);
      if (ZERO >= sptrp_->DecRef()) {
        if (ptr_) {
          addrs_.erase(ptr_);
          SAFE_DELETE(ptr_);
          _TRACE("allocated ptr_ deleted.");
        }
        UNLOCK(DOOR);
        SAFE_DELETE(sptrp_);
        return;
      }
      UNLOCK(DOOR);
    }
    _TRACE("ASharedPtr released");
  }

  template<typename T>
  T* const ASharedPtr<T>::operator&() const {
    SCOPELOCK(DOOR);
    return ptr_;
  }

  template<typename T>
  T* const ASharedPtr<T>::operator*() const {
    SCOPELOCK(DOOR);
    return ptr_;
  }

  template<typename T>
  T* const ASharedPtr<T>::operator->() const {
    SCOPELOCK(DOOR);
    return ptr_;
  }

  template<typename T>
  const Int ASharedPtr<T>::GetRef() const {
    if (!sptrp_) return 0;
    SCOPELOCK(DOOR);
    return sptrp_->refcnt;
  }

  template<typename T>
  const Bool ASharedPtr<T>::IsEmpty() const {
    return (nullptr EQ ptr_);
  }

}//! namespace atug


