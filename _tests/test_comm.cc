#include "gtest/gtest.h"
#include "acomm_protocol.h"

using namespace atug2;
TEST(CommProtocolTest, packet_processing) {
  const Ushort src_id{ 0 };
  const Ushort dest_id{ 1 };

  // assemble
  string plainmsg{ "just a message" };
  auto assembled = CommProtocol::Assemble(OpCode::kPlainMsg,
    src_id, dest_id, (BytePtr)plainmsg.c_str(), plainmsg.length());

  // data continued
  ADataBucket serialized(kMaxPacketSize);
  ADataBucket extracted(ZERO);
  Bool succeed{ false };
  succeed = CommProtocol::Attach(serialized, assembled.Get(), 7);
  ASSERT_EQ(succeed, true);
  succeed = CommProtocol::ExtractOneValidPacket(extracted, serialized);
  ASSERT_EQ(succeed, false) << "no normal packet exist";;
  succeed = CommProtocol::Attach(serialized, &assembled.Get()[7], assembled.UsedLength()-7);
  ASSERT_EQ(succeed, true) << "1 normal packet made";
  succeed = CommProtocol::Attach(serialized, assembled.Get(), assembled.UsedLength());
  ASSERT_EQ(succeed, true) << "1 normal packet added";
  succeed = CommProtocol::ExtractOneValidPacket(extracted, serialized);
  ASSERT_EQ(succeed, true) << "1 packet extracted (1 remaining)";
  EXPECT_EQ(memcmp(assembled.Get(), extracted.Get(), extracted.Size()), ZERO);
  EXPECT_EQ(memcmp(assembled.Get(), serialized.Get(), serialized.UsedLength()), ZERO);
  succeed = CommProtocol::ExtractOneValidPacket(extracted, serialized);
  ASSERT_EQ(succeed, true) << "1 packet extracted (0 remaining)";
  EXPECT_EQ(memcmp(assembled.Get(), extracted.Get(), extracted.Size()), ZERO);
  EXPECT_EQ(serialized.UsedLength(), ZERO);
  
  // disassemble
  Ushort opcode{ ZERO };
  Ushort src{ ZERO };
  Ushort dest{ ZERO };
  BytePtr cont_ptr{ nullptr };
  SizeT cont_len{ ZERO };
  CommProtocol::Disassemble(extracted.Get(), opcode, src, dest, cont_ptr, cont_len);
  EXPECT_EQ(opcode, OpCode::kPlainMsg);
  EXPECT_EQ(src, 0);
  EXPECT_EQ(dest, 1);
  EXPECT_EQ(cont_ptr[0], plainmsg[0]);
  EXPECT_EQ(memcmp(cont_ptr, plainmsg.c_str(), cont_len), 0);
  EXPECT_EQ(cont_len, plainmsg.length());
}