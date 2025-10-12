#pragma once

#include "Arduino.h"

#include "roo_io/core/input_stream.h"

namespace roo_io {

// Adapter to read from an arduino serial as an InputStream. The stream is
// considered opened until close() is explicitly called.
template <typename SerialType>
class ArduinoSerialInputStream : public InputStream {
 public:
  ArduinoSerialInputStream(SerialType& input) : input_(input), status_(kOk) {}

  size_t tryRead(byte* buf, size_t count) override {
    if (!isOpen()) return 0;
    size_t available = input_.available();
    if (count > available) count = available;
    if (count == 0) return 0;
    return input_.readBytes((char*)buf, count);
  }

  size_t read(byte* buf, size_t count) override {
    if (!isOpen() || count == 0) return 0;
    while (true) {
      size_t available = input_.available();
      if (count > available) count = available;
      if (count == 0) ++count;
      size_t result = input_.readBytes((char*)buf, count);
      if (result > 0) return result;
    }
  }

  size_t readFully(byte* buf, size_t count) override {
    if (!isOpen() || count == 0) return 0;
    size_t total = 0;
    while (total < count) {
      size_t result = input_.readBytes((char*)buf, count);
      total += result;
    }
    return total;
  }

  bool isOpen() const override { return status() == kOk; }

  void close() override { status_ = kClosed; }

  Status status() const override { return status_; }

 private:
  SerialType& input_;
  mutable Status status_;
};

}  // namespace roo_io