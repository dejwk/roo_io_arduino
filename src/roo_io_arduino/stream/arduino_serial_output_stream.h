#pragma once

#include "Arduino.h"
#include "roo_io/core/output_stream.h"
#include "roo_logging.h"

namespace roo_io {

// Adepter to write to an arduino serial as an OutputStream. The stream is
// considered opened until close() is explicitly called.
template <typename SerialType>
class ArduinoSerialOutputStream : public OutputStream {
 public:
  ArduinoSerialOutputStream(SerialType& output)
      : output_(output), status_(kOk) {}

  size_t tryWrite(const byte* buf, size_t count) override {
    if (status() != kOk) return 0;
    size_t available = output_.availableForWrite();
    if (count > available) count = available;
    if (count == 0) return 0;
    return output_.write((const uint8_t*)buf, count);
  }

  size_t write(const byte* buf, size_t count) override {
    if (status() != kOk) return 0;
    size_t available = output_.availableForWrite();
    if (count > available) count = available;
    if (count == 0) ++count;
    return output_.write((const uint8_t*)buf, count);
  }

  size_t writeFully(const byte* buf, size_t count) override {
    // size_t total_written = 0;
    // while (count > 0) {
    //   if (status() != kOk) break;
    //   size_t write_now = output_.availableForWrite();
    //   if (write_now > count) write_now = count;
    //   if (write_now == 0) {
    //     delay(1);
    //     continue;
    //   }
    //   size_t written = output_.write((const uint8_t*)buf, write_now);
    //   buf += written;
    //   count -= written;
    //   total_written += written;
    // }
    // return total_written;
    if (status() != kOk) return 0;
    return output_.write((const uint8_t*)buf, count);
  }

  void flush() override {}

  void close() override {
    flush();
    status_ = kClosed;
  }

  Status status() const override { return status_; }

 private:
  SerialType& output_;
  mutable Status status_;
};

#if (defined ESP32 || defined ROO_TESTING)
#endif

}  // namespace roo_io