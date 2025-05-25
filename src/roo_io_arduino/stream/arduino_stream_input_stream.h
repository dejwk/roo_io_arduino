#pragma once

#include <Stream.h>

#include "roo_io/core/input_stream.h"

namespace roo_io {

// Adepter to read from an arduino stream as an InputStream. The stream is
// considered opened until close() is explicitly called.
class ArduinoStreamInputStream : public InputStream {
 public:
  ArduinoStreamInputStream(Stream& input);

  size_t tryRead(byte* buf, size_t count) override;

  size_t read(byte* buf, size_t count) override;

  size_t readFully(byte* buf, size_t count) override;

  bool isOpen() const override;

  void close() override { status_ = kClosed; }

  Status status() const override { return status_; }

 private:
  Stream& input_;
  mutable Status status_;
};

}  // namespace roo_io