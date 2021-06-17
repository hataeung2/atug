#ifndef _ATUG2_THREAD_H_
#define _ATUG2_THREAD_H_
#include "atug2.h"
#include "amemory.h"
#include "ashared_ptr.h"

namespace atug2 {
  class ARoutine;
  
  typedef unsigned int ThreadId;
  typedef unsigned int Interval;
  typedef unsigned int EventNo;

  enum ThreadResult {
    kThreadComplete,
    kThreadError,
    kThreadReleased
  };
    
  struct ThreadAttr;
  class ATUG2_API AThread : public IHeapTracker {
    DONT_ALLOW_COPY_AND_ASSIGN(AThread)
    friend class ASharedPtr<AThread>;
  public:
    ~AThread();
  public:
    static ASharedPtr<AThread> Create(ASharedPtr<ARoutine> _routine, const Uint _interval = 1/*ms*/);
  private:
    AThread(ASharedPtr<ARoutine> _routine, const Uint _interval);
    const ThreadResult Release();
  public:
    Void Start();
    Void Join();
  public:
    const Bool Suspend();
    const Bool Resume();
    Void Stop();
    const Bool IsEscapedFromLoop();
  public:
    inline Void SetInterval(const Uint _interval);
    inline Void SetThreadConditionIntoBreak();

  private:
    ThreadAttr* attr_;
  };
}//!namespace atug2

#endif//!_ATUG2_THREAD_H_
