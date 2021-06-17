#include "asocket_comm_routine.h"
#include "athread.h"
#include <list>
#include <map>
#include <mutex>
#include "acomm_protocol.h"
#include "acomm_react.h"

namespace atug2 {

  ASocketCommRoutine::ASocketCommRoutine(ASharedPtr<ASocket> _s)
    : s_(_s), proc_(nullptr), reactor_(nullptr),
    proc_rsc_(new SockCommProp(*s_, nullptr, ZERO)) {
    PrepareBuffer(kDefaultBufSize);
  }
  ASocketCommRoutine::ASocketCommRoutine(const ASocketCommRoutine& _copy)
    : s_(ASharedPtr<ASocket>{}) {
  }
  const ASocketCommRoutine& ASocketCommRoutine ::operator=(const ASocketCommRoutine& _assign) {
    return *this;
  }
  ASocketCommRoutine ::~ASocketCommRoutine() {
    Release();
  }
  const Bool ASocketCommRoutine::Release() {
    SAFE_DELETE(proc_rsc_);
    SAFE_DELETE(reactor_);
    return true;
  }

  const Bool ASocketCommRoutine::Initialize(AsockCommProc _p, Reactor* _r) {
    // set procedure
    assert(nullptr NE _p);
    if (nullptr EQ _p) { return false; }
    proc_ = _p;

    assert(nullptr NE _r);
    if ((nullptr EQ _r) OR (false EQ _r->IsOnHeap())) { return false; }
    reactor_ = _r;

    return true;
  }

  Void ASocketCommRoutine::PrepareBuffer(const Uint _size) {
    proc_rsc_->PrepareBuffer(_size);
  }

  ASharedPtr<ASocket> ASocketCommRoutine::GetSocket() {
    return s_;
  }


  /**
  * server - tcp io multiplex 
  */
  AServerTcpMultiplexorRoutine::AServerTcpMultiplexorRoutine()
    : ASocketCommRoutine(ASocket::Create(SockRunType::kServerTcpMultiplexor)), fd_max_(0), fd_num_(0) {
    // set 1 for check if changed correctly at Initialize
    CLEAR_VAL(&reads_, fd_set, 1);
    CLEAR_VAL(&reads_remember_, fd_set, 1);
    CLEAR_VAL(&toutinfo_, timeval, 1);
  }

  AServerTcpMultiplexorRoutine::~AServerTcpMultiplexorRoutine() {
  }

  const Bool AServerTcpMultiplexorRoutine::Initialize(
    AsockCommProc _p, Reactor* _r,
    const Uint _timeout_ms /*= kDefaultTimeout*/, const char* const _addr_port_string /*= "8888"*/, const Uint _max_conn /*=kDefaultConnCnt*/)
  {
    if (!ASocketCommRoutine::Initialize(_p, _r)) {
      return false;
    }
    SockRes result = s_->CreateSocket();
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }
    Int timout{ kDefaultTimeout };
    result = s_->SetSockOption(SOL_SOCKET, SO_RCVTIMEO, &timout);
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }

    // set file descriptor
    FD_ZERO(&reads_);
    //FD_SET(0 /*stdin; console*/, &reads_);
    FD_SET(s_->GetSocket(), &reads_);
    // set linux fd_cnt base
    fd_max_ = s_->GetSocket();

    // set timeout
    toutinfo_.tv_sec = _timeout_ms/1000;
    toutinfo_.tv_usec = _timeout_ms%1000;

    result = s_->BindSocket(_addr_port_string);
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }
    result = s_->StartListening(_max_conn);
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }

    return (prepared_ = true);
  }

