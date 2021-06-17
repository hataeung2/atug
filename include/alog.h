#ifndef _ATUG2_LOG_H_
#define _ATUG2_LOG_H_
#include "atug2.h"
#include "adata.h"
#include "ashared_ptr.h"
#include "atime.h"
#define LOGTHREAD_TIMEOUT 1000U

namespace atug2 {
  //ATUG2_API extern const char* GetHexString(Void* _target, const int _target_size, const char* _empty_string);
  ATUG2_API extern const astring GetLogDir(const astring& _additional_path = L"log");

  struct LogProp;
  class ATUG2_API ALog : public IHeapTracker {
    DONT_ALLOW_COPY_AND_ASSIGN(ALog);
    friend class ASharedPtr<ALog>;
  private: explicit ALog();
  public: ~ALog();
  private: Void Release();
  public: 
    static ASharedPtr<ALog> Create(const astring _path = L"log", const TimeUnit _timeunit = TimeUnit::kDay, 
                                  const adata::CharEnc _enc = adata::CharEnc::kUtf8);
  public:
    Void Start();
    Void Stop();
  public:
    const Uint Push(const string& _str);
    const Uint Push(const astring& _str);
  private:
    const Bool IsStarted();
  private:
    LogProp* logp_;
  };
}//!namespace atug2

#endif//!_ATUG2_LOG_H_