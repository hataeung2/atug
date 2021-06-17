#include "aroutine.h"
#include "asafelist.h"

namespace atug2 {
  ARoutine::ARoutine() : prepared_(false) {
  }
  ARoutine::~ARoutine() {
    prepared_ = false;
  }
  const Bool ARoutine::IsPrepared() const {
    return prepared_;
  }
}//! namespace atug


