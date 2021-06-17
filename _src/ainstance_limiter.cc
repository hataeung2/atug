#include "ainstance_limiter.h"
#include <string>

#if defined(_WINDOWS)
typedef DWORD err_no;
typedef HANDLE handle;
#endif

namespace atug2 {

  class ATUG2_API AInstanceLimiterImpl {
    DONT_ALLOW_COPY_AND_ASSIGN(AInstanceLimiterImpl);
  public:
    AInstanceLimiterImpl() : already_exist_(false), last_err_(NULL), mutex_lock_(NULL) {}
    virtual ~AInstanceLimiterImpl() {}
  public:
    virtual const Bool IsInstanceMutexAlreadyExist() PURE;
  protected:
    Bool already_exist_;
    err_no last_err_;
    handle mutex_lock_;
  };
  class ATUG2_API ALimitInstImplWin : public AInstanceLimiterImpl {
  public:
    ALimitInstImplWin(wstring _mutexname) {
      const wstring mtxname{ L"aGlobal\\" + _mutexname };
      mutex_lock_ = CreateMutexW(NULL, TRUE, mtxname.c_str());
      if (mutex_lock_) {
        last_err_ = GetLastError();
        if (ERROR_ALREADY_EXISTS == last_err_) {
          already_exist_ = true;
        }
        _TRACE("LAST ERROR: " << (last_err_));
      } else {
        already_exist_ = false;
      }
    }
    virtual ~ALimitInstImplWin() {
      if (mutex_lock_) {
        CloseHandle(mutex_lock_);
        mutex_lock_ = NULL;
      }
    }
  public:
    virtual const Bool IsInstanceMutexAlreadyExist() {
      return already_exist_;
    }
  };

  AInstanceLimiter::AInstanceLimiter(wstring _mutexname) {
#if defined(_WINDOWS)
    attr_ = new ALimitInstImplWin(_mutexname);
#elif defined(_LINUX)
    attr_ = new ALimitInstImplLinux(_mutexname);
#endif
  }
  AInstanceLimiter::~AInstanceLimiter() {
    SAFE_DELETE(attr_);
  }

  const Bool AInstanceLimiter::IsInstanceMutexAlreadyExist() {
    return attr_->IsInstanceMutexAlreadyExist();
  }

}//!namespace atug
