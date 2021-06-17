#include "atime.h"
#include <array>
#pragma warning(disable: 4820)
namespace atug2 {
  Void _GetLocalTime(Time* _time) {
    _ftime_s(&(_time->tb));
    localtime_s(&(_time->t), &(_time->tb.time));
  }
  const time_t _GetElapsedTime(const Time* _after, const Time* _before) {
    return (_after->tb.time - _before->tb.time) * 1000 + (_after->tb.millitm - _before->tb.millitm);
  }
  
  ALocalTime::ALocalTime() {
  }

  ALocalTime::~ALocalTime() {
  }

  const Time ALocalTime::Get() {
    Time t; _GetLocalTime(&t);
    return t;
  }

  astring ALocalTime::GetWstr(const TimeUnit _unit /*= kMilliSec*/) {
    Time t; _GetLocalTime(&t);
    const size_t time_length{ 128 };
    std::array<Wchar, time_length> time;
    switch (_unit) {
    case TimeUnit::kDay:
      std::swprintf(time.data(), time_length, L"%0004d%02d%02d",
        t.t.tm_year + 1900, t.t.tm_mon + 1, t.t.tm_mday);
      break;
    case TimeUnit::kHour:
      std::swprintf(time.data(), time_length, L"%0004d%02d%02d %02d",
        t.t.tm_year + 1900, t.t.tm_mon + 1, t.t.tm_mday,
        t.t.tm_hour);
      break;
    case TimeUnit::kMin:
      std::swprintf(time.data(), time_length, L"%0004d%02d%02d %02d%02d",
        t.t.tm_year + 1900, t.t.tm_mon + 1, t.t.tm_mday,
        t.t.tm_hour, t.t.tm_min);
      break;
    case TimeUnit::kSec:
      std::swprintf(time.data(), time_length, L"%0004d%02d%02d %02d%02d%02d",
        t.t.tm_year + 1900, t.t.tm_mon + 1, t.t.tm_mday,
        t.t.tm_hour, t.t.tm_min, t.t.tm_sec);
      break;
    case TimeUnit::kMilliSec:
      std::swprintf(time.data(), time_length, L"%0004d%02d%02d %02d%02d%02d %003d",
        t.t.tm_year + 1900, t.t.tm_mon + 1, t.t.tm_mday,
        t.t.tm_hour, t.t.tm_min, t.t.tm_sec,
        t.tb.millitm);
      break;
    default:
      throw std::exception();
    }
    return time.data();
  }
  string ALocalTime::GetStr(const TimeUnit _unit /*= TimeUnit::kMilliSec*/) {
    auto wstr{ GetWstr(_unit) };
    return adata::StrFromWcs(wstr);
  }

  const time_t ALocalTime::GetElapsedTime(const Time & _before, const Time & _after /*= Get()*/) {
    return (_after.tb.time - _before.tb.time) * 1000 + (_after.tb.millitm - _before.tb.millitm);
  }

}//! namespace atug


