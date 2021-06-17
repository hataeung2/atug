#include "asocket.h"
#include "adata.h"
#include <WS2tcpip.h>
#include <map>
#include <mutex>
#pragma warning(disable: 4302)
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)
namespace atug2 {

  // IPv4
  astring GetSaInfoStr(const SockAddrIn4& _addr) {
    char buf[128]{};
    inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
    string info_str = "IP: ";
    info_str += buf;
    info_str += " PORT: ";
    info_str += adata::ToStr((Int)ntohs(_addr.sin_port));
    return adata::WcsFromStr(info_str);
  };
  // IPv6
  astring GetSaInfoStr(const SockAddrIn6& _addr) {
    char buf[128]{};
    inet_ntop(AF_INET, &_addr.sin6_addr, buf, sizeof(buf));
    string info_str = "IP: ";
    info_str += buf;
    info_str += " PORT: ";
    info_str += adata::ToStr((Int)ntohs(_addr.sin6_port));
    return adata::WcsFromStr(info_str);
  };


  //# ASocket::SockProp
  class ASocket::SockProp {
  public:
    SockProp(const AddrFamily _af, const SockType _t, const Sock _s) 
      : af_(_af), type_(_t), s_(_s) { }
    const SockProp& operator=(const SockProp& _assign) {
      type_ = _assign.type_;
      s_ = _assign.s_;
      af_ = _assign.af_;
      _TRACE("socket property assigned");
    }
    virtual ~SockProp() {
      s_ = NULL;
      _TRACE("socket property released");
    }
    virtual const SockAddr* GetSockAddr() PURE;
    virtual const Int GetSockAddrSize() PURE;
    virtual const astring GetInfoStr() PURE;
    SockType type_; // STREAM, DGRAM
    Sock s_;
    AddrFamily af_;
  };
  class ASocket::ASa4 : public ASocket::SockProp {
  public:
    ASa4(const AddrFamily _af, const SockType _t, const Sock _s) 
      : SockProp(_af, _t, _s) {
    }
    Void SetAddr(const char* const _host, const char* const _port) {
      CLEAR_VAL(&sockaddr_in_, sizeof(sockaddr_in_), 1); 
      sockaddr_in_.sin_family = af_;
      inet_pton(af_, _host, &sockaddr_in_.sin_addr);
      sockaddr_in_.sin_port = htons((Ushort)(atoi(_port)));
    }
    Void SetAddr(const AddrHostNumber _addr_host_number, const AddrPortNumber _addr_port_number) {
      CLEAR_VAL(&sockaddr_in_, sizeof(sockaddr_in_), 1); 
      sockaddr_in_.sin_family = af_;
      sockaddr_in_.sin_addr.s_addr = htonl(_addr_host_number);
      sockaddr_in_.sin_port = htons(_addr_port_number);
    }
    virtual const SockAddr* GetSockAddr() override { return (const SockAddr*)&sockaddr_in_; }
    virtual const Int GetSockAddrSize() override { return sizeof(sockaddr_in); }
    virtual const astring GetInfoStr() override { return GetSaInfoStr(sockaddr_in_); }
  private:
    SockAddrIn4 sockaddr_in_;
  };
  class ASocket::ASa6 : public ASocket::SockProp {
  public:
    ASa6(const AddrFamily _af, const SockType _t, const Sock _s) 
      : SockProp(_af, _t, _s) {
    }
    const Bool SetAddr(
      const char* const _host, 
      const char* const _port, 
      const Int _ai_flags = AI_PASSIVE) {
      Bool set{ false };
      CLEAR_VAL(&sockaddr_in_, sizeof(sockaddr_in_), 1); 
      if (nullptr NE _host) {
        // for getaddrinfo
        struct addrinfo hints, *res = nullptr, *result = nullptr;
        CLEAR_VAL(&hints, addrinfo, 1);
        hints.ai_family = af_;
        hints.ai_socktype = type_;
        hints.ai_flags = _ai_flags;
        if (ZERO NE getaddrinfo(_host, _port, &hints, &result)) {
          _TRACE("getaddrinfo ERROR for address of " << _host << ":" << _port);
          return set = false;
        }
        res = result;
        while(res) {
          if (af_/*AF_INET6*/ EQ res->ai_family) {
            memcpy(&sockaddr_in_, res->ai_addr, res->ai_addrlen);
            set = true;
          } res = res->ai_next;
        }//!while
        freeaddrinfo(result);
        return set;
      }
      else {
        // TODO: SCOPE ID ���� ��ȿ���� Ȯ��
        sockaddr_in_.sin6_family = af_;
        sockaddr_in_.sin6_flowinfo = 0;
        sockaddr_in_.sin6_addr = in6addr_any;
        sockaddr_in_.sin6_port = htons((Ushort)(atoi(_port)));
        return set = true;
      }
    }
    virtual const SockAddr* GetSockAddr() override { return (const SockAddr*)&sockaddr_in_; }
    virtual const Int GetSockAddrSize() override { return sizeof(sockaddr_in); }
    virtual const astring GetInfoStr() override { return GetSaInfoStr(sockaddr_in_); }
  private:
    SockAddrIn6 sockaddr_in_;
  };
  //#!ASocket::SockProp


