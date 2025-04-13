#include "roo_io_arduino/stream/arduino_stream_input_stream.h"

namespace roo_io {

ArduinoStreamInputStream::ArduinoStreamInputStream(Stream& input)
    : status_(kOk), input_(input) {}

bool ArduinoStreamInputStream::isOpen() const { return status() == kOk; }

size_t ArduinoStreamInputStream::tryRead(byte* buf, size_t count) {
  if (!isOpen()) return 0;
  size_t available = input_.available();
  if (count > available) count = available;
  if (count == 0) return 0;
  return input_.readBytes(buf, count);
}

size_t ArduinoStreamInputStream::read(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t available = input_.available();
  if (count > available) count = available;
  if (count == 0) ++count;
  return input_.readBytes(buf, count);
}

}  // namespace roo_io