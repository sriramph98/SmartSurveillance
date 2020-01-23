#pragma once
#include <em/EventManager.hpp>
#include <binge/Types.hpp>
#include <string>

namespace binge {
enum class FrameReaderStates { ERROR, READ, COMPLETE };

class Binge: public em::EventManager<FrameReaderStates, Frame> {
  using FrameReaderCallback = EventCallback;

 public:
  Binge();

  void read(std::string input);
  void read(const char* input);

  ~Binge();
};
}