  struct ASocket::ClientList {
    Void Insert(const Sock& _s, const string& _add_info) {
      std::lock_guard<std::mutex> lock(door_);
      _TRACE("New client socket accepted. Handle: " << _s << " " << (_add_info.c_str()));
      sockets_.insert(std::map<const Sock, string>::value_type(_s, _add_info));
    }
    const Bool Remove(const Sock& _s) {
      std::lock_guard<std::mutex> lock(door_);
      auto it{ sockets_.find(_s) };
      if (sockets_.end() NE it) {
        _TRACE("Closing client. fd: " << _s << " " << (it->second.c_str()));
        sockets_.erase(it->first);
      }
      return (ZERO EQ closesocket(_s));
    }
    const Bool ResetContainer() {
      int fail_cnt{ 0 };
      std::lock_guard<std::mutex> lock(door_);
      for (auto it = sockets_.begin(); sockets_.end() NE it; ++it) {
        _TRACE("Closing client. fd: " << it->first << " " << (it->second.c_str()));
        if (ZERO NE closesocket(it->first)) {
          _TRACE("Socket closing failed. fd: " << it->first);
          ++fail_cnt;
        }
      }
      sockets_.clear();
      return (ZERO EQ fail_cnt);
    }
    Void SetClientInfo(const Sock _s, const string& _info) {
      auto it = sockets_.find(_s);
      if (sockets_.end() NE it) {
        it->second = _info;
      }
    }
    const string GetClientInfoOf(const Sock _s, const string& _key) {
      auto it = sockets_.find(_s);
      if (sockets_.end() NE it) {
        // @info foramt: key=value&key=value&...& // always end with '&'
#pragma BUILD_MSG("TODO: decide how to use?");
        return it->second;
      }
      else {
        return string();
      }
    }
    std::mutex door_;
    std::map<const Sock, string> sockets_;
  };




