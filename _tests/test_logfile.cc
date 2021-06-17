#include "gtest/gtest.h"
#include "afile.h"
#include "alog.h"

 TEST(FilePtrTest, create_setptrpos_with_trunc) {
   _mkdir("log-atug-libraries-test");
   auto dir = GetCurrentDir() + L"/log-atug-libraries-test";

   // file create
   ASharedPtr<AFile> f{ AFile::Create(L"test-set-ptr-pos",L"txt", dir.c_str(), FileOpenType::kOpenAlways) };
   ASSERT_EQ(true, f->Open()) << "File must be opened with option of kOpenAlways";
  
   // get fileptr pos
   FilePtrPos pos = f->GetFilePtrPos();
   _TRACE("fileptr pos: " << pos << " when opened");

   // pos to beginning + 1
   pos = f->SetFilePtrPos(1, FilePtrStd::kBegin);
   _TRACE("fileptr pos: " << pos << " begin+1");
   // pos to end - 15
   pos = f->SetFilePtrPos(-15, FilePtrStd::kEnd);
   _TRACE("fileptr pos: " << pos << " end-15");
  
   // prepare buffer for reading
   ADataBucket read_buffer(256);
   // file content read to the buffer
   int read_len = f->Read(read_buffer);

   // get back to beginning of the file
   pos = f->SetFilePtrPos(-read_len, FilePtrStd::kCurrent);
   _TRACE("fileptr pos: " << pos);
   pos = f->GetFilePtrPos();
   _TRACE("fileptr pos: " << pos);
   int write_len = f->Write(ADataBucket((BytePtr)u8" hi again:)", 12));
   pos = f->GetFilePtrPos();
   pos = f->SetFilePtrPos(-write_len, FilePtrStd::kCurrent);
  
   // pointer go to begin of the file
   pos = f->SetFilePtrPos(-pos, FilePtrStd::kCurrent); 
   _TRACE("fileptr pos: " << pos);
  
   // try to place to ahead of the beginning of the file
   pos = f->SetFilePtrPos(-10, FilePtrStd::kCurrent); 
   _TRACE("fileptr pos: " << pos);
   EXPECT_EQ(pos, -1) << "should be '-1'";

   // go to end of the file
   pos = f->SetFilePtrPos(0, FilePtrStd::kEnd);
   _TRACE("fileptr pos: " << pos);

   // try to place to behind of the end of the file
   pos = f->SetFilePtrPos(10, FilePtrStd::kEnd);
   _TRACE("fileptr pos: " << pos); // just move the cursor to the place by 'SetFilePtrPos'

   f->Close();
 }

TEST(LogTest, create_write_file_1min_u8) {
  auto additional_logtestpath = wstring(L"log-atug-libraries-test") + DIV + L"alog-test-u8-min";
  ASharedPtr<ALog> l{ ALog::Create((additional_logtestpath), TimeUnit::kMin) };
  l->Start();
  l->Push(u8str("log one"));

  int second{ 0 };
  while (60 > ++second) {
    _TRACE(second << " second(s)");
    _asleep(1000);
  }
  //must written in another logfile;
  l->Push("로그투"); // written with EUC-KR like encoding;
  //l->Push(L"로그쓰리"); // will not be written AS UTF-8;
  l->Push(u8"로그포");
  EXPECT_EQ(true, true) << "1 minute unit logging";
  l->Stop();
}

TEST(LogTest, create_write_file_1h_unicode) {
  auto additional_logtestpath = wstring(L"log-atug-libraries-test") + DIV + L"alog-test-unicode";
  ASharedPtr<ALog> l{ ALog::Create((additional_logtestpath), TimeUnit::kHour, CharEnc::kUnicode) };
  l->Start();
  l->Push(astr("log one"));
  int second{ 0 };
  while (10 > ++second) {
    _TRACE(second << " second(s)");
    _asleep(1000);
    l->Push(L"로그"); // will not be written AS UTF-8;
  }
  //must written in another logfile;
  l->Push(L"로그투"); // written with EUC-KR encoding;
  EXPECT_EQ(true, true);
  l->Stop();
}