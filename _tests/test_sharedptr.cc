#include "gtest/gtest.h"
#include "ashared_ptr.h"
#include "asocket_comm_routine.h"
#include "aroutine_factory.h"
#include "aexception.h"
#include "acomm_react.h"
//TEST(SharedPtrTest, shared_ptr_create_refcount_check) {
//#define AOBJ_SPTR ASharedPtr<AObj>
//  AOBJ_SPTR ptr1{ AObj::Create() };
//  EXPECT_EQ(1, ptr1.GetRef()) << "ptr1 ref-count should be 1";
//  AOBJ_SPTR ptr2 = ptr1;
//  EXPECT_EQ(2, ptr1.GetRef()) << "ptr1 ref-count should be 2";
//  EXPECT_EQ(2, ptr2.GetRef()) << "ptr2 ref-count should be 2";
//
//  AOBJ_SPTR ptr3(AObj::Create()/*이건 사라짐*/);
//  AOBJ_SPTR ptr4(AObj::Create());
//  EXPECT_EQ(1, ptr3.GetRef()) << "ptr3 ref-count should be 1";
//  ptr3 = ptr4; // ptr3이 가진 메모리 삭제하고 ptr4를 ptr3에 할당
//  EXPECT_EQ(2, ptr3.GetRef()) << "ptr3 ref-count should be 2";
//  EXPECT_EQ(2, ptr4.GetRef()) << "ptr4 ref-count should be 2";
//  //AObj* obj2{ new AObj };
//  //ptr3 = obj2;
//  //DBG(ptr3.GetRef() << "ea");
//  //EXPECT_EQ(ptr3.GetRef(), 1);
//}

TEST(SharedPtrTest, addr_that_is_not_pointer_check) {
  //AServerTcpMultiplexorRoutine r;
  //// @runtime error : AException
  //ASharedPtr<ARoutine> ptr_local_addr1 = &r;
  //ASharedPtr<ARoutine> ptr_local_addr2(&r);

  // @normal usage
  try {
    ARoutine* r = ARoutineFactory::NewServerTcpMultiplex(nullptr, new ReactorServerSample);
    // OK
    ASharedPtr<ARoutine> ptr_empty1 = ASharedPtr<ARoutine>(r);
    ASharedPtr<ARoutine> ptr_empty2 = r;
  } catch (AException& e) {
    
  }
  
  // ref count test
  ASharedPtr<ARoutine> ptr_heap_addr1(ARoutineFactory::NewServerTcpMultiplex(nullptr, new ReactorServerSample));
  ASharedPtr<ARoutine> ptr_heap_addr2 = ptr_heap_addr1;
  EXPECT_EQ(ptr_heap_addr1.GetRef(), 2);
  EXPECT_EQ(ptr_heap_addr2.GetRef(), 2);
  ASharedPtr<ARoutine> ptr_heap_addr3(ARoutineFactory::NewServerTcpMultiplex(nullptr, new ReactorServerSample));
  ptr_heap_addr2 = ptr_heap_addr3;
  EXPECT_EQ(ptr_heap_addr2.GetRef(), 2);
  EXPECT_EQ(ptr_heap_addr1.GetRef(), 1);
}