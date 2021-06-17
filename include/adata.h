// thread safe byte buffer wrapper class
//

#ifndef _ATUG2_DATA_H_
#define _ATUG2_DATA_H_
#include "atug2.h"
#include "amem_macro.h"
#include "amemory.h"
#include <string>
#define astr(str) L##str
#define u8str(str) u8##str

namespace atug2 {
  
  namespace adata {
    enum class FileEnc : char { kUnknown, kAnsi, kUtf8, kUtf8Bom, kU16Le, kU16Be, kU32Le, kU32Be };
    ATUG2_API const FileEnc GetFileEncoding(Str _fullpath);

    ATUG2_API astring ToWstr(const Number _data);
    ATUG2_API string ToStr(const Number _data, const int _radix = 10);
    ATUG2_API const Long ToNumber(const string& _data, const int _radix = 10);
    ATUG2_API const Long ToNumber(const astring& _data, const int _radix = 10);

    /**
    * @brief: "Unicode" does not mean one encoding. this could be utf16, utf32, it depends on the compiler's decision.
    */
    ATUG2_API astring WcsFromStr(const string& _data);
    ATUG2_API string StrFromWcs(const astring& _data);
    ATUG2_API astring UnicodeFromUtf8(const string& _utf8);
    ATUG2_API string Utf8FromUnicode(const astring& _unicode);
    ATUG2_API string AnsiFromUtf8(const string& _utf8);
    ATUG2_API string Utf8FromAnsi(const string& _ansi);

    ATUG2_API wstring Replace(wstring& _target, const wstring& _to_be_replaced, const wstring& _new_one);
    
#define TRIM_SPACE L" \t\n\r\f\v"
    ATUG2_API astring Trim(astring& _s, const astring& _drop = TRIM_SPACE);
    ATUG2_API astring Ltrim(astring& _s, const astring& _drop = TRIM_SPACE);
    ATUG2_API astring Rtrim(astring& _s, const astring& _drop = TRIM_SPACE);


    /**
    * @brief: string container for storing encoding converted
    */
    enum CharEnc : char { kUnknown, kAnsi, kUtf8, kUnicode, kCnt/*=4*/ };
    struct StrProp;
    class ATUG2_API AStringContainer : public atug2::IRefCounter {
      DONT_ALLOW_NEW_DELETE;
      DONT_ALLOW_ACCESS_AS_PTR;
      DONT_ALLOW_ASSIGN(AStringContainer);
    public:
      AStringContainer();
      AStringContainer(const astring& _s);
      /**
      * @brief: make sure using u8"..." for encoding of UTF8
      * @see
      * wchar for L"..."
      * utf8 for u8"..."
      */
      AStringContainer(const string& _s, const CharEnc _enc = CharEnc::kUtf8);
      AStringContainer(const AStringContainer& _copy);
      ~AStringContainer();
    public:
      Wstr GetWstr() const;
      Str GetStr(const CharEnc _enc = CharEnc::kUtf8) const;
      Void Set(const astring& _src);
      Void Set(const string& _src, const CharEnc _enc = CharEnc::kUtf8);
    public:
      const BytePtr GetByte(const CharEnc _enc = CharEnc::kUtf8);
      const SizeT& GetByteLen(const CharEnc _enc = CharEnc::kUtf8) const;
    public:
      Void Encode(const CharEnc _type);
      Void Clear();
    private:
      StrProp* strp_;
    };

    struct DataProp;
    class ATUG2_API ADataBucket : public atug2::IRefCounter {
      DONT_ALLOW_NEW_DELETE;
      DONT_ALLOW_ACCESS_AS_PTR;
      DONT_ALLOW_ASSIGN(ADataBucket)
    public:
      explicit ADataBucket(const SizeT _blocksize = 0);
      explicit ADataBucket(const BytePtr _src, const SizeT _len);
      ADataBucket(const ADataBucket& _copy);
    public:
      ~ADataBucket();
    public:
      const Bool IsEmpty() const;
      const Bool Fill(const BytePtr _src, const SizeT _len);
      const Bool Attach(const BytePtr _src, const SizeT _len);
      const Bool operator << (const SizeT& _to_shift_front);
    public:
      Void Prepare(const SizeT _blocksize);
      const BytePtr& Get() const;
      const SizeT& UsedLength() const;
      const SizeT& Size() const;
    private:
      DataProp* datap_;
    };

  }//!namespace adata
}//!namespace atug2

#endif//!_ATUG2_DATA_H_
