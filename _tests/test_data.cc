#include "gtest/gtest.h"
#include "adata.h"
#include "amemory.h"
using namespace atug2;
using namespace atug2::adata;
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
 TEST(AMemoryUsage, defaulttype_alloc_free_on_AManagedHeap) {
   // byte array assign
   BytePtr byteArr = anew(Byte, 100);
   EXPECT_EQ(AManagedHeap::IsOnHeap(byteArr), true) << "byteArr should exist";
  
   // int value assign
   Int* intVal = anew(Int, 1);
   EXPECT_EQ(AManagedHeap::IsOnHeap(intVal), true) << "intVal should exist";

   // assigned count check
   EXPECT_EQ(AManagedHeap::Count(), 2) << "2 dynamic memory should exist";

   // release 
   adelete(byteArr);
   adelete(intVal);

   // empty check
   EXPECT_EQ(AManagedHeap::IsEmpty(), true) << "Empty:" << AManagedHeap::IsEmpty();
 }

 TEST(AMemoryUsage, userdefinedtype_alloc_free_on_AManagedHeap) {
   adata::ADataBucket d1;
   //adata::ADataBucket* d{ new adata::ADataBucket }; // compile error

   // allocation check  
   char data_from_outside[] {"this is a data from outside not allocated"};
   d1.Fill((BytePtr)(data_from_outside), strlen(data_from_outside));
   EXPECT_EQ(strcmp((char*)d1.Get(), data_from_outside), 0) << "should be same";

   adata::ADataBucket d2;
   //d1 = d2; // compile error
  
   // d2 is empty
   d2.Replace(d1);

   // now d2 is filled with d1's data
   EXPECT_EQ(strcmp((char*)d2.Get(), data_from_outside), 0) << "should be same";

   // now d1 is emtpy
   EXPECT_EQ(d1.Get(), nullptr) << "should be empty";

   char data_newly_created[]{ "new data" };
   auto data_newly_created_size{ strlen(data_newly_created) };
   // just prepare memory block. size check
   d1.Prepare(data_newly_created_size);
   EXPECT_EQ(d1.Size(), data_newly_created_size) << "should be 8";
  
   // memory copy from outside
   memcpy_s(d1.Get(), data_newly_created_size, data_newly_created, data_newly_created_size);
   EXPECT_EQ(strcmp((char*)d1.Get(), data_newly_created), 0) << "should be same";

   adata::ADataBucket d3;
   d3.Fill((BytePtr)data_newly_created, data_newly_created_size);
   EXPECT_EQ(strcmp((char*)d3.Get(), data_newly_created), 0) << "should be same";
   EXPECT_EQ(d3.Size(), data_newly_created_size) << "should be 8";
 }

 TEST(DataManipulation, converters) {
   Number num = 15;
   const string utf8_str = u8"u8 string here 유팔";
   const string ansi_str = "ansi string here KOREAN can't be converted";
   const astring unicode_str = L"unicode_string here 유니코드";

   string num_str = ToStr(num);
   EXPECT_EQ(num_str, "15");
   astring num_wstr = ToWstr(num);
   EXPECT_EQ(num_wstr, L"15");
   astring converted_from_u8 = UnicodeFromUtf8(utf8_str);
   EXPECT_EQ(converted_from_u8, L"u8 string here 유팔");
   astring converted_from_ansi = WcsFromStr(ansi_str);
   EXPECT_EQ(converted_from_ansi, L"ansi string here KOREAN can't be converted");

   num = ToNumber("15", 10);
   EXPECT_EQ(num, 15);
   num = ToNumber("F", 16);
   EXPECT_EQ(num, 15);
   num = ToNumber("17", 8);
   EXPECT_EQ(num, 15);
   num = ToNumber("1111", 2);
   EXPECT_EQ(num, 15);

   astring trimer = astr("  trim check    ");
   auto trimedA = Trim(trimer);
   auto trimedL = Ltrim(trimer);
   auto trimedR = Rtrim(trimer);
   EXPECT_EQ(trimedA, L"trim check");
   EXPECT_EQ(trimedL, L"trim check    ");
   EXPECT_EQ(trimedR, L"  trim check");

   Replace(trimer, L"trim", L"TRIM");
   EXPECT_EQ(trimer, L"  TRIM check    ");
 }

 TEST(DataConvert, encoding) {
   // empty one
   adata::AStringContainer s1; 

   // set ansi
   s1.Set("ansi string", adata::CharEnc::kAnsi);
   string ansi_str = s1.GetStr(adata::CharEnc::kAnsi);
   EXPECT_EQ(ansi_str, ("ansi string"));

   // set utf8
   s1.Set(u8str("hi utf8 string"));
   string u8_str = s1.GetStr();
   EXPECT_EQ(u8_str, u8str("hi utf8 string"));

   // set unicode
   s1.Set(astr("hello unicode string"));
   astring unicode_str = s1.GetWstr();
   EXPECT_EQ(unicode_str, astr("hello unicode string"));

   // GetByte()
   BytePtr ansi_bytes = s1.GetByte(adata::CharEnc::kAnsi);
   BytePtr utf8_bytes = s1.GetByte(adata::CharEnc::kUtf8);
   BytePtr unicode_bytes = s1.GetByte(adata::CharEnc::kUnicode);
   EXPECT_EQ(string((char*)ansi_bytes), ansi_str);
   EXPECT_EQ(string((char*)utf8_bytes), u8_str);
   EXPECT_EQ(astring((wchar_t*)unicode_bytes), unicode_str);

   // GetByteLen()
   const SizeT ansi_len = s1.GetByteLen(adata::CharEnc::kAnsi);
   const SizeT utf8_len = s1.GetByteLen(adata::CharEnc::kUtf8);
   const SizeT unicode_len = s1.GetByteLen(adata::CharEnc::kUnicode);
   EXPECT_EQ(ansi_len, strnlen_s((char*)ansi_bytes, 256));
   EXPECT_EQ(utf8_len, strnlen_s((char*)utf8_bytes, 256));
   EXPECT_EQ(unicode_len/2, wcsnlen_s((wchar_t*)unicode_bytes, 256));

   // Encode() from ansi
   adata::AStringContainer encode("FromAnsi 한글?", adata::CharEnc::kAnsi);
   encode.Encode(adata::CharEnc::kUtf8);
   string u8_from_ansi = encode.GetStr(adata::CharEnc::kUtf8);
   encode.Encode(adata::CharEnc::kUnicode);
   astring unicode_from_ansi = encode.GetWstr();
   EXPECT_EQ(u8_from_ansi, u8"FromAnsi 한글?");
   EXPECT_EQ(unicode_from_ansi, L"FromAnsi 한글?");
  
   // Encode() from utf8
   encode.Clear();
   encode.Set(u8str("FromUtf8입니다"), adata::CharEnc::kUtf8);
   encode.Encode(adata::CharEnc::kUnicode);
   astring unicode_from_u8 = encode.GetWstr();
   encode.Encode(adata::CharEnc::kAnsi);
   string ansi_from_u8 = encode.GetStr(adata::CharEnc::kAnsi);
   EXPECT_EQ(ansi_from_u8, "FromUtf8입니다");
   EXPECT_EQ(unicode_from_u8, astr("FromUtf8입니다"));

   // Encode() from unicode
   encode.Clear();
   encode.Set(astr("FromUnicode입니다"));
   encode.Encode(adata::CharEnc::kAnsi);
   string ansi_from_unicode = encode.GetStr(adata::CharEnc::kAnsi);
   encode.Encode(adata::CharEnc::kUtf8);
   string u8_from_unicode = encode.GetStr(adata::CharEnc::kUtf8);
   EXPECT_EQ(ansi_from_unicode, "FromUnicode입니다");
   EXPECT_EQ(u8_from_unicode, u8str("FromUnicode입니다"));

   encode.Clear();
 }

TEST(AStringContainerTest, reference_count) {
  AStringContainer s1("s1");
  AStringContainer s2(s1);
  _TRACE("s1: " << s1.RefCnt() << "s2: " << s2.RefCnt());
  EXPECT_EQ(s1.RefCnt(), s2.RefCnt());
}

