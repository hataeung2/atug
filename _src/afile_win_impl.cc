#include "afile.h"
#include "afile_win_impl.h"

namespace atug2 {

  FileImplWin::FileImplWin() {
  }

  FileImplWin::~FileImplWin() {
  }

  const Dword FileImplWin::GetError() {
    err_ = GetLastError();
    return err_;
  }

  const Bool FileImplWin::IsExistingInDir() {
    wstring filepath = filep_->path_.GetFullpath();
      //wstring(filep_->dir_) + DIV + wstring(filep_->name_) + L'.' + wstring(filep_->extension_);
    // @result of _access is { true: 0, false: -1 }
    return (0 EQ access(adata::StrFromWcs(filepath).c_str(), 0)) ? TRUE : FALSE; 
  }

  const Bool FileImplWin::Open() {

    Dword open{ 0 };
    if (filep_->open_type_ EQ FileOpenType::kOpenExist) open = OPEN_EXISTING;
    else if (filep_->open_type_ EQ FileOpenType::kOpenAlways) open = OPEN_ALWAYS;
    else if (filep_->open_type_ EQ FileOpenType::kCreateNew) open = CREATE_NEW;
    else if (filep_->open_type_ EQ FileOpenType::kCreateAlways) open = CREATE_ALWAYS;

    Dword access_flags{ 0 };
    if (filep_->access_flagset_ & kRead) access_flags |= GENERIC_READ;
    if (filep_->access_flagset_ & kWrite) access_flags |= GENERIC_WRITE;
      
    Dword share_flags{ 0 };
    if (filep_->share_flagset_ & kShareRead) share_flags |= FILE_SHARE_READ;
    if (filep_->share_flagset_ & kShareWrite) share_flags |= FILE_SHARE_WRITE;
    if (filep_->share_flagset_ & kShareDelete) share_flags |= FILE_SHARE_DELETE;

    Dword fileattr_flags{ 0 };
    if (filep_->attr_flagset_ & kOverlapped) {
      //TODO: OVERLAPPED 
      fileattr_flags |= FILE_FLAG_OVERLAPPED;
    }
    if (filep_->attr_flagset_ & kDeleteOnClose) fileattr_flags |= FILE_FLAG_DELETE_ON_CLOSE;
    if (filep_->attr_flagset_ & kReadOnly) fileattr_flags |= FILE_ATTRIBUTE_READONLY;
    if (filep_->attr_flagset_ & kNormal) fileattr_flags = FILE_ATTRIBUTE_NORMAL/*OVERRIDE OTHERS*/;

    //wstring(filep_->dir_ + DIV + filep_->name_ + L'.' + filep_->extension_).c_str()
    //wstring filename{ SWS(filep_->dir_) + DIV + SWS(filep_->name_) + L'.' + SWS(filep_->extension_) };
    wstring filename{ filep_->path_.GetFullpath() };
    filehandle_ = CreateFile(filename.c_str(),
      access_flags,
      share_flags, 
      NULL, 
      open,
      fileattr_flags, 
      NULL);
      
    Bool result = TRUE;
    if (INVALID_HANDLE_VALUE EQ filehandle_) {
      err_ = GetLastError();
      filehandle_ = NULL;
      result = FALSE;
    }
    return result;
  }
  const Void FileImplWin::Close() {
    CloseHandle(filehandle_);
    filehandle_ = NULL;
  }
  const SizeT FileImplWin::Read(const ADataBucket& _d) {
    Number result = ReadFile(filehandle_,
      (LPVOID)_d.Get(),
      filep_->handling_bytes_at_once_,
      (LPDWORD)(&_d.Size()), 
      (LPOVERLAPPED)NULL); 
#pragma BUILD_MSG("TODO: OVERLAPPED USE")
    //if (ZERO EQ result) {
    //  if (ERROR_IO_PENDING EQ GetLastError()) {
    //    // not err
    //  } else {
    //    return 0;
    //  }
    //}
    _d.Get()[_d.Size()] = ENDA;
    return _d.Size();
  }
  const Bool FileImplWin::Bof() {
    const FilePtrPos pos{ GetFilePtrPos() };
    return (pos EQ ZERO); // When getting -1 it means the case further above
  }
  const Bool FileImplWin::Eof() {
    const FilePtrPos pos{ GetFilePtrPos() };
    LARGE_INTEGER size; GetFileSizeEx(filehandle_, &size);
    return (pos >= size.QuadPart);
  }
  const Int FileImplWin::Write(const ADataBucket& _d) {
    Int written{ 0 };
    Number result = WriteFile(filehandle_,
      (LPCVOID)_d.Get(),
      (DWORD)_d.Size(),
      (LPDWORD)&written,
      (LPOVERLAPPED)NULL);
#pragma BUILD_MSG("TODO: OVERLAPPED USE")
    return written;
  }
  const Int FileImplWin::Write(const AStringContainer& _str, const CharEnc _enc) {
    Int written{ 0 };
    Number result = WriteFile(filehandle_,
      (LPCVOID)const_cast<AStringContainer&>(_str).GetByte(_enc),
      (DWORD)_str.GetByteLen(_enc),
      (LPDWORD)&written,
      (LPOVERLAPPED)NULL);
    return written;
  }
  const Int FileImplWin::Write(const BytePtr _str, const Int _len)
  {
    Int written{ 0 };
    Number result = WriteFile(filehandle_, _str, _len, (LPDWORD)&written, NULL);
    return written;
  }
  const FilePtrPos FileImplWin::SetFilePtrPos(const Number _dist_move, const FilePtrStd _std_pos) {
    LARGE_INTEGER fp; fp.QuadPart = _dist_move;
    LARGE_INTEGER moved_file_ptr;
    if (SetFilePointerEx(filehandle_,
      fp, /*from "starting point"*/
      &moved_file_ptr,
      (DWORD)_std_pos /*starting point: FILE_BEGIN(default), FILE_CURRENT, FILE_END*/
    )) {
      return moved_file_ptr.QuadPart;
    }
    return INVALID_FILE_PTR;
    //return SetFilePtrPos(AData::Create(&fp, sizeof(LARGE_INTEGER), nullptr, (Int)_std_pos));
  }
  const FilePtrPos FileImplWin::GetFilePtrPos() {
    LARGE_INTEGER dist_to_move; dist_to_move.QuadPart = 0;
    LARGE_INTEGER moved_file_ptr;
    if (SetFilePointerEx(filehandle_, dist_to_move, &moved_file_ptr, FILE_CURRENT)) {
      return moved_file_ptr.QuadPart;
    }
    return INVALID_FILE_PTR;
  }

}//!namespace atug
