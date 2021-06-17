#include "afile.h"
#include "afile_win_impl.h"

namespace atug2 {
  
  AFile::AFile() : fileimpl_(nullptr) {
#if defined(_WINDOWS)
    fileimpl_ = new FileImplWin;
#elif defined(_LINUX)
#elif defined(_OSX)
#endif
  }

  Void AFile::Release() { SAFE_DELETE(fileimpl_); }
  AFile::~AFile() { Release(); }
  ASharedPtr<AFile> AFile::Create(
    Wstr _name, Wstr _extension /*= L"afile"*/, Wstr _dir /*= GetCurrentDir().c_str()*/,
    FileOpenType _open_type /*= kOpenExist*/, 
    Dword _access_flags /*= kRead | kWrite*/,
    Dword _share_flags /*= kShareRead*/,
    Dword _attr_flags /*= kNormal*/, 
    Int _handling_bytes_at_once /*= 512*/) {
    AFile* new_file{ new AFile };
    if (!new_file->fileimpl_) { _TRACE("FileImpl is empty."); return ASharedPtr<AFile>(nullptr); }
    new_file->fileimpl_->Set(
      new FileProp(_name, _extension, _dir, 
        _open_type, _access_flags, _share_flags, _attr_flags, _handling_bytes_at_once) );
    return ASharedPtr<AFile>(new_file);
  }

  const Bool AFile::IsOpened() const {
    return fileimpl_->IsOpened();
  }

  const FileProp& AFile::GetFileAttr() const {
    return *fileimpl_->filep_;
  }

  Void AFile::SetFileName(Wstr _name, Wstr _ext) {
    fileimpl_->filep_->path_.SetName(_name);
    fileimpl_->filep_->path_.SetExtension(_ext);
  }

  const Dword AFile::GetError() {
    return fileimpl_->GetError();
  }

  const Bool AFile::IsExistingInDir()
  {
    return fileimpl_->IsExistingInDir();
  }

  const Bool AFile::Open() {
    return fileimpl_->Open();
  }

  const Void AFile::Close() {
    return fileimpl_->Close();
  }

  const SizeT AFile::Read(const ADataBucket& _d) {
    return fileimpl_->Read(_d);
  }

  const Bool AFile::Bof() {
    return fileimpl_->Bof();
  }

  const Bool AFile::Eof() {
    return fileimpl_->Eof();
  }

  const Int AFile::Write(const ADataBucket& _d) {
    return fileimpl_->Write(_d);
  }
  const Int AFile::Write(const AStringContainer& _str, const CharEnc _enc) {
    return fileimpl_->Write(_str, _enc);
  }

  const Int AFile::Write(const BytePtr _str, const Int _len)
  {
    return fileimpl_->Write(_str, _len);
  }

  const FilePtrPos AFile::SetFilePtrPos(const Number _dist_move, const FilePtrStd _std_pos) {
    return fileimpl_->SetFilePtrPos(_dist_move, _std_pos);
  }

  const FilePtrPos AFile::GetFilePtrPos() {
    return fileimpl_->GetFilePtrPos();
  }

  AFile::FileImpl::FileImpl() 
    : filep_(nullptr), filehandle_(NULL), err_(NULL) {
  }

  AFile::FileImpl::~FileImpl() {
    SAFE_DELETE(filep_);
  }

  const Bool AFile::FileImpl::IsOpened() const {
    return (filehandle_) ? TRUE : FALSE;
  }

  const FileProp* AFile::FileImpl::get_fileattr_() const {
    return filep_;
  }

    

  Void AFile::FileImpl::Set(const FileProp* _prop) {
    filep_ = const_cast<FileProp*>(_prop);
  }

}//!namespace atug2