  //
  // ASocket
  // 
#define IS_SERVER SockRunType::kClientTcp > runtype_
  ASocket::ASocket(const SockRunType _runtype)
    : sockp_(nullptr), runtype_(_runtype), clients_(nullptr) {
    if (IS_SERVER) {
      clients_ = new ClientList;
    }
  }
  ASharedPtr<ASocket> ASocket::Create(const SockRunType _runtype) {
    return new ASocket(_runtype);
  }
  ASocket::~ASocket() { Release(); }
  const SockRes ASocket::Release() {
    if (IS_SERVER) {
      clients_->sockets_.clear();
      SAFE_DELETE(clients_);
    }
    CloseSocket();
    SAFE_DELETE(sockp_);
    return SockRes::kSucceeded;
  }
  const SockRes ASocket::CleanupEnvironment()
  {
#if defined(_LINUX)
#elif defined(_WINDOWS)
    WSACleanup();
#endif
    return SockRes::kSucceeded;
  }
  const SockRes ASocket::SetupEnvironment()
  {
#if defined(_LINUX)
    // ������ ȯ�濡�� �¾�
#elif defined(_WINDOWS)
    WSADATA wsa_data;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
      return SockRes::kSetupFailed;
    }
#endif
    return SockRes::kSucceeded;
  }

  const SockRunType& ASocket::GetRunType() {
    return runtype_;
  }
  const SockRes ASocket::CreateSocket(
    const ProtocolFamily _pf, const SockType _sock_type, const Protocol _protocol) {

    // create socket
    Sock sock{ socket(_pf, _sock_type, _protocol) };
    if (kInvalidSock EQ sock) {
      _TRACE("Invalid socket.");
      return SockRes::kCreateSocketFalied;
    }
    // set socket attr
    switch (_pf) {
    case AF_INET6/*PF_INET6*/:
    {
      sockp_ = new ASa6(_pf, _sock_type, sock);
      auto opt_res = SetSockOption(IPPROTO_IPV6, IPV6_V6ONLY, (void*)1/*SET*/);
      if (SockRes::kSucceeded != opt_res) { _TRACE("IPV6_V6ONLY ERR"); return SockRes::kSetSocketOptionFailed; }
    } break;
    case AF_INET/*PF_INET*/:
      sockp_ = new ASa4(_pf, _sock_type, sock);
      break;
    default:
      _TRACE("Address family NOT SUPPORTED. value: " << _pf);
      break;
    }

    SockRes opt_res = SetSockOption(SOL_SOCKET, SO_RCVBUF, (void*)kDefaultBufSize);
    if (SockRes::kSucceeded != opt_res) { _TRACE("SO_RCVBUF ERR"); return SockRes::kSetSocketOptionFailed; }
    opt_res = SetSockOption(SOL_SOCKET, SO_SNDBUF, (void*)kDefaultBufSize);
    if (SockRes::kSucceeded != opt_res) { _TRACE("SO_SNDBUF ERR"); return SockRes::kSetSocketOptionFailed; }

    return SockRes::kSucceeded;
  }

  const SockRes ASocket::CreateSocket(const ProtocolFamily _pf /*= PF_INET*/) {
    Sock sock{kInvalidSock};
    SockType sock_type{};
    switch (runtype_) {
    case SockRunType::kServerTcp:
    case SockRunType::kClientTcp:
    case SockRunType::kServerTcpMultiplexor:
      sock_type = SOCK_STREAM;
      sock = socket(_pf, sock_type, IPPROTO_TCP);
      break;
#if defined(_LINUX)
    case kServerTcpEpoll:
      sock = socket(_pf, SOCK_STREAM, IPPROTO_TCP);
      break;
#elif defined(_WINDOWS)
    case SockRunType::kServerTcpAsync:
      break;
    case SockRunType::kServerTcpIocp:
      sock_type = SOCK_STREAM;
      sock = WSASocket(_pf, sock_type, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
      break;
#endif
    case SockRunType::kServerUdp:
    case SockRunType::kClientUdp:
    case SockRunType::kServerUdpBroadcastor:
    case SockRunType::kSenderUdpMulticastor:
    case SockRunType::kReceiverUdpMulticastor:
      sock_type = SOCK_DGRAM;
      sock = socket(_pf, sock_type, IPPROTO_UDP);
      break;
    case SockRunType::kUnknown:
    default:
      _TRACE("Invalid SockRunType.");
      break;
    }
    if (kInvalidSock EQ sock) {
      _TRACE("Invalid socket.");
      return SockRes::kCreateSocketFalied;
    }

    // set socket attr
    switch (_pf) {
    case AF_INET6/*PF_INET6*/:
    {
      sockp_ = new ASa6(_pf, sock_type, sock);
      auto opt_res = SetSockOption(IPPROTO_IPV6, IPV6_V6ONLY, (void*)1/*SET*/);
      if (SockRes::kSucceeded != opt_res) { _TRACE("IPV6_V6ONLY ERR"); return SockRes::kSetSocketOptionFailed; }
    } break;
    case AF_INET/*PF_INET*/:
      sockp_ = new ASa4(_pf, sock_type, sock);
      break;
    default:
      _TRACE("Address family NOT SUPPORTED. value: " << _pf);
      break;
    }

    SockRes opt_res = SetSockOption(SOL_SOCKET, SO_RCVBUF, (void*)kDefaultBufSize);
    if (SockRes::kSucceeded != opt_res) { _TRACE("SO_RCVBUF ERR"); return SockRes::kSetSocketOptionFailed; }
    opt_res = SetSockOption(SOL_SOCKET, SO_SNDBUF, (void*)kDefaultBufSize);
    if (SockRes::kSucceeded != opt_res) { _TRACE("SO_SNDBUF ERR"); return SockRes::kSetSocketOptionFailed; }

    return SockRes::kSucceeded;
  }
  const SockRes ASocket::SetSockOption(
    const SockLevel _level, const SockOption _sock_option, void* _option_value) {
    Int is_set_failed{ -1 };
    const Sock& s = sockp_->s_;
    if (SOL_SOCKET EQ _level) {
      switch (_sock_option) {
      case SO_BROADCAST:
      case SO_DEBUG:
      case SO_DONTLINGER:
      case SO_DONTROUTE:
      case SO_OOBINLINE:
      case SO_KEEPALIVE:
      case SO_REUSEADDR:
      {
        BOOL opt_val{ (BOOL)_option_value };
        is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(&opt_val), sizeof(BOOL));
      }
      break;
      case SO_LINGER:
        /*is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(struct LINGER*)(_option_value), sizeof(LINGER));*/
        break;
      case SO_RCVBUF:
      case SO_SNDBUF:
      {
        socklen_t opt_val{ (socklen_t)_option_value };
        is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(&opt_val), sizeof(socklen_t));
      }
      break;
      case SO_RCVTIMEO:
      case SO_SNDTIMEO:
      {
        socklen_t opt_val{ (socklen_t)_option_value };
        is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(&opt_val), sizeof(socklen_t));
      }
      break;
      default:
        return SockRes::kSetSocketOptionFailed;
      }
    }
    else if (IPPROTO_IP EQ _level) {
      switch (_sock_option) {
      case IP_MULTICAST_LOOP:
      case IP_MULTICAST_TTL:
      {
        socklen_t opt_val{ (socklen_t)_option_value };
        is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(&opt_val), sizeof(socklen_t));
        break;
      }
      case IP_ADD_MEMBERSHIP:
      {
        struct ip_mreq* join_addr{ (struct ip_mreq*)_option_value };
        is_set_failed =
          setsockopt(s, _level, _sock_option, (const char*)(join_addr), sizeof(struct ip_mreq));
      }
      break;
      }
    }
    else if (IPPROTO_TCP EQ _level) {
      switch (_sock_option) {
      case TCP_NODELAY: //nagle algorithm
        break;
      default:
        return SockRes::kSetSocketOptionFailed;
      }
    }
    else if (IPPROTO_IPV6 EQ _level) {
      switch (_sock_option) {
      case IPV6_V6ONLY:
        int opt_val{ (int)_option_value };
        is_set_failed = setsockopt(s, _level, _sock_option, (const char*)&opt_val, sizeof(opt_val));
        break;
      }
    }

    if (is_set_failed) {
      // TODO(hataeung2@gmail.com): error log into file
      _TRACE("Set socket option failed.");
      return SockRes::kSetSocketOptionFailed;
    }
    return SockRes::kSucceeded;
  }

  const SockRes ASocket::SetNonBlockingMode(const Bool _mode /*= true*/) {
    Bool result{ false };
    Ulong mode{ 0 };
#if defined(_LINUX)
    fcntl();
#elif defined(_WINDOWS)
    if (_mode) { mode = 1; }  // If (0 != mode), non-blocking mode is enabled.
    if (SOCKET_ERROR NE ioctlsocket((SOCKET)sockp_->s_, FIONBIO, &mode)) { result = true; }
#endif
    if (!result) { return SockRes::kSwitchBlockingModeFailed; }
    else { return SockRes::kSucceeded; }
  }
  const SockRes ASocket::Shutdown(const SockShutdown _sd /*= SockShutdown::kRecv*/) {
    if (ZERO EQ shutdown(sockp_->s_, _sd)) {
      return SockRes::kSucceeded;
    } else {
      switch (errno) {
      case EBADF:
        return SockRes::kNoSockExist;
      case ENOTSOCK:
        return SockRes::kNotaSock;
      case ENOTCONN:
        return SockRes::kNotConnected;
      case EINVAL:
        return SockRes::kInvalidParam;
      }
    }
  }
  const SockRes ASocket::CloseSocket()
  {
    if (nullptr EQ sockp_) { 
      _TRACE("NO socket property exist.");
      return SockRes::kCloseSocketFailed;
    }
    Int err{ 0 };
#if defined(_LINUX)
    close(attr_->sock_);

#elif defined(_WINDOWS)
    if (sockp_->s_) {
      WaitForSingleObject((HANDLE)sockp_->s_, kDefaultTimeout);
      err = closesocket(sockp_->s_);
      if (SOCKET_ERROR EQ err) {
        err = WSAGetLastError();
        _TRACE("Socket closing err. WSAGetLastError(): " << err);
        return SockRes::kCloseSocketFailed;
      }
    }
#endif
    _TRACE("Socket closing. " << (sockp_->GetInfoStr()));
    return SockRes::kSucceeded;
  }

  const Sock ASocket::GetSocket() {
    return sockp_->s_;
  }

  const Int ASocket::Recv(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags) {
    return recv(_s, (char*)_buf, (int)_buf_size, _flags);
  }

  const Int ASocket::Send(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags) {
    return send(_s, (const char*)_buf, (int)_buf_size, _flags);
  }

  const Int ASocket::RecvFrom(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags, SockAddr* _from) {
    return recvfrom(_s, (char*)_buf, _buf_size, _flags, (struct sockaddr*)_from, nullptr);
  }

  const Int ASocket::SendTo(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags, const SockAddr* _to) {
    return sendto(_s, (char*)_buf, _buf_size, _flags, (struct sockaddr*)_to, sizeof(struct sockaddr));
  }


  const SockRes ASocket::ConnectTo(
    const char* const _host, const char* const _port) {
    const int connect_failed{ -1 };

    // set server attr
    SockProp* server_sa{ nullptr };
    switch (sockp_->af_) {
    case AF_INET6/*PF_INET6*/: {
      auto server_sa6 = new ASa6(sockp_->af_, sockp_->type_, sockp_->s_);
      server_sa6->SetAddr(_host, _port);
      server_sa = server_sa6;
    } break;
    case AF_INET/*PF_INET*/: {
      auto server_sa4 = new ASa4(sockp_->af_, sockp_->type_, sockp_->s_);
      server_sa4->SetAddr(_host, _port);
      server_sa = server_sa4;
    } break;
    default:
      _TRACE("Address family NOT SUPPORTED. value: " << sockp_->af_);
      break;
    }
    SockRes res{ SockRes::kConnectFailed };
    // connect
    if (connect_failed == connect(sockp_->s_,
      server_sa->GetSockAddr(), server_sa->GetSockAddrSize())) {
      _TRACE("connection failed");
      res = SockRes::kConnectFailed;
    } else {
      res = SockRes::kSucceeded;
    }
    SAFE_DELETE(server_sa);
    return res;
  }

