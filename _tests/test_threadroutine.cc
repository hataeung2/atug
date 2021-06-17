#include "gtest/gtest.h"
#include "atime.h"
#include "athread.h"
#include "asocket_comm_routine.h"
#include "aroutine_factory.h"
#include "acomm_protocol.h"
#include "acomm_react.h"
using namespace atug2;



static ADataBucket server_buffer(kDefaultBufSize);
//static ReactorClientSample client_react;

TEST(ThreadTest, create_run_stop_release_with_routine) {
  ASocket::SetupEnvironment();

  // SERVER routine - TCP, Multiplexor
  ASharedPtr<ARoutine> server_r{ ARoutineFactory::NewServerTcpMultiplex(ASOCK_PROC{
    const Sock& opponent{ _p->opponent };
    if (_p->IsDataRecvd()) {
      // data accumulate
      if (false EQ CommProtocol::Attach(_p->GetRecvBucket(), _p->buffer, _p->read_length)) {
        return RoutineAns::kErr;
      }
      // extract
      ADataBucket recvd(kMaxPacketSize);
      while (CommProtocol::ExtractOneValidPacket(recvd, _p->GetRecvBucket())) {
        // generate react
        Reactor::GenReact(recvd.Get(), recvd.Size(), _p, _r);
      }
    }
    return RoutineAns::kOk;
  }, new ReactorServerSample, 2500, "8888", 1000)};
  

  ASharedPtr<AThread> server_t{ AThread::Create(server_r, 100) };
  _TRACE(server_r.GetRef());
  
  server_t->Start();
  _asleep(15 * 1000);
  server_t->Stop();

  ASocket::CleanupEnvironment();
}

//class MultiplexorCommRoutineTest : public ::testing::Test {
//protected:
//  void SetUp() final {
//    server->Run();
//    client->Run();
//  }
//
//  void TearDown() final {
//    client->Stop();
//    server->Stop();
//  }
//  AServerTcpMultiplexorRoutine r;
//  ASharedPtr<AThread> server{ AThread::Create(new AServerTcpMultiplexorRoutine, 1) };
//  ASharedPtr<AThread> client{ AThread::Create(new AClientTcpRoutine, 1000) };
//};
//
//TEST_F(MultiplexorCommRoutineTest, server_open) {
//  
//}
//TEST_F(MultiplexorCommRoutineTest, client_connection) {
//
//}
//TEST_F(MultiplexorCommRoutineTest, data_request_response) {
//
//}
//TEST_F(MultiplexorCommRoutineTest, server_shutdown_when_client_alive) {
//
//}