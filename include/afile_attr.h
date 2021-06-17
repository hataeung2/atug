#ifndef _ATUG2_FILE_ATTR_H_
#define _ATUG2_FILE_ATTR_H_
#include "adata.h"

namespace atug2 {
  
#define INVALID_FILE_PTR -1
#if defined(_WINDOWS)
#define DIV L'\\'
#elif defined(_LINUX) || defined(_OSX)
#define DIV L'/'
#endif

#define DOT L'.'

  class PathStr {
    DONT_ALLOW_COPY_AND_ASSIGN(PathStr);
  public:
    PathStr() {}
    ~PathStr() {}
  public:
    astring GetFullpath() const;
    astring GetDir() const;
    astring GetName() const;
    astring GetExtension() const;
    Void SetDir(Wstr _dir);
    Void SetName(Wstr _name);
    Void SetExtension(Wstr _ext);
  private:
    adata::AStringContainer dir_;
    adata::AStringContainer name_;
    adata::AStringContainer ext_;
  };

  // file functions
  ATUG2_API extern const wstring GetCurrentDir();
  ATUG2_API extern const string GetCurrentDirA();
  ATUG2_API extern const wstring GetFilenameFromFullpath(const wstring& _fullpath);

  // file attributes
  typedef Longlong FilePtrPos;
  enum class FileSystem : char { kWindows = 1, kLinux, kMac, };
  enum class FileOpenType : char { kOpenExist = 1, kOpenAlways, kCreateAlways, kCreateNew, };
  enum FileAccessFlag : char { kRead = 0x01, kWrite = 0x02, };
  enum FileShareFlag : char { kShareRead = 0x01, kShareWrite = 0x02, kShareDelete = 0x03 };
  enum FileAttrFlag : char { kNormal = 1, kOverlapped = 2, kReadOnly, kDeleteOnClose, };
  enum class FilePtrStd : char { kBegin, kCurrent, kEnd };
  struct FileProp {
    ~FileProp();
    FileProp();
    FileProp(Wstr _name, Wstr _extension = L"afile", Wstr _dir = GetCurrentDir().c_str(),
      FileOpenType _open_type = FileOpenType::kOpenExist, Dword _access_flags = kRead | kWrite,
      Dword _share_flags = kShareRead, Dword _attr_flags = kNormal, Int _handling_bytes_at_once = 512);
      
    PathStr path_;
    FileOpenType open_type_;
    Dword access_flagset_;
    Dword share_flagset_;
    Dword attr_flagset_;
    Int handling_bytes_at_once_;
  };

  //// macro for SetFilePtrPos()
#if defined(_WINDOWS)
  // pos_after: file ptr pos after function executed
  // f: file handle
  // move: distance to move
  // from: FILE_BEGIN, FILE_CURRENT, FILE_END
  //#define _SetFilePtrPos(pos_after, f, move, from) \
//{ LARGE_INTEGER fp; fp.QuadPart = move; \
//pos_after = f->SetFilePtrPos(adata::Data::Create(&fp, sizeof(LARGE_INTEGER), nullptr, from)); }
#elif defined(_LINUX)
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#elif defined(_OSX)
#endif

}//!namespace atug2
#endif//!_ATUG2_FILE_ATTR_H_