#define BIND(it) if (bind_failed == bind(it->s_, it->GetSockAddr(), it->GetSockAddrSize())) { return SockRes::kBindFailed; } 
  const SockRes ASocket::BindSocket(
    const char* const _port /*= "8888"*/) {
    const int bind_failed{ -1 };
    switch (sockp_->af_) {
    case AF_INET: {
      ASa4* prop = dynamic_cast<ASa4*>(sockp_);
      prop->SetAddr(INADDR_ANY/*in4addr_any*/, (Ushort)atoi(_port));
      BIND(prop);
    } break;
    case AF_INET6: {
      ASa6* prop = dynamic_cast<ASa6*>(sockp_);
#pragma BUILD_MSG("IP6?");
      prop->SetAddr(nullptr, _port);
      BIND(prop);
    } break;
    default:
      break;
    }
    return SockRes::kSucceeded;
  }
  const SockRes ASocket::BindSocketTo(
    const char* const _host /*= "127.0.0.1"*/,
    const char* const _port /*= "8888"*/) {
    const int bind_failed{ -1 };
    switch (sockp_->af_) {
    case AF_INET: {
      ASa4* prop = dynamic_cast<ASa4*>(sockp_);
      prop->SetAddr(_host, _port);
      BIND(prop);
    } break;
    case AF_INET6: {
      ASa6* prop = dynamic_cast<ASa6*>(sockp_);
      prop->SetAddr(_host, _port);
      BIND(prop);
    } break;
    default:
      break;
    }
    return SockRes::kSucceeded;
  }


  const SockRes ASocket::StartListening(const Int _max_connection /*= kMaxConnection*/) {
    const int listen_failed{ -1 };
    if (listen_failed == listen(sockp_->s_, _max_connection)) {
      return SockRes::kListenFailed;
    }
    return SockRes::kSucceeded;
  }
  const Sock ASocket::Accept(const string& _client_add_info /*= string()*/) {
#pragma BUILD_MSG("client's sockaddr?");
    Int addr_size{ sizeof(SockAddrIn4) };
    SockAddrIn4 client_addr{};
    Sock s{ accept(sockp_->s_, (struct sockaddr*)&client_addr, &addr_size) };
    if (kInvalidSock NE s) {
      clients_->Insert(s, _client_add_info);
    }
    return s;
  }
  const SockRes ASocket::CloseConnectedSocketOf(const Sock _s /*= ALL*/) {
    Bool res{ true };
    if (ALL EQ _s) {
      res = clients_->ResetContainer();
    } else {
      res = clients_->Remove(_s);
    }
    return (true EQ res) ? kSucceeded : kCloseConnectedSocketFailed;
  }
  
  const astring ASocket::ListClients() {
    astring list{};
    list += L"[";
    for each (auto c in clients_->sockets_) {
      list += L"{'socket':";
      list += std::to_wstring(c.first);
      list += L",'addinfo':'" + adata::WcsFromStr(c.second) + L"'";
      list += L"},";
    }
    list.at(list.size() - 1) = ']';
    _TRACE("clients: " << list);
    return list;
  }
  Void ASocket::Broadcast(const BytePtr& _msg, const Uint _len) {
    for each (auto c in clients_->sockets_) {
      send(c.first, (const char*)_msg, _len, NULL);
    }
  }
  

}//! namespace atug2


