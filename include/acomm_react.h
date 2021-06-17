#ifndef _ATUG2_COMM_REACT_H_
#define _ATUG2_COMM_REACT_H_
#include "acomm_protocol.h"
#include <array>

namespace atug2 {
  using std::string;
  using std::array;
  using namespace adata;

  /*
  * Reaction(Answer) Generator base class
  */
  class ATUG2_API Reactor : public IHeapTracker {
  public:
    Reactor() {}
    virtual ~Reactor() {}
  public:
    static Void GenReact(const BytePtr _packet, const SizeT _len, SockCommProp* _p, Reactor* _r) {
      assert(nullptr NE _p);
      assert(nullptr NE _r);

      Ushort opcode{ ZERO };
      Ushort src{ ZERO };
      Ushort dest{ ZERO };
      BytePtr cont_start{ nullptr };
      SizeT cont_len{ ZERO };
      CommProtocol::Disassemble(_packet, opcode, src, dest, cont_start, cont_len);
      (*_r)(opcode, src, dest, cont_start, cont_len, _p);
    }
  public:
    virtual Void operator()(const Ushort _opcode,
      const Ushort _src, const Ushort _dest, const BytePtr _content, const SizeT _len, SockCommProp* _p) PURE;
  };

  /**
  * Sample Reactor for Server
  */
  class ATUG2_API ReactorServerSample : public Reactor {
  public:
    ReactorServerSample() {}
    virtual ~ReactorServerSample() {}
  public:
    virtual Void operator()(const Ushort _opcode,
      const Ushort _src, const Ushort _dest, const BytePtr _content, const SizeT _len, SockCommProp* _p) final
    {
      const auto s{ _p->self };
      Bool is_sendbuffer_available{ false };

      const Ushort src{ _dest };
      const Ushort dest{ _src };

      switch (_opcode) {
      case OpCode::kNak: {
        ADataBucket sent(kMaxPacketSize);
        CommProtocol::ExtractOneValidPacket(sent, _p->GetSendBucket());
        s->Send(_p->opponent, sent.Get(), sent.Size(), NULL);
        is_sendbuffer_available = CommProtocol::Attach(_p->GetSendBucket(), sent.Get(), sent.Size());
      } break;

      case OpCode::kAck: {
        CommProtocol::ExtractOneValidPacket(ADataBucket(kMaxPacketSize), _p->GetSendBucket());
      } break;

      case OpCode::kEcho: {
        const ADataBucket sent{ CommProtocol::Assemble(_opcode, src, dest, _content, _len) };
        s->Send(_p->opponent, sent.Get(), sent.Size(), NULL);
      } break;

      case OpCode::kPlainMsg:
      case OpCode::kUrgentMsg:
      case OpCode::kPlainReq:
      case OpCode::kUrgentReq: {
        const ADataBucket sent{ CommProtocol::Assemble(_opcode, src, dest, (BytePtr)"ACK", 3) };
        s->Send(_p->opponent, sent.Get(), sent.Size(), NULL);
        is_sendbuffer_available = CommProtocol::Attach(_p->GetSendBucket(), sent.Get(), sent.Size());
      } break;

      /*case OpCode::kBroadcast:
        break;
      case OpCode::kToClientsGroup:
        break;
      case OpCode::kShutdownSendReq:
        break;
      case OpCode::kShutdownRecvReq:
        break;
      case OpCode::kShutdownBothReq:
        break;*/
      default:
        break;
      }


      if (false EQ is_sendbuffer_available) {
        //s->Send(_p->opponent, )
        s->CloseConnectedSocketOf(_p->opponent);
        _p->GetSendBucket().Prepare(kDefaultBufSize);
      }
    }
  private:

  };

  /**
  * Sample Reactor for Client
  */
  class ATUG2_API ReactorClientSample : public Reactor {
  public:
    ReactorClientSample() {}
    virtual ~ReactorClientSample() {}
  public:
    virtual Void operator()(const Ushort _opcode,
      const Ushort _src, const Ushort _dest, const BytePtr _content, const SizeT _len, SockCommProp* _p) final
    {
    }
  };

}//!namespace atug2 {

#endif//!_ATUG2_COMM_REACT_H_