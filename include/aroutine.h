#ifndef _ATUG2_ROUTINE_H_
#define _ATUG2_ROUTINE_H_
#include "atug2.h"
#include "amemory.h"
#include "ashared_ptr.h"
#include "asafelist.h"

namespace atug2 {
  
  // answer from routine
  const enum RoutineAns {
    kOk = 0x00,
    kBreak = 0x10,
    kErr = 0x80,
  };

  class ATUG2_API ARoutine : public IHeapTracker {
  public:
    DONT_ALLOW_COPY_AND_ASSIGN(ARoutine)
    explicit ARoutine();
    virtual ~ARoutine();
    virtual const Bool Release() PURE;
    virtual const RoutineAns Run() PURE;
  public:
    const Bool IsPrepared() const;
  protected:
    Bool prepared_;
  };

}//!namespace atug2

#endif//!_ATUG2_ROUTINE_H_
