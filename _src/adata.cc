#include "adata.h"
#include "amemory.h"
#include <set>
#include <mutex>
#include <algorithm>
#include <icu.h>
#include <fstream>
#include <vector>
#include <codecvt>
#include <type_traits>

namespace atug2 {
  namespace adata {

    /*
      License
      Copyright(c) 2008 - 2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
      Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
      The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
      
      //utf8 detection from
      //http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    */
#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

    static const uint8_t utf8d[] = {
      // The first part of the table maps bytes to character classes that
      // to reduce the size of the transition table and create bitmasks.
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
      8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
      10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

      // The second part is a transition table that maps a combination
      // of a state of the automaton and a character class to a state.
      0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
      12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
      12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
      12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
      12,36,12,12,12,12,12,12,12,12,12,12,
    };

    uint32_t inline
      DecodeUtf8(uint32_t* state, uint32_t* codep, uint32_t byte) {
      uint32_t type = utf8d[byte];

      *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3fu) | (*codep << 6) :
        (0xff >> type) & (byte);

      *state = utf8d[256 + *state + type];
      return *state;
    }
    uint32_t ValidateUtf8(uint32_t *state, char *str, size_t len) {
      size_t i;
      uint32_t type;
      for (i = 0; i < len; i++) {
        // We don't care about the codepoint, so this is
        // a simplified version of the decode function.
        type = utf8d[(uint8_t)str[i]];
        *state = utf8d[256 + *state + type];
        if (*state == UTF8_REJECT) { break; }
      }
      return *state;
    }
    ATUG2_API const FileEnc GetFileEncoding(Str _fullpath) {
      FileEnc encoding;
      // bom
      // [1] 0xxxxxxx ASCII < 0x80 (128)
      // [2] 110xxxxx 10xxxxxx 2 - byte >= 0x80
      // [3] 1110xxxx 10xxxxxx 10xxxxxx 3 - byte >= 0x400
      // [4] 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 4 - byte >= 0x10000

      // UTF - 8 : (bom[0] & 0xFF) == 0xEF && (bom[1] & 0xFF) == 0xBB && (bom[2] & 0xFF) == 0xBF
      // UTF - 16BE : (bom[0] & 0xFF) == 0xFE && (bom[1] & 0xFF) == 0xFF
      // UTF - 16LE : (bom[0] & 0xFF) == 0xFF && (bom[1] & 0xFF) == 0xFE
      // UTF - 32BE : (bom[0] & 0xFF) == 0x00 && (bom[1] & 0xFF) == 0x00 && (bom[0] & 0xFF) == 0xFE && (bom[1] & 0xFF) == 0xFF
      // UTF - 32LE : (bom[0] & 0xFF) == 0xFF && (bom[1] & 0xFF) == 0xFE && (bom[0] & 0xFF) == 0x00 && (bom[1] & 0xFF) == 0x00

      std::ifstream f(_fullpath, std::ios::in | std::ios::binary);

      char bom[8]{ '\0' };
      if (f.is_open()) {
        f.read(bom, 8);
        if ((bom[0] & 0xFF) EQ 0xEF AND(bom[1] & 0xFF) EQ 0xBB AND(bom[2] & 0xFF) == 0xBF) {
          encoding = FileEnc::kUtf8Bom;
        }
        // UTF16 OR UCS-2
        else if ((bom[0] & 0xFF) EQ 0xFF AND(bom[1] & 0xFF) EQ 0xFE) {
          encoding = FileEnc::kU16Le;
        } else if ((bom[0] & 0xFF) EQ 0xFE AND(bom[1] & 0xFF) EQ 0xFF) {
          encoding = FileEnc::kU16Be;
        }
        //!UTF16 OR UCS-2
        // UTF32 OR UCS-4
        else if ((bom[0] & 0xFF) EQ 0xFF AND(bom[1] & 0xFF) EQ 0xFE AND(bom[0] & 0xFF) EQ 0x00 AND(bom[1] & 0xFF) EQ 0x00) {
          encoding = FileEnc::kU32Le;
        } else if ((bom[0] & 0xFF) EQ 0x00 AND(bom[1] & 0xFF) EQ 0x00 AND(bom[0] & 0xFF) EQ 0xFE AND(bom[1] & 0xFF) EQ 0xFF) {
          encoding = FileEnc::kU32Be;
        }
        //!UTF32 OR UCS-4
        else {
          // have to read whole file content
          f.seekg(-1, std::ios::end);
          size_t file_len = (size_t)f.tellg();
          f.seekg(0, std::ios::beg);
          char* file_cont = new char[file_len] {'\0'};
          f.read(file_cont, file_len);

          uint32_t state{ UTF8_ACCEPT };
          if (UTF8_ACCEPT EQ ValidateUtf8(&state, file_cont, file_len)) {
            encoding = FileEnc::kUtf8;
          } else {
            // check contain nulls
            size_t pos{ 0 };
            bool containing_nulls{ false };
            while (pos < file_len) {
              if (ZERO EQ file_cont[pos++]) { containing_nulls = true; break; }
            }
            if (containing_nulls) {
              encoding = FileEnc::kUnknown;
            }
            else {
              encoding = FileEnc::kAnsi;
            }

          }//!ValidateUtf8
          delete[] file_cont;
          file_cont = nullptr;
        }
      } f.close();
      return encoding;
    }
    //! http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    





    ATUG2_API astring ToWstr(const Number _data) {
      return std::to_wstring(_data);
    }
    ATUG2_API string ToStr(const Number _data, const int _radix /*= 10*/) {
      string str{};
      str = itoa(_data, (char*)str.c_str(), _radix);
      for (auto it = str.begin(); str.end() NE it; ++it) {
        *it = toupper(*it);
      }
      return str;
    }
    ATUG2_API const Long ToNumber(const string& _data, const int _radix /*= 10*/) {
      return strtol(_data.c_str(), NULL/**/, _radix);
    }

    ATUG2_API const Long ToNumber(const astring& _data, const int _radix /*= 10*/) {
      return wcstol(_data.c_str(), NULL/**/, _radix);
    }

#pragma warning(disable: 4244)
    ATUG2_API astring WcsFromStr(const string& _data) {
      astring converted_str;
      converted_str.assign(_data.begin(), _data.end());
      return converted_str;
    }

    ATUG2_API string StrFromWcs(const astring& _data) {
      /*size_t size = 0;
      ALLOC_ARR(Char, data, _data.length() + 2);
      errno_t err = wcstombs_s(&size, data, _data.length() + 2, _data.c_str(), _TRUNCATE);
      string str_to_return = data;
      SAFE_FREE(data);*/
      string converted_str;
      converted_str.assign(_data.begin(), _data.end());
      return converted_str;
    }
#pragma warning(default: 4244)
    ATUG2_API astring UnicodeFromUtf8(const string& _utf8) {
      wstring ws;
      wchar_t wc;
      for (size_t i = 0; i < _utf8.length(); ) {
        char c = _utf8[i];
        if ((c & 0x80) == 0) {
          wc = c;
          ++i;
        } else if ((c & 0xE0) == 0xC0) {
          wc = (_utf8[i] & 0x1F) << 6;
          wc |= (_utf8[i + 1] & 0x3F);
          i += 2;
        } else if ((c & 0xF0) == 0xE0) {
          wc = (_utf8[i] & 0xF) << 12;
          wc |= (_utf8[i + 1] & 0x3F) << 6;
          wc |= (_utf8[i + 2] & 0x3F);
          i += 3;
        } else if ((c & 0xF8) == 0xF0) {
          wc = (_utf8[i] & 0x7) << 18;
          wc |= (_utf8[i + 1] & 0x3F) << 12;
          wc |= (_utf8[i + 2] & 0x3F) << 6;
          wc |= (_utf8[i + 3] & 0x3F);
          i += 4;
        } else if ((c & 0xFC) == 0xF8) {
          wc = (_utf8[i] & 0x3) << 24;
          wc |= (_utf8[i] & 0x3F) << 18;
          wc |= (_utf8[i] & 0x3F) << 12;
          wc |= (_utf8[i] & 0x3F) << 6;
          wc |= (_utf8[i] & 0x3F);
          i += 5;
        } else if ((c & 0xFE) == 0xFC) {
          wc = (_utf8[i] & 0x1) << 30;
          wc |= (_utf8[i] & 0x3F) << 24;
          wc |= (_utf8[i] & 0x3F) << 18;
          wc |= (_utf8[i] & 0x3F) << 12;
          wc |= (_utf8[i] & 0x3F) << 6;
          wc |= (_utf8[i] & 0x3F);
          i += 6;
        }
        ws += wc;
      }
      return ws;
    }

    ATUG2_API string Utf8FromUnicode(const wstring& _unicode) {
#pragma warning( disable : 4333 )
#pragma warning( disable : 4244 )
      string s;
      for (size_t i = 0; i < _unicode.size(); ++i) {
        wchar_t wc = _unicode[i];
        if (0 <= wc && wc <= 0x7f) {
          s += (char)wc;
        } else if (0x80 <= wc && wc <= 0x7ff) {
          s += (0xc0 | (wc >> 6));
          s += (0x80 | (wc & 0x3f));
        } else if (0x800 <= wc && wc <= 0xffff) {
          s += (0xe0 | (wc >> 12));
          s += (0x80 | ((wc >> 6) & 0x3f));
          s += (0x80 | (wc & 0x3f));
        } else if (0x10000 <= wc && wc <= 0x1fffff) {
          s += (0xf0 | (wc >> 18));
          s += (0x80 | ((wc >> 12) & 0x3f));
          s += (0x80 | ((wc >> 6) & 0x3f));
          s += (0x80 | (wc & 0x3f));
        } else if (0x200000 <= wc && wc <= 0x3ffffff) {
          s += (0xf8 | (wc >> 24));
          s += (0x80 | ((wc >> 18) & 0x3f));
          s += (0x80 | ((wc >> 12) & 0x3f));
          s += (0x80 | ((wc >> 6) & 0x3f));
          s += (0x80 | (wc & 0x3f));
        } else if (0x4000000 <= wc && wc <= 0x7fffffff) {
          s += (0xfc | (wc >> 30));
          s += (0x80 | ((wc >> 24) & 0x3f));
          s += (0x80 | ((wc >> 18) & 0x3f));
          s += (0x80 | ((wc >> 12) & 0x3f));
          s += (0x80 | ((wc >> 6) & 0x3f));
          s += (0x80 | (wc & 0x3f));
        }
      }
#pragma warning( default : 4333 )
#pragma warning( default : 4244 )
      return s;
    }

#if defined(_WINDOWS)
    ATUG2_API string AnsiFromUtf8(const string & _utf8) {
      const char* pszCode = _utf8.c_str();
      BSTR    bstrWide;
      char*   pszAnsi;
      int     nLength;

      nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, (int)strlen(pszCode) + 1, NULL, NULL);
      bstrWide = SysAllocStringLen(NULL, nLength);

      MultiByteToWideChar(CP_UTF8, 0, pszCode, (int)strlen(pszCode) + 1, bstrWide, nLength);

      nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
      pszAnsi = new char[nLength];

      WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
      SysFreeString(bstrWide);

      string ret{ pszAnsi }; delete[] pszAnsi;
      return ret;
    }
    ATUG2_API string Utf8FromAnsi(const string & _ansi) {
      const char* pszCode = _ansi.c_str();
      int     nLength, nLength2;
      BSTR    bstrCode;
      char*   pszUTFCode = NULL;

      nLength = MultiByteToWideChar(CP_ACP, 0, pszCode, (int)strlen(pszCode), NULL, NULL);
      bstrCode = SysAllocStringLen(NULL, nLength);
      MultiByteToWideChar(CP_ACP, 0, pszCode, (int)strlen(pszCode), bstrCode, nLength);

      nLength2 = WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, 0, NULL, NULL);
      pszUTFCode = new char[nLength2 + 1];
      WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, nLength2, NULL, NULL);

      string ret{ pszUTFCode }; delete[] pszUTFCode;
      return ret;
    }
#elif defined(_LINUX)

#endif
    ATUG2_API wstring Replace(wstring& _target,
      const wstring& _to_be_replaced, const wstring& _new_one) {
      size_t pos{ 0 };
      while (wstring::npos NE(pos = _target.find(_to_be_replaced, pos))) {
        _target = _target.replace(pos, _to_be_replaced.length(), _new_one);
      }
      return _target;
    }
    ATUG2_API wstring Trim(wstring& _s, const wstring& _drop /*= TRIM_SPACE*/) {
      wstring r = _s;
      return Rtrim(Ltrim(r, _drop), _drop);
    }
    ATUG2_API wstring Ltrim(wstring& _s, const wstring& _drop /*= TRIM_SPACE*/) {
      wstring r = _s;
      return r.erase(0, r.find_first_not_of(_drop));
    }
    ATUG2_API wstring Rtrim(wstring& _s, const wstring& _drop /*= TRIM_SPACE*/) {
      wstring r = _s;
      return r.erase(r.find_last_not_of(_drop) + 1);
    }
    




    struct StrProp : public IHeapTracker {
      StrProp() { Clear();  }
      ~StrProp() { 
        for each (BytePtr s in str) {
          SAFE_FREE(s);
        }
      }
      Void Clear() {
        for (Number i = CharEnc::kUnknown; CharEnc::kCnt NE i; ++i) {
          str[i] = nullptr;
          size[i] = 0;
        }
      }
      BytePtr str[CharEnc::kCnt];
      SizeT size[CharEnc::kCnt];
    };

    AStringContainer::AStringContainer() : IRefCounter(), strp_(new StrProp) {
    }
    AStringContainer::AStringContainer(const astring& _s) : IRefCounter(), strp_(new StrProp) {
      Set(_s);
    }
    AStringContainer::AStringContainer(const string& _s, CharEnc _enc /*= kUtf8*/) : IRefCounter(), strp_(new StrProp) {
      Set(_s, _enc);
    }
    AStringContainer::AStringContainer(const AStringContainer& _copy) : IRefCounter(_copy) {
      this->strp_ = _copy.strp_;
    }
    AStringContainer::~AStringContainer() {
      if (false EQ IsReferenced()) {
        SAFE_DELETE(strp_);
      }
    }
    Wstr AStringContainer::GetWstr() const {
      return (wchar_t*)(strp_->str[CharEnc::kUnicode]);
    }
    Str AStringContainer::GetStr(const CharEnc _enc /*= CharEnc::kUtf8*/) const {
      return (char*)(strp_->str[_enc]);
    }
    Void AStringContainer::Set(const astring& _src) {
      const size_t bufsize{ _src.length() * 2 + 2 };
      const CharEnc encoding{ CharEnc::kUnicode };
      SAFE_FREE(strp_->str[encoding]);

      size_t size{ 0 };
      ALLOC_ARR(Byte, alloced, bufsize);
      memcpy_s(alloced, bufsize, _src.c_str(), _src.length()*sizeof(Wchar));//alloced = (BytePtr)_src.c_str();
      strp_->str[encoding] = alloced;
      strp_->size[encoding] = bufsize - 2;
    }
    Void AStringContainer::Set(const string& _src, const CharEnc _enc /*= CharEnc::kUtf8*/) {
      const size_t bufsize{ _src.length() + 1 };
      const CharEnc encoding{ _enc };
      SAFE_FREE(strp_->str[encoding]);

      ALLOC_ARR(Byte, alloced, bufsize);
      memcpy_s(alloced, bufsize, _src.c_str(), _src.length()*sizeof(Char));//alloced = (BytePtr)_src.c_str();
      strp_->str[encoding] = alloced;
      strp_->size[encoding] = bufsize - 1;
    }
    const BytePtr AStringContainer::GetByte(const CharEnc _enc /*= CharEnc::kUtf8*/) {
      if (nullptr EQ strp_->str[_enc]) {
        Encode(_enc);
      }
      return strp_->str[_enc];
    }
    const SizeT& AStringContainer::GetByteLen(const CharEnc _enc /*= CharEnc::kUtf8*/) const {
      return strp_->size[_enc];
    }
    Void AStringContainer::Encode(const CharEnc _type) {
      if (nullptr NE strp_->str[_type] AND 0 NE strp_->size[_type]) { return; }

      switch (_type) {
        case CharEnc::kUnknown:
#pragma BUILD_MSG("TODO: should throw a pre-defined exception.");
          break;
        case CharEnc::kAnsi:
          if (strp_->str[CharEnc::kUtf8]) {
            Set(AnsiFromUtf8((char*)strp_->str[CharEnc::kUtf8]), CharEnc::kAnsi);
          } else if (strp_->str[CharEnc::kUnicode]) {
            Set(AnsiFromUtf8(Utf8FromUnicode((wchar_t*)strp_->str[CharEnc::kUnicode])), CharEnc::kAnsi);
          } break;
        case CharEnc::kUtf8:
          if (strp_->str[CharEnc::kUnicode]) {
            Set(Utf8FromUnicode((wchar_t*)strp_->str[CharEnc::kUnicode]), CharEnc::kUtf8);
          } else if (strp_->str[CharEnc::kAnsi]) {
            Set(Utf8FromAnsi((char*)strp_->str[CharEnc::kAnsi]), CharEnc::kUtf8);
          } break;
        case CharEnc::kUnicode:
          if (strp_->str[CharEnc::kUtf8]) {
            Set(UnicodeFromUtf8((char*)strp_->str[CharEnc::kUtf8]));
          } else if (strp_->str[CharEnc::kAnsi]) {
            Set(WcsFromStr((char*)strp_->str[CharEnc::kAnsi]));
          } break;
      }//!switch
    }
    Void AStringContainer::Clear() {
      SAFE_DELETE(strp_);
      strp_ = new StrProp;
    }

    // struct DataProp
    struct DataProp : public IHeapTracker {
      DONT_ALLOW_ACCESS_AS_PTR;
      DataProp(const BytePtr _bytes, const SizeT _len) : data(nullptr), size(ZERO), used(ZERO) {
        _TRACE("DataProp ctor");
        if (_len) {
          assert(nullptr NE _bytes);
          Overwrite(_bytes, _len);
        }
      }
      ~DataProp() {
        _TRACE("DataProp dtor");
        Destroy();
      }
      Void Destroy() {
        used = ZERO;
        size = ZERO;
        SAFE_FREE(data);
      }
      Void Empty() {
        used = ZERO;
        size = ZERO;
        data = nullptr;
      }
      Void Prepare(const SizeT _len) {
        assert(ZERO < _len);
        if (size >= _len) { SET_VAL(data, ZERO, Byte, size); return; }
        if (nullptr NE data) { Destroy(); }
        _TRACE(_len << " of memory block prepared");
        // prepare memory block
        size = _len;
        ALLOC(data, Byte, size);
      }
      Void Overwrite(const BytePtr _bytes, const SizeT _len) {
#pragma BUILD_MSG("no matter '_bytes' has been allocated or not, this function will allocate a new memory block. make sure you deal with '_bytes' memory outside of this function")
        Prepare(_len);
        memcpy_s(data, size, _bytes, _len);
        used = _len;
      }
      const Bool Attach(const BytePtr _bytes, const SizeT _len) {
        SizeT remaining{ size - used };
        if (remaining >= _len) {
          memcpy_s(&data[used], remaining, _bytes, _len);
          used += _len;
          return true;
        } else {
          return false;
        }
      }
      const Bool IsEmpty() const {
        return (nullptr EQ data);
      }
      BytePtr data;
      SizeT size;
      SizeT used;
    };
    //!struct DataProp


    // ADataBucket::ADataBucket
    ADataBucket::ADataBucket(const SizeT _blocksize /*= 0*/) : IRefCounter(), datap_(new DataProp(nullptr, ZERO)) {
      _TRACE("ADataBucket ctor - blocksize of " << _blocksize);
      Prepare(_blocksize);
    }
    ADataBucket::ADataBucket(const BytePtr _src, const SizeT _len) : IRefCounter(), datap_(new DataProp(_src, _len)) {
      _TRACE("ADataBucket ctor - filled one");
    }
    ADataBucket::ADataBucket(const ADataBucket& _copy) : IRefCounter(_copy) {
      _TRACE("ADataBucket ctor - copy");
      this->datap_ = _copy.datap_;
    }
    ADataBucket::~ADataBucket() {
      _TRACE("ADataBucket dtor");
      if (false EQ IsReferenced()) {
        SAFE_DELETE(datap_);
      }
    }

    const Bool ADataBucket::IsEmpty() const {
      return (nullptr EQ datap_->data) ? true : false;
    }

    // no matter the '_src' has been allocated or not, 
    // Fill allocate a new memory
    const Bool ADataBucket::Fill(const BytePtr _src, const SizeT _len) {
      datap_->Overwrite(_src, _len);
      return true;
    }
    const Bool ADataBucket::Attach(const BytePtr _src, const SizeT _len) {
      return datap_->Attach(_src, _len);
    }
    const Bool ADataBucket::operator<<(const SizeT& _to_shift_front) {
      if (_to_shift_front <= datap_->size) {
        const SizeT size_remaining{ datap_->size - _to_shift_front };
        memmove(datap_->data, &datap_->data[_to_shift_front], size_remaining);
        memset(&datap_->data[size_remaining], ZERO, _to_shift_front);
        datap_->used -= _to_shift_front;
        return true;
      } else {
        return false;
      }
    }

    Void ADataBucket::Prepare(const SizeT _blocksize) {
      datap_->Prepare(_blocksize);
    }
    const BytePtr& ADataBucket::Get() const {
      return (datap_->data);
    }
    const SizeT& ADataBucket::UsedLength() const {
      return (datap_->used);
    }
    const SizeT& ADataBucket::Size() const {
      return (datap_->size);
    }
    //!ADataBucket::ADataBucket

}//! namespace adata
}//! namespace atug
