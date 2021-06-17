#include "amemory.h"
#include "amem_macro.h"
#include <set>
#include <map>
#include "atime.h"
#include <mutex>

namespace atug2 {

  typedef const void* RawAddr;
  static std::set<RawAddr> addrs_;
  static std::mutex door_;

#if defined(_DEBUG)
  struct AllocInfo {
    AllocInfo(const char* _file, const int _line, const size_t _size) : filename(_file), line(_line), size(_size) {
      atug2::_GetLocalTime(&time);
    }
    string filename;
    int line;
    size_t size;
    Time time;
  };
  static std::map<RawAddr, AllocInfo> allocinfo_;
#endif

  IHeapTracker::IHeapTracker() {
  }

  IHeapTracker::~IHeapTracker() {
  }

  void* IHeapTracker::operator new(size_t _size) {
    void* memPtr{ ::operator new(_size) };
    addrs_.insert(memPtr);
    return memPtr;
  }
  void IHeapTracker::operator delete(void* _ptr) {
    SCOPELOCK(door_);
    std::set<RawAddr>::iterator it{ addrs_.find(_ptr) };
    if (addrs_.end() NE it) {
      addrs_.erase(it);
      ::operator delete(_ptr);
    }
  }
  void* IHeapTracker::operator new[](size_t _size) {
    void* memPtr{ ::operator new[](_size) };
    addrs_.insert(memPtr);
    return memPtr;
  }
  void IHeapTracker::operator delete[](void* _ptr) {
    SCOPELOCK(door_);
    std::set<RawAddr>::iterator it{ addrs_.find(_ptr) };
    if (addrs_.end() NE it) {
      addrs_.erase(it);
      ::operator delete[](_ptr);
    }
  }
  const bool IHeapTracker::IsOnHeap() const {
    RawAddr rawAddr{ dynamic_cast<RawAddr>(this) };
    std::set<RawAddr>::iterator it{ addrs_.find(rawAddr) };
    return (addrs_.end() NE it);
  }


  void* IHeapTracker::operator new(size_t _size, const char* _file, int _line) {
    void* memPtr{ AManagedHeap::Allocate(_size, _file, _line) };
    addrs_.insert(memPtr);
    return memPtr;
  }
  void* IHeapTracker::operator new[](size_t _size, const char* _file, int _line) {
    void* memPtr{ AManagedHeap::Allocate(_size, _file, _line) };
    addrs_.insert(memPtr);
    return memPtr;
  }

  void* AManagedHeap::Allocate(const size_t _size) {
    void* memPtr{ malloc(_size) };
    addrs_.insert(memPtr);
    memset(memPtr, 0, _size);
    return memPtr;
  }
  void* AManagedHeap::Allocate(const size_t _size, const char* _file, const int _line) {
    void* memPtr{ malloc(_size) };
    addrs_.insert(memPtr);
#if defined(_DEBUG)
    allocinfo_.insert(std::map<RawAddr, AllocInfo>::value_type(
      memPtr, AllocInfo(_file, _line, _size)
    ));
#endif
    memset(memPtr, 0, _size);
    return memPtr;
  }
  void AManagedHeap::Free(void* _ptr) {
    LOCK(door_);
    std::set<RawAddr>::iterator it{ addrs_.find(_ptr) };
    if (addrs_.end() NE it) {
      addrs_.erase(it);
      free(_ptr);
    }
    UNLOCK(door_);
#if defined(_DEBUG) 
    { std::map<RawAddr, AllocInfo>::iterator it{ allocinfo_.find(_ptr) };
      if (allocinfo_.end() NE it) {
        allocinfo_.erase(it);
    } }
#endif
    _ptr = nullptr;
  }
  const bool AManagedHeap::IsOnHeap(const void* _ptr) {
    RawAddr rawAddr{ static_cast<RawAddr>(_ptr) };
    std::set<RawAddr>::iterator it{ addrs_.find(rawAddr) };
    return (addrs_.end() NE it);
  }
  const bool AManagedHeap::IsEmpty() {
    return (0 EQ addrs_.size());
  }

  const size_t AManagedHeap::Count() {
    return addrs_.size();
  }

  struct RefCntProp {
    RefCntProp() : refcnt(0) {}
    ~RefCntProp() { refcnt = 0; }
    Number refcnt;  
    std::mutex door;
  };
  IRefCounter::IRefCounter() : refp_(new RefCntProp) {
    operator++();
  }
  IRefCounter::~IRefCounter() {
    operator--();
    if (ZERO >= refp_->refcnt) { 
      SAFE_DELETE(refp_); 
    }
  }
  IRefCounter::IRefCounter(const IRefCounter& _copy) {
    this->refp_ = _copy.refp_;
    operator++();
  }
  IRefCounter& IRefCounter::operator++() {
    SCOPELOCK(refp_->door);
    ++(refp_->refcnt);
    return (*this);
  }
  IRefCounter& IRefCounter::operator--() {
    SCOPELOCK(refp_->door);
    --(refp_->refcnt);
    return (*this);
  }
  const Bool IRefCounter::IsReferenced() const {
    return (1 < refp_->refcnt) ? true : false;
  }
  const Number IRefCounter::RefCnt() const {
    return refp_->refcnt;
  }
}//!namespace atug2 {