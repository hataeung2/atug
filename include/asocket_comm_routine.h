#ifndef _ATUG2_SOCKET_COMM_ROUTINE_H_
#define _ATUG2_SOCKET_COMM_ROUTINE_H_
#include "aroutine.h"
#include "asocket.h"
#include <array>
#include <map>
#include <thread>
#include <functional>

namespace atug2 {
  class Reactor;

  // for Socket Communication Procedure
#pragma message("server, client, param, ...?")
  struct TempBuffer {
    TempBuffer(BytePtr _buf, const Uint _buf_size) : buffer(nullptr), buffer_size(ZERO) {
      PrepareBuffer(_buf_size);
      memcpy_s(buffer, buffer_size, _buf, _buf_size);
    }
    ~TempBuffer() {
      SAFE_DELETE_ARR(buffer);
      buffer_size = ZERO;
    }
    Void PrepareBuffer(const Uint _size) {
      Uint size{ _size };
      if (kDefaultBufSize * 10 < _size) {
        _TRACE("buffer size can't exceed 10 times bigger than kDefaultBufSize.");
        size = kDefaultBufSize * 10;
      }
      delete[] buffer;
      buffer = new Byte[size];
      CLEAR_VAL(buffer, Byte, size);
      buffer_size = size;
    }
    Void ResetBuffer() {
      CLEAR_VAL(buffer, Byte, buffer_size);
    }
    BytePtr buffer;
    Uint buffer_size;
  };
  struct SockCommProp : TempBuffer {
    SockCommProp(ASocket* _s, BytePtr _buf, const Uint _buf_size)
      : TempBuffer(_buf, _buf_size), read_length(ZERO), self(_s), opponent(kInvalidSock) {
    }
    ~SockCommProp() {
      read_length = ZERO;
      self = nullptr;
      opponent = kInvalidSock;
    }
    Void SetOpponent(const Sock _opponent) {
      opponent = _opponent;
    }
    Void ResetBuffer() {
      TempBuffer::ResetBuffer();
      read_length = ZERO;
    }
    const SizeT IsDataRecvd() const {
      return read_length;
    }
    adata::ADataBucket& GetRecvBucket() {
      auto it{ recvbuckets.find(opponent) };
      if (recvbuckets.end() EQ it) { 
        auto d{ adata::ADataBucket(kDefaultBufSize) }; recvbuckets.insert(std::map<Sock, adata::ADataBucket>::value_type(opponent, d)); 
        return d;
      } else {
        return it->second;
      }
    }
    adata::ADataBucket& GetSendBucket() {
      auto it{ sendbuckets.find(opponent) };
      if (sendbuckets.end() EQ it) {
        auto d{ adata::ADataBucket(kDefaultBufSize) }; sendbuckets.insert(std::map<Sock, adata::ADataBucket>::value_type(opponent, d));
        return d;
      } else {
        return it->second;
      }
    }

    SizeT read_length;
    ASocket* self;
    Sock opponent;
    std::map<Sock, adata::ADataBucket> recvbuckets;
    std::map<Sock, adata::ADataBucket> sendbuckets;
    adata::ADataBucket react;
  };


  // function pointer "AsockCommProc"
  typedef const RoutineAns(*AsockCommProc)(SockCommProp* _p, Reactor* _r);
  // std function "AsockCommProc"
  //typedef std::function <const RoutineAns(ASocket* _s, SockCommProp* _p)> AsockCommProc;
  // lambda define "ASOCK_PROC"
  #define ASOCK_PROC [](SockCommProp* _p, Reactor* _r) -> const RoutineAns


  class ATUG2_API ASocketCommRoutine : public ARoutine {
  public:
    ASocketCommRoutine(ASharedPtr<ASocket> _s);
    ASocketCommRoutine(const ASocketCommRoutine& _copy);
    const ASocketCommRoutine & operator=(const ASocketCommRoutine& _assign);
    virtual ~ASocketCommRoutine();
  public:
    virtual const Bool Release() override;
    virtual const RoutineAns Run() PURE;
  public:
    const Bool Initialize(AsockCommProc _proc, Reactor* _r);
  public:
    Void PrepareBuffer(const Uint _size);
  public:
    ASharedPtr<ASocket> GetSocket();
  protected:
    ASharedPtr<ASocket> s_; // socket itself
    AsockCommProc proc_; // procedure set
    Reactor* reactor_;
  protected:
    SockCommProp* proc_rsc_; // parameters for procedure
  };

  /**
  * server - tcp io multiplex
  */
  class ATUG2_API AServerTcpMultiplexorRoutine : public ASocketCommRoutine {
  private:
    friend class ARoutineFactory;
    AServerTcpMultiplexorRoutine();
  public:
    virtual ~AServerTcpMultiplexorRoutine();
    const Bool Initialize(AsockCommProc _proc, Reactor* _reactor,
      const Uint _timeout_ms, const char* const _addr_port_string, const Uint _max_conn);
  public:
    Void DealWithAcceptError();
    Void DealWithRecvError(const Sock _client);
    virtual const RoutineAns Run() final;
  public:
    Void OpenConnection(const Sock _client_sock);
    Void CloseConnection(const Sock _client_sock);
  public:
    Void PushBroadcastMsg(const adata::ADataBucket _msg);

  private:
    fd_set reads_, reads_remember_;
    timeval toutinfo_;
    Int fd_max_; // only for unix like os
    Int fd_num_;
  private:
    ASafeList<adata::ADataBucket> broadcast_msgs_;
  };

  /**
  * client
  */
  class ATUG2_API AClientTcpRoutine : public ASocketCommRoutine {
  public:
    AClientTcpRoutine();
  public:
    virtual ~AClientTcpRoutine();
  public:
    virtual const Bool Initialize(AsockCommProc _p, Reactor* _r, const char* const _host, const char* const _port);
  public:
    virtual const RoutineAns Run() final;
    Void CloseConnection();
  };
 
}//!namespace atug2

#endif//!_ATUG2_SOCKET_COMM_ROUTINE_H_
