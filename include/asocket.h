#ifndef _ATUG2_SOCKET_BASE_H_
#define _ATUG2_SOCKET_BASE_H_
#include "atug2.h"
#include "amemory.h"
#include "ashared_ptr.h"

namespace atug2 {
  // os specified
  const Ushort kMaxBufferSize{ 65535 };
  const Ushort kDefaultBufSize{ 1024 };
  const Ushort kMaxPacketSize{ 256 };
#if defined(_WINDOWS)
  typedef SOCKET Sock;
  const unsigned int kDefaultConnCnt{ 65535 };
  const unsigned int kMaxConnection{ SOMAXCONN };
  const unsigned __int64 kInvalidSock{ INVALID_SOCKET };
#elif defined(_LINUX)
  typedef int Sock;
  const unsigned int kInvalidSock{ -1 };
#endif

  // common
  typedef sockaddr SockAddr;
  typedef sockaddr_in SockAddrIn4;
  typedef sockaddr_in6 SockAddrIn6;
  typedef int SockType;
  typedef unsigned short ProtocolFamily, AddrFamily;
  typedef int Protocol, SockLevel, SockOption;
  typedef long AddrHostNumber;
  typedef short AddrPortNumber;
  const int kDefaultTimeout{ 3000 };
  const unsigned int kInfinite{ 0xFFFFFFFF/*INFINITE*/ };
#define ALL kInfinite
 
  enum class SockRunType {
    kUnknown = 0x00,

    kServerTcp = 0x10,
    kServerTcpMultiplexor = 0x11,
    kServerTcpEpoll = 0x12,
    kServerTcpAsync = 0x13,
    kServerTcpIocp = 0x14,

    kServerUdp = 0x20,
    kServerUdpBroadcastor = 0x21,
    kSenderUdpMulticastor = 0x22,
    kReceiverUdpMulticastor = 0x23,
    
    kClientTcp = 0x40,

    kClientUdp = 0x80,
  };

  enum SockRes {
    kSucceeded,
    kCleanupFailed,
    kSetupFailed,
    kCreateSocketFalied,
    kSetSocketOptionFailed,
    kSwitchBlockingModeFailed,
    kBindFailed,
    kListenFailed,
    kConnectFailed,
    kCloseSocketFailed,
    kCloseConnectedSocketFailed,
    
    kNoSockExist,
    kNotaSock,
    kNotConnected,
    kInvalidParam,
    kNoUseFailed = kInvalidSock
  };

  enum SockShutdown {
    kRecv = 0x00,
    kSend = 0x01,
    kBoth = 0x02
  };


    
  class ATUG2_API ASocket : public IHeapTracker {
  public:
    DONT_ALLOW_ACCESS_AS_PTR;
    DONT_ALLOW_COPY_AND_ASSIGN(ASocket);
  protected:
    class SockProp; class ASa4; class ASa6;
    explicit ASocket(const SockRunType _runtype);
  public:
    static ASharedPtr<ASocket> Create(const SockRunType _runtype);
    virtual ~ASocket();
    const SockRes Release();
  public:
    static const SockRes CleanupEnvironment();
    static const SockRes SetupEnvironment();

  public: // run type branch
    const SockRunType& GetRunType();
  public: // open
    /**
    * @see socket create configurations
    * _pf: PF_INET, PF_INET6, PF_LOCAL, PF_PACKET, PF_IPX
    * _sock_type: SOCK_STREAM, SOCK_DGRAM
    * _protocol: IPPROTO_TCP, IPPROTO_UDP
    */
    const SockRes CreateSocket(
      const ProtocolFamily _pf, const SockType _sock_type, const Protocol _protocol);
    const SockRes CreateSocket(const ProtocolFamily _pf = PF_INET);
    const SockRes SetSockOption(const SockLevel _level, const SockOption _sock_option, void* _option_value);
    const SockRes SetNonBlockingMode(const Bool _mode = true);

  public: // close
    const SockRes Shutdown(const SockShutdown _sd = SockShutdown::kRecv);
    const SockRes CloseSocket();
  public: // socket info
    // itself
    const Sock GetSocket();
    const SockAddr* GetSocketAddr();
    // get peer info with socket
    template <typename SA = SockAddrIn4>
    const SA GetSockInfo(const Sock& _s) {
      SA addr{}; Int len{ 0 };
      getpeername(_s, (struct sockaddr*)&addr, &len);
      return addr;
    }

  public: // recv & send
    /**
    * @see socket recv/send options
    * MSG_OOB(send & recv): urgent data (out-of-band)
    * MSG_PEEK(recv): just check if recv buffer has data
    * MSG_DONTROUTE(send): don't use router (local network)
    * MSG_DONTWAIT(send & recv): using Nonblock IO 
    * MSG_WAITALL(recv): wait until requested bytes are fully filled
    */
    const Int Recv(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags);
    const Int Send(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags);
  public: // udp recvfrom & sendto
    const Int RecvFrom(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags, SockAddr* _from);
    const Int SendTo(const Sock _s, BytePtr _buf, const Uint _buf_size, const Int _flags, const SockAddr* _to);

  protected:
    SockProp* sockp_;
    SockRunType runtype_;

    // client
  public:
    const SockRes ConnectTo(const char* const _host, const char* const _port);
    
    // client & server
  public:
    const SockRes BindSocket(
      const char* const _addr_port_string = "8888");
    
    // server
  public:
    const SockRes BindSocketTo(
      const char* const _addr_host_string = "127.0.0.1"/*localhost*/,
      const char* const _addr_port_string = "8888");
  public:
    const SockRes StartListening(const Int _max_connection = kMaxConnection);
    const Sock Accept(const string& _client_add_info = string());
    const SockRes CloseConnectedSocketOf(const Sock _s = ALL);
  public:
    const astring ListClients();
    Void Broadcast(const BytePtr& _msg, const Uint _len);
    
  private:
    // clients management
    struct ClientList;
    ClientList* clients_;
  };

}//!namespace atug2

#endif//!_ATUG2_SOCKET_BASE_H_
