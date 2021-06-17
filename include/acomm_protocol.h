#ifndef _ATUG2_COMM_PROTOCOL_H_
#define _ATUG2_COMM_PROTOCOL_H_
#include "adata.h"
#include "acrc.h"
#include "asocket_comm_routine.h"
#include "aexception.h"
#include <array>

// atug2 protocol base
namespace atug2 {
  using std::string;
  using std::array;
  using namespace adata;

  // opcodes
  enum OpCode : Ushort {
    kNak = 0x0000,
    kAck = 0x0001,
    kEcho = 0x0002,

    // 1 : 1
    kPlainMsg = 0x1101,
    kUrgentMsg = 0x1102,
    kPlainReq = 0x1104,
    kUrgentReq = 0x1108,

    // 1 : n
    kBroadcast = 0x1F01,
    kToClientsGroup = 0x1F02,

    // socket closing
    kShutdownSendReq = 0xFFF1,
    kShutdownRecvReq = 0xFFF2,
    kShutdownBothReq = 0xFFF4,
  };

  // defines
  #define STX 0x02
  #define ETX 0x03
  const Ushort kHeadLen{ 9 };
  const Ushort kTailLen{ 3 };
  const Ushort kWrapperLen{ kHeadLen + kTailLen };
  typedef array<Byte, kMaxPacketSize> ByteArr;
  const Ushort kServerId{ 0x0000 };


  /** 
  * @brief Communication Protocol class
  * @details 
  * @author Taeung, Ha <hataeung2@hotmail.com>
  * @date June, 2021
  * @version 1
  */
  class ATUG2_API CommProtocol {
  public:
    /**
    * @brief Assemble one packet for data transaction
    * @details one packet contains opcode, source, destination, content, length and wrapper data(STX, ETX, CRC)
    * @param _opcode: Operation Code to identify what to do
    * @param _src: communication node from
    * @param _dest: communication node to
    * @param _content: data to transaction
    * @param _len: data byte length of _content
    * @return Assembled data packet
    */
    static ADataBucket Assemble(const Ushort _opcode, const Ushort _src, const Ushort _dest,
      const BytePtr _content, const SizeT _len) {
      
      ByteArr assembled{};
      assembled[0] = STX;
      assembled[1] = (_len >> (8 * 1)) & 0xFF;
      assembled[2] = (_len >> (8 * 0)) & 0xFF;
      assembled[3] = (_opcode >> (8 * 1)) & 0xFF;
      assembled[4] = (_opcode >> (8 * 0)) & 0xFF;
      assembled[5] = (_src >> (8 * 1)) & 0xFF;
      assembled[6] = (_src >> (8 * 0)) & 0xFF;
      assembled[7] = (_dest >> (8 * 1)) & 0xFF;
      assembled[8] = (_dest >> (8 * 0)) & 0xFF;
      memcpy_s(&assembled[9], assembled.size() - 9, _content, _len);
      auto ptr{ kHeadLen + _len };
      const unsigned short crc16 = Crc::GetCrc16xmodem(assembled.data(), ptr);
      assembled[ptr] = (crc16 >> (8 * 1)) & 0xFF;
      assembled[++ptr] = (crc16 >> (8 * 0)) & 0xFF;
      assembled[++ptr] = ETX;

      return ADataBucket(assembled.data(), kHeadLen + _len + kTailLen);
    }
    static Void Disassemble(const BytePtr _assembled,
      Ushort& _opcode, Ushort& _src, Ushort& _dest, BytePtr& _cont_start, SizeT& _len) {
      _len =    (_assembled[1] << (8 * 1)) | (_assembled[2] << (8 * 0));
      _opcode = (_assembled[3] << (8 * 1)) | (_assembled[4] << (8 * 0));
      _src =    (_assembled[5] << (8 * 1)) | (_assembled[6] << (8 * 0));
      _dest =   (_assembled[7] << (8 * 1)) | (_assembled[8] << (8 * 0));
      _cont_start = &_assembled[kHeadLen];

      auto stx = _assembled[0];
      auto etx = _assembled[kHeadLen + _len + kTailLen - 1];

      const auto crc16recvd{ _assembled[kHeadLen + _len] << (8 * 1) | _assembled[kHeadLen + _len + 1] << (8 * 0) };
      const Ushort validation_crc16{ Crc::GetCrc16xmodem(_assembled, kHeadLen + _len) };
      if (crc16recvd NE validation_crc16) {
        throw AException("CRC Value Invalid");
      }
    }

    static const Bool Attach(ADataBucket& _serialized, const BytePtr _data, const SizeT& _len) {
      return _serialized.Attach(_data, _len);
    }
    static const Bool ExtractOneValidPacket(ADataBucket& _extracted, ADataBucket& _serialized) {
      const SizeT size{ _serialized.UsedLength() };
      if (kWrapperLen > size) { return false; }

      const BytePtr data{ _serialized.Get() };
      for (SizeT pos = ZERO; size > pos; ++pos) {
        if (STX EQ data[pos]) {
          const SizeT stxpos{ pos };
          const auto datalen{ (data[1] << (8 * 1)) | (data[2] << (8 * 0)) };
          const auto packetsize{ kHeadLen + datalen + kTailLen };
          if (ETX EQ data[packetsize - 1]) {
            _extracted.Fill(data, packetsize);
            _serialized << packetsize;
#if defined(_DEBUG)
            ByteArr dbgserial{ 0, }, dbgextract{ 0, };
            memcpy_s(dbgserial.data(), kMaxPacketSize, _serialized.Get(), kMaxPacketSize);
            memcpy_s(dbgextract.data(), kMaxPacketSize, _extracted.Get(), _extracted.UsedLength());
#endif
            return true;
          } else {
            continue; // ignore this packet
          }
        }
      }//!for
    }

  }; //!CommProtocol


}//!namespace atug2 {


#endif//!_ATUG2_COMM_PROTOCOL_H_