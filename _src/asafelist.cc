#include "asafelist.h"
#include <mutex>
#include <list>

namespace atug2 {
  using namespace adata;
  #define TEMPLATET template<typename T> 
  #define DOOR (slistp_->lock_)

  template class ASafeList<ADataBucket>;
  template class ASafeList<AStringContainer>;

  // SafeListProp
  TEMPLATET struct SafeListProp {
    SafeListProp() { }
    std::mutex lock_;
    std::list<T> list_;
  };
  //!SafeListProp


    
  // ASafeList
  TEMPLATET ASafeList<T>::ASafeList()
    : slistp_(new SafeListProp<T>) {
  }
  TEMPLATET ASafeList<T>::~ASafeList() {
    Clear();
    SAFE_DELETE(slistp_);
  }
  TEMPLATET Void ASafeList<T>::Clear() {
    SCOPELOCK(DOOR);
    slistp_->list_.clear();
  }
  TEMPLATET const Bool ASafeList<T>::IsEmpty() {
    SCOPELOCK(DOOR);
    return slistp_->list_.empty();
  }
  TEMPLATET const SizeT ASafeList<T>::Size() {
    SCOPELOCK(DOOR);
    return slistp_->list_.size();
  }
  TEMPLATET const SizeT ASafeList<T>::PushBack(T _data) {
    SCOPELOCK(DOOR);
    slistp_->list_.push_back(_data);
    return slistp_->list_.size();
  }

  TEMPLATET const SizeT ASafeList<T>::PushFront(T _data) {
    SCOPELOCK(DOOR);
    slistp_->list_.push_front(_data);
    return slistp_->list_.size();
  }

  TEMPLATET const T ASafeList<T>::PopBack() {
    SCOPELOCK(DOOR);
    T poped = slistp_->list_.back();
    slistp_->list_.pop_back();
    return poped;
  }

  TEMPLATET const T ASafeList<T>::PopFront() {
    SCOPELOCK(DOOR);
    T poped = slistp_->list_.front();
    slistp_->list_.pop_front();
    return poped;
  }
  //!ASafeList
    
}//!namespace atug2
