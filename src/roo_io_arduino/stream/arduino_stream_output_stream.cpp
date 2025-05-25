#include "roo_io_arduino/stream/arduino_stream_output_stream.h"

namespace roo_io {

ArduinoStreamOutputStream::ArduinoStreamOutputStream(Stream& output)
    : output_(output), status_(kOk) {}

size_t ArduinoStreamOutputStream::tryWrite(const byte* buf, size_t count) {
  if (status() != kOk) return 0;
  size_t available = output_.availableForWrite();
  if (count > available) count = available;
  if (count == 0) return 0;
  return output_.write((const uint8_t*)buf, count);
}

size_t ArduinoStreamOutputStream::write(const byte* buf, size_t count) {
  if (status() != kOk) return 0;
  size_t available = output_.availableForWrite();
  if (count > available) count = available;
  if (count == 0) ++count;
  return output_.write((const uint8_t*)buf, count);
}

size_t ArduinoStreamOutputStream::writeFully(const byte* buf, size_t count) {
  if (status() != kOk) return 0;
  return output_.write((const uint8_t*)buf, count);
}

void ArduinoStreamOutputStream::flush() {
  // Arduino 'flush' is stronger than our semantics, in that it waits for the
  // data to be actually sent out, which we don't require. if (status() != kOk)
  // return; output_.flush();
}

void ArduinoStreamOutputStream::close() {
  flush();
  status_ = kClosed;
}

}  // namespace roo_io