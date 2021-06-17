#include "athread.h"
#include "aroutine.h"
#include "atime.h"
#include "aexception.h"
#include <mutex>

namespace atug2 {
  struct ThreadAttr {
    ThreadAttr(ASharedPtr<ARoutine> _routine, const Uint _interval)
      : is_in_break_condition_(false), is_suspended_(false), is_escaped_loop_(false)
      , interval_(_interval), routine_(_routine) { }
    ~ThreadAttr() {
      while (!is_escaped_loop_) {
        _asleep(interval_);
      }
      if (thread_.joinable()) { thread_.join(); }
      _TRACE("Thread joined.");
    }
    std::thread thread_;
    std::mutex thread_door_; // for suspend
    std::condition_variable thread_cv_; // for suspend
    Bool is_in_break_condition_;
    Bool is_suspended_;
    Bool is_escaped_loop_;
    Int interval_;
    ASharedPtr<atug2::ARoutine> routine_;
  };
    
  AThread::~AThread() { Release(); }
  ASharedPtr<AThread> AThread::Create(ASharedPtr<ARoutine> _routine, const Uint _interval /*= 1*/) {
    return (new AThread(_routine, _interval));
  }

  const ThreadResult AThread::Release() {
    Stop();
    SAFE_DELETE(attr_);
    return ThreadResult::kThreadReleased;
  }
  AThread::AThread(ASharedPtr<ARoutine> _routine, const Uint _interval) : attr_(nullptr) {
    if (false EQ _routine->IsPrepared()) {
      throw AException("routine NOT initialized");
    } else {
      attr_ = new ThreadAttr(_routine, _interval);
    }
  }

  Void AThread::Start() {
    // ref get
    Bool& is_thread_in_break_condition{ attr_->is_in_break_condition_ };
    Int& interval{ attr_->interval_ };
    std::condition_variable& cv{ attr_->thread_cv_ };
      
    // thread start
    attr_->thread_ = std::thread([&]() -> ThreadResult {

      RoutineAns ans{ RoutineAns::kErr };

      while (!is_thread_in_break_condition) {
        // suspend condition check
        std::unique_lock<std::mutex> lock(attr_->thread_door_); // needed to use cv.wait 
        if (attr_->is_suspended_) { 
          _TRACE("Thread ID " << attr_->thread_.get_id() << " is suspended.");
          // when suspend == false and thread_cv_ is notify_one or notify_all then can move toward
          cv.wait(lock, [&] {return !attr_->is_suspended_; });
        }

        // run one single routine
        ans = attr_->routine_->Run();
        if (kOk NE ans) {
          astring errmsg{};
          switch (ans) {
          case kErr:
            errmsg = astr("'s routine end with error.");
            break;
          case kBreak:
            errmsg = astr("'s routine requested break loop.");
            break;
          }
          if (errmsg.length()) {
            _TRACE("Thread ID " << attr_->thread_.get_id() << errmsg);
            SetThreadConditionIntoBreak();
          }
        }
        // wait interval
        if (ZERO < interval) { _asleep(interval); }
      }
      _TRACE("Thread loop escaped.");
      attr_->is_escaped_loop_ = true;
      return kThreadComplete;
    });
  }//!Void AThread::Run()

  Void AThread::Join() {
    attr_->thread_.join();
  }
  const Bool AThread::Suspend() {
    _TRACE("Thread of " << attr_->thread_.get_id() << " call Suspend.");
    return (attr_->is_suspended_ = true);
  }
  const Bool AThread::Resume() {
    _TRACE("Thread of " << attr_->thread_.get_id() << " call Resume.");
    attr_->is_suspended_ = false;
    attr_->thread_cv_.notify_one(); // notify_all also same. here we have one place using thread
    return (!attr_->is_suspended_);
  }
  Void AThread::Stop() {
    SetThreadConditionIntoBreak();
  }
  const Bool AThread::IsEscapedFromLoop() {
    return attr_->is_escaped_loop_;
  }
  inline Void AThread::SetInterval(const Uint _interval) {
    attr_->interval_ = _interval;
  }
  inline Void AThread::SetThreadConditionIntoBreak() {
    attr_->is_in_break_condition_ = true;
  }
    
}//!namespace atug2

