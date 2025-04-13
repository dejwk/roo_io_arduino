#pragma once

#include <Arduino.h>

#include "roo_io/core/output_stream.h"

namespace roo_io {

// Adepter to write to an arduino stream as an OutputStream. The stream is
// considered opened until close() is explicitly called.
class ArduinoStreamOutputStream : public OutputStream {
 public:
  ArduinoStreamOutputStream(Stream& output);

  size_t tryWrite(const byte* buf, size_t count) override;

  size_t write(const byte* buf, size_t count) override;

  void flush() override;

  void close() override;

  Status status() const override { return status_; }

 private:
  Stream& output_;
  mutable Status status_;
};

}  // namespace roo_io