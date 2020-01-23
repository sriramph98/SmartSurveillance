#include <binge/Debug.hpp>

#include <clog/clog.h>

using namespace binge;

void Debug::enableLog(bool shouldLogEnable) {
  if (shouldLogEnable) {
    clog_enable();
  } else {
    clog_disable();
  }
}