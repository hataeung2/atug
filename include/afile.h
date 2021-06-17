#ifndef _ATUG2_FILE_H_
#define _ATUG2_FILE_H_
#include "afile_attr.h"
#include "ashared_ptr.h"
#if defined(_WINDOWS)
#define INVALID_FILEPTR -1
#elif defined(_LINUX)
#define INVALID_FILEPTR ?
#endif
using namespace atug2::adata;
namespace atug2 {

  class ATUG2_API AFile : public IHeapTracker {
  public:
    DONT_ALLOW_COPY_AND_ASSIGN(AFile)
  private: explicit AFile();
  public: ~AFile();
  private: Void Release();
  public: static ASharedPtr<AFile> Create(
    Wstr _name, Wstr _extension = L"afile", Wstr _dir = GetCurrentDir().c_str(),
    FileOpenType _open_type = FileOpenType::kOpenExist, 
    Dword _access_flags = kRead | kWrite,
    Dword _share_flags = kShareRead,
    Dword _attr_flags = kNormal, Int _handling_bytes_at_once = 512);
  public:
    const Bool IsOpened() const;
    const FileProp& GetFileAttr() const;
    Void SetFileName(Wstr _name, Wstr _ext);
  public:
    const Dword GetError();
    const Bool IsExistingInDir();
    const Bool Open();
    const Void Close();
    const SizeT Read(const ADataBucket& _d);
    const Bool Bof();
    const Bool Eof();
    const Int Write(const ADataBucket& _d);
    const Int Write(const AStringContainer& _str, const CharEnc _enc);
    const Int Write(const BytePtr _str, const Int _len);

    // _dist_move: distance to move
    // _std_pos: standard position / FILE_BEGIN, FILE_CURRENT, FILE_END
    const FilePtrPos SetFilePtrPos(const Number _dist_move, const FilePtrStd _std_pos);
    const FilePtrPos GetFilePtrPos();
    
  public:
    class FileImpl {
      friend class AFile;
    public:
      FileImpl();
      virtual ~FileImpl();
    public:
      const Bool IsOpened() const;
      const FileProp* get_fileattr_() const;
    public:
      virtual const Dword GetError() PURE;
      virtual const Bool IsExistingInDir() PURE;
      virtual const Bool Open() PURE;
      virtual const Void Close() PURE;
      virtual const SizeT Read(const ADataBucket& _d) PURE;
      virtual const Bool Bof() PURE;
      virtual const Bool Eof() PURE;
      virtual const Int Write(const ADataBucket& _d) PURE;
      virtual const Int Write(const AStringContainer& _str, const CharEnc _enc) PURE;
      virtual const Int Write(const BytePtr _str, const Int _len) PURE;
      virtual const FilePtrPos SetFilePtrPos(const Number _dist_move, const FilePtrStd _std_pos) PURE;
      virtual const FilePtrPos GetFilePtrPos() PURE;
    protected:
      Void Set(const FileProp* _prop);
    protected:
      FileProp* filep_;
      Handle filehandle_;
      Dword err_;
    };
    FileImpl* fileimpl_;
  };
    
}//!namespace atug2

#endif//!_ATUG2_FILE_H_
