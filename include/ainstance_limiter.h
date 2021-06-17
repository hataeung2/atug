#ifndef _ATUG2_LIMIT_INSTANCE_H_
#define _ATUG2_LIMIT_INSTANCE_H_
#include "atug2.h"
#include "amem_macro.h"

namespace atug2 {
  class AInstanceLimiterImpl;
  class ATUG2_API AInstanceLimiter {
    DONT_ALLOW_COPY_AND_ASSIGN(AInstanceLimiter);
  public:
    /**
    * @param _mutexname unique name for preventing other process open
    */
    AInstanceLimiter(wstring _mutexname);
    ~AInstanceLimiter();
  public:
    const Bool IsInstanceMutexAlreadyExist();
  private:
    AInstanceLimiterImpl* attr_;
  };
}//!namespace atug2

#endif//!_ATUG2_LIMIT_INSTANCE_H_