#pragma BUILD_MSG("error meaning check")
  Void AServerTcpMultiplexorRoutine::DealWithAcceptError() {
    switch (errno) {

    }
#if defined(_LINUX)
    errno;
#else defined(_WINDOWS)
    errno_t err{ WSAGetLastError() };
    _TRACE(err);
#endif
  }
  Void AServerTcpMultiplexorRoutine::DealWithRecvError(const Sock _client) {
    switch (errno) {
    case EPIPE:
      CloseConnection(_client);
      return;
    }
#if defined(_LINUX)
    errno;
#else defined(_WINDOWS)
    errno_t err{ WSAGetLastError() };
    _TRACE("WSAGetLastError: " << err);
    CloseConnection(_client);
    switch (err) {
    default:
      break;
    }//!switch
#endif
  }
  const RoutineAns AServerTcpMultiplexorRoutine::Run() {
    _TRACE("io multiplex routine execute.");
    RoutineAns res{ RoutineAns::kErr };

    // fd set remember
    reads_remember_ = reads_;
#if defined(_WINDOWS)
    const Uint fd_cnt{ reads_.fd_count };
#else
    const Uint fd_cnt{ fd_max_ + 1 }; // windows also can set but no work
#endif
    const Uint changed_fd_cnt = 
      select(fd_cnt, &reads_remember_, nullptr, nullptr, &toutinfo_);

    const Int err_occured{ -1 };
    const Int timeout_occured{ 0 };
    if (err_occured/*-1*/ EQ changed_fd_cnt) { // error
      res = RoutineAns::kErr;
    }
    else if (timeout_occured/*0*/ EQ changed_fd_cnt) { // timeout
      res = RoutineAns::kOk;
    }
    else { // changed file descriptor check
      Sock cur_sock{ NULL };

      for (Uint i = 0; i < fd_cnt; ++i) {
#if defined (_WINDOWS)
        if (FD_ISSET(reads_.fd_array[i], &reads_remember_)) {
          cur_sock = reads_.fd_array[i];
#else
        if (FD_ISSET(i, &s->reads_remember_)) {
          cur_sock = i;
#endif
          // connection request
          if (s_->GetSocket() EQ cur_sock) {
            Sock client_sock{ s_->Accept() }; // EQ accept(s_->GetSocket(), (struct sockaddr*)&client_addr, &addr_size);
            
            // err check
            if (INVALID_SOCKET EQ client_sock) {
              DealWithAcceptError();
            }
            // linux fd range set
            if (fd_max_ < client_sock) {
              fd_max_ = client_sock;
            }
            OpenConnection(client_sock);
          }
          // read -> do something
          else { 
            proc_rsc_->ResetBuffer();
            const Int read_length = proc_rsc_->read_length = 
              s_->Recv(cur_sock, proc_rsc_->buffer, proc_rsc_->buffer_size, NULL);

            if (ZERO < read_length) {
              proc_rsc_->SetOpponent(cur_sock); // set client to send message
              RoutineAns ans = proc_(proc_rsc_, reactor_); // call procedure 1 time
              if (RoutineAns::kOk NE ans) {
                _TRACE("server routine FAIL. close the client of " << cur_sock);
                CloseConnection(cur_sock);
              }
            } else if (ZERO EQ read_length) {
              //recv return 0 means opposite side closes the connection
              s_->Send(cur_sock, (BytePtr)"see you again! :)\n", 19, NULL);
            } else if (ZERO > read_length) {
              DealWithRecvError(cur_sock);
            }
          }//!read -> do something

        }//!if FD_ISSET
      }//!for fd_cnt

    }

    // broadcast if needed
    while (broadcast_msgs_.Size()) {
      auto msg{ broadcast_msgs_.PopFront() };
      auto packet{ CommProtocol::Assemble(OpCode::kBroadcast, kServerId, NULL, msg.Get(), msg.Size()) };
      s_->Broadcast(packet.Get(), packet.Size());
    }

    return res;
  }

  Void AServerTcpMultiplexorRoutine::OpenConnection(const Sock _client_sock) {
    FD_SET(_client_sock, &reads_);
#pragma BUILD_MSG("TEST");
    send(_client_sock, "welcome!", 8, NULL);
    _TRACE("client connected: " << _client_sock);
  }

  Void AServerTcpMultiplexorRoutine::CloseConnection(const Sock _client_sock) {
#pragma BUILD_MSG("TEST");
    send(_client_sock, "goodbye!", 8, NULL);
    s_->CloseConnectedSocketOf(_client_sock);
    FD_CLR(_client_sock, &reads_);
    _TRACE("client disconnected: " << _client_sock);
  }

  /**
  * client tcp
  */
  AClientTcpRoutine::AClientTcpRoutine() : ASocketCommRoutine(ASocket::Create(SockRunType::kClientTcp)) {

  }
  AClientTcpRoutine::~AClientTcpRoutine() {
  }
  const Bool AClientTcpRoutine::Initialize(AsockCommProc _p, Reactor* _r, const char* const _host, const char* const _port)
  {
    if (!ASocketCommRoutine::Initialize(_p, _r)) {
      return false;
    }
    SockRes result = s_->CreateSocket();
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }
    Int timout{ kDefaultTimeout };
    result = s_->SetSockOption(SOL_SOCKET, SO_RCVTIMEO, &timout);
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }

    proc_rsc_->SetOpponent(s_->GetSocket());

    result = s_->ConnectTo(_host, _port);
    if (SockRes::kSucceeded != result) { _TRACE(result); return false; }

    return (prepared_ =  true);
  }
  const RoutineAns AClientTcpRoutine::Run() {
    _TRACE("client routine start");
    return proc_(proc_rsc_, reactor_);
  }

  Void AClientTcpRoutine::CloseConnection() {
    closesocket(s_->GetSocket());
  }

  
}//! namespace atug2



