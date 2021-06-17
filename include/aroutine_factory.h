#ifndef _ATUG2_ROUTINE_FACTORY_H_
#define _ATUG2_ROUTINE_FACTORY_H_
#include "asocket_comm_routine.h"

namespace atug2 {
  class ATUG2_API ARoutineFactory {
  public:
    static ARoutine* NewServerTcpMultiplex(
      AsockCommProc _proc, Reactor* _reactor,
      const Uint _timeout_ms = kDefaultTimeout,
      const char* const _addr_port_string = "8888", 
      const Uint _max_conn = kDefaultConnCnt) {
      
      AServerTcpMultiplexorRoutine* r{ new AServerTcpMultiplexorRoutine };
      if (true EQ r->Initialize(_proc, _reactor, _timeout_ms, _addr_port_string, _max_conn)) {
        return r;
      } else {
        return nullptr;
      }
    }

    static ARoutine* NewClientTcp(AsockCommProc _proc, Reactor* _reactor, 
      const char* const _host, const char* const _port) {

      AClientTcpRoutine* r{ new AClientTcpRoutine };
      if (true EQ r->Initialize(_proc, _reactor, _host, _port)) {
        return r;
      } else {
        return nullptr;
      }
    }
  };
}//!namespace atug2

#endif//!_ATUG2_ROUTINE_FACTORY_H_
