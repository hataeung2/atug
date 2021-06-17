#include "alog.h"
#include "afile.h"
#include "atime.h"
#include "asafelist.h"
#include <thread>
#include <atomic>
#include <stdlib.h>
#include <clocale>

namespace atug2 {
#define LOGGING_DELAY 1
  // no member functions
  const astring GetLogDir(const astring& _additional_path /*= L"log"*/) {
    astring cur_dir{ GetCurrentDir() };
    //if ((cur_dir.length()-1) NE cur_dir.rfind(DIV)) { cur_dir += DIV; }
    //if (DIV EQ _additional_path.at(0)) { const_cast<astring&>(_additional_path).replace(0, 1, L""); }
    if (DIV NE _additional_path.at(0)) { const_cast<astring&>(_additional_path) = DIV + _additional_path; }
    astring log_dir = cur_dir + _additional_path;
    if (_wmkdir(log_dir.c_str())/*0 : ok, -1 : err*/) {
      switch (errno) {
      case EEXIST:
        return log_dir;
      case ENOENT:
      default:
        return cur_dir;
      }
    }
    return log_dir;
  }

  struct LogProp {
    LogProp()
      : logthread_(new std::thread),
      is_log_running_(false),
	    time_save_unit_(TimeUnit::kDay),
      logthread_condition_to_continue_(true),
      file_(nullptr), encoding_(adata::CharEnc::kUtf8) {

      setlocale(LC_ALL, "");
    }
    ~LogProp() {
      if (logthread_->joinable()) { logthread_->join(); }
      SAFE_DELETE(logthread_);
    }
    Void Set(astring _logpath, const TimeUnit _time_save_unit, const adata::CharEnc _enc /*= adata::CharEnc::kUtf8*/) {
      logpath_ = _logpath;
      time_save_unit_ = _time_save_unit;
      encoding_ = _enc;
    }
    bool is_log_running_;
    astring current_filename_;
    astring logpath_;
    TimeUnit time_save_unit_; // day, hour, min, sec
    ASafeList<AStringContainer> list_log_;
    std::thread* logthread_;
    std::atomic_bool logthread_condition_to_continue_;
    ASharedPtr<AFile> file_;
    CharEnc encoding_;
  };

  // no memeber functions
  const Bool IsTimeToSliceLogFile(LogProp* _prop) {
    return _prop->file_->GetFileAttr().path_.GetName() NE ALocalTime::GetWstr(_prop->time_save_unit_);
  }
  const Bool NewLogFile(LogProp* _prop) {
    if (_prop->file_->IsOpened()) { _prop->file_->Close(); }

    const FileProp& fileattr = _prop->file_->GetFileAttr();
    TimeUnit time_unit{ TimeUnit::kMilliSec };
    if (FileOpenType::kCreateAlways EQ fileattr.open_type_) {
      time_unit = TimeUnit::kMilliSec;
    } else if (FileOpenType::kOpenAlways EQ fileattr.open_type_) {
      time_unit = _prop->time_save_unit_;
    } else {
      return false;
    }
    _prop->file_->SetFileName(ALocalTime::GetWstr(_prop->time_save_unit_).c_str(), L"alog");

    if (ZERO NE _prop->logpath_.rfind(DIV)) {
      _prop->logpath_ += DIV;
    }
    astring fullpath{ _prop->logpath_ + fileattr.path_.GetName() };

    if (true NE _prop->file_->Open()) {
      _TRACE("FILE OPEN ERROR. : " << _prop->file_->GetError());
      return false;
    }

    FilePtrPos pos_after{ _prop->file_->SetFilePtrPos(0, FilePtrStd::kEnd) };
    return true;
  }



  //
  // 
  ALog::ALog()
    : logp_(new LogProp) {
  }
  ALog::~ALog() {
    Release();
  }
  Void ALog::Release() {
    Stop();
    SAFE_DELETE(logp_);
  }
  ASharedPtr<ALog> ALog::Create(const astring _path /*= L"log"*/,
    const TimeUnit _timeunit /*= kDay*/, const adata::CharEnc _enc /*= adata::CharEnc::kUtf8*/) {
    auto new_log{ ASharedPtr<ALog>(new ALog) };
    new_log->logp_->Set(GetLogDir(_path), _timeunit, _enc);
    return new_log;
  }

  Void ALog::Start() {
    // thread start. lamda func
    *(logp_->logthread_) = std::thread([&]() -> Int {

      // get log file name 
      logp_->current_filename_ = ALocalTime::GetWstr(logp_->time_save_unit_);
      // create file obj
      ASharedPtr<AFile> f = logp_->file_ =
        AFile::Create(logp_->current_filename_.c_str(), L"alog", logp_->logpath_.c_str(), FileOpenType::kOpenAlways);
      // check if file exist
      const bool file_exist = logp_->file_->IsExistingInDir();
      // log file open
      if (TRUE NE f->Open()) {
        logp_->is_log_running_ = false;
        return f->GetError();
      }
      const CharEnc encoding{ logp_->encoding_ };
      logp_->is_log_running_ = true;
      // set file ptr pos
      FilePtrPos pos = f->SetFilePtrPos(0, FilePtrStd::kEnd);

      //if (file_exist) {
      //  string linefeed = "==============================\r\n";
      //  logp_->file_->Write((BytePtr)linefeed.c_str(), (Int)linefeed.length());
      //}

      // loop
      ASafeList<AStringContainer>& loglist{ logp_->list_log_ };
      while (logp_->logthread_condition_to_continue_) {
        // log thread logic
        if (IsTimeToSliceLogFile(logp_)) {
          NewLogFile(logp_);
        }
        while (TRUE NE loglist.IsEmpty()) {
          logp_->file_->Write(loglist.PopFront(), encoding);
        }
        _asleep(LOGGING_DELAY);
      }
        
      // when escape remaining ones will be written into the file
      while (TRUE NE loglist.IsEmpty()) {
        logp_->file_->Write(loglist.PopFront(), encoding);
      }
      // log file close
      f->Close();
      logp_->is_log_running_ = false;
      return ZERO;
    });
  }

  Void ALog::Stop() {
    logp_->logthread_condition_to_continue_.store(false);
  }
  const Uint ALog::Push(const string& _str) {
    string log = ALocalTime::GetStr();
    log += u8' ' + (_str);
    log += u8"\r\n";
    // string utf8log = adata::UnicodeToUtf8(log);
    return static_cast<Uint>(logp_->list_log_.PushBack(log));
  }
  const Uint ALog::Push(const astring& _str) {
    astring log = ALocalTime::GetWstr();
    log += L' ' + _str;
    log += L"\r\n";
    // string utf8log = adata::UnicodeToUtf8(log);
    return static_cast<Uint>(logp_->list_log_.PushBack(log));
  }
  const Bool ALog::IsStarted()
  {
    return logp_->is_log_running_;
  }
}//!namespace atug
