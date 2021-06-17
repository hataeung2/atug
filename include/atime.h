#ifndef _ATUG2_TIME_H_
#define _ATUG2_TIME_H_
#include "atug2.h"
#include "adata.h"
#include <sys/timeb.h> // millisec
#include <ctime>
#include <thread>
#include <chrono>

using namespace std::chrono;

namespace atug2 {
  enum class TimeUnit : char { kDay, kHour, kMin, kSec, kMilliSec, kMicroSec, kNanoSec };
  //to use _asleep(s) include <thread>, <chrono>
#define _asleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))

#ifdef __cplusplus
   extern "C" {
#endif

#define GET_LOCALTIME(time) _ftime_s(&(time)->tb); localtime_s(&(time)->t, &(time)->tb.time)
    struct Time {
      _timeb tb;
      tm t;
    };
    ATUG2_API Void _GetLocalTime(Time* _time);

#ifdef __cplusplus
  }
#endif

  class ATUG2_API ALocalTime {
  public:
    ALocalTime();
    ~ALocalTime();
  public:
    static const Time Get();
    static astring GetWstr(const TimeUnit _unit = TimeUnit::kMilliSec);
    static string GetStr(const TimeUnit _unit = TimeUnit::kMilliSec);
    static const time_t GetElapsedTime(const Time& _before, const Time& _after = Get());
  };

  /**
  * @see ClockTypes
  * std::chrono::system_clock, steady_clock, high_resolution_clock
  */
  template <typename ClockType> 
  class AClock {
  public:
    AClock() : start_(ClockType::now()), end_(ZERO) {}
    ~AClock() {}
    typedef std::chrono::time_point<ClockType> SystemClock;
    typedef Longlong Elapsed;
  public:
    Void Start() { start_ = ClockType::now(); }
    Elapsed End(const TimeUnit _unit = kMilliSec) { 
      end_ = ClockType::now(); 
      return GetElapsedTime(_unit);
    }
  private:
    Longlong GetElapsedTime(const TimeUnit _unit) {
      if (ZERO EQ end_) { End(); }
      switch (_unit) {
      case kDay:
        return (std::chrono::duration_cast<std::chrono::hours>(end_ - start_)).count() / 24;
      case kHour:
        return (std::chrono::duration_cast<std::chrono::hours>(end_ - start_)).count();
      case kMin:
        return (std::chrono::duration_cast<std::chrono::minutes>(end_ - start_)).count();
      case kSec:
        return (std::chrono::duration_cast<std::chrono::seconds>(end_ - start_)).count();
      case kMilliSec:
        return (std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_)).count(); 
      case kMicroSec:
        return (std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_)).count(); 
      case kNanoSec:
        return (std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_)).count(); 
      default: // milliseconds
        return (std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_)).count(); 
      }
    }
      
  private:
    std::chrono::time_point<ClockType> start_;
    std::chrono::time_point<ClockType> end_;
  };

}//! namespace atug2


#endif//!_ATUG2_TIME_H_