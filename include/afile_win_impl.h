#ifndef _ATUG2_FILE_WIN_IMPL_H_
#define _ATUG2_FILE_WIN_IMPL_H_
#include "afile.h"

using namespace atug2::adata;
namespace atug2 {

  class FileImplWin : public AFile::FileImpl {
    DONT_ALLOW_COPY_AND_ASSIGN(FileImplWin);
  public:
    FileImplWin();
    virtual ~FileImplWin();
  public:
    virtual const Dword GetError();
    virtual const Bool IsExistingInDir();
    virtual const Bool Open();
    virtual const Void Close();
    virtual const SizeT Read(const ADataBucket& _d);
    virtual const Bool Bof();
    virtual const Bool Eof();
    virtual const Int Write(const ADataBucket& _d);
    virtual const Int Write(const AStringContainer& _str, const CharEnc _enc);
    virtual const Int Write(const BytePtr _str, const Int _len);

    virtual const FilePtrPos SetFilePtrPos(const Number _dist_move, const FilePtrStd _std_pos);
      
    /**
    * @return: current file pointer position
    * if failed, returns INVALID_SET_FILE_POINTER(-1)
    */
    virtual const FilePtrPos GetFilePtrPos();
  };
    
}//!namespace atug2



#endif//!_ATUG2_FILE_WIN_IMPL_H_
