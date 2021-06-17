#ifndef _ATUG2_CRC_H_
#define _ATUG2_CRC_H_
#include "atug2.h"

namespace atug2 {
  class ATUG2_API Crc {
  public:
    static unsigned short GetCrc16xmodem(unsigned char* buf, int len);
  };
}//!namespace atug2 {
#endif//!_ATUG2_CRC_H_