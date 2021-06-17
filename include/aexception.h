#ifndef _ATUG2_EXCEPTION_H_
#define _ATUG2_EXCEPTION_H_

#include <exception>  // 예외와 bad_exception
#include <stdexcept>  // 대부분은 논리와 실행시간 오류
#include <system_error> // 시스템 오류
#include <new>  // 메모리 부족
#include <ios>  // I/O 예외
#include <future> // async()와 future ..
#include <typeinfo> // bad_cast와 bad_typeid

namespace atug2 {
  class ATUG2_API AException {
  public:
    AException(Str _msg) {
      _TRACE("AException! \t" << _msg);
#ifdef _WINDOWS
      const short kErrMsgLen{ 512 };
      char msg[kErrMsgLen]{ "> AException! :\t" };
      memcpy_s((void*)&msg[16], kErrMsgLen, _msg, strnlen_s(_msg, kErrMsgLen));
      memcpy_s((void*)&msg[strnlen_s(msg, kErrMsgLen)], kErrMsgLen, "\r\n", 2);
      _DBGOUT(msg);
#endif
    }
  };
}//!namespace atug2 {

#endif//!_ATUG2_EXCEPTION_H_