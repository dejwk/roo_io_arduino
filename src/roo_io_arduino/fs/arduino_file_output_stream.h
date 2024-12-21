#pragma once

#include <FS.h>

#include "roo_io/core/output_stream.h"

namespace roo_io {

class ArduinoFileOutputStream : public OutputStream {
 public:
  ArduinoFileOutputStream(Status error) : file_(), status_(error) {}

  ArduinoFileOutputStream(fs::File file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    size_t result = file_.write((const uint8_t*)buf, count);
    if (result < count) {
      status_ = roo_io::kWriteError;
    }
    return result;
  }

  void flush() override {
    if (status_ == kClosed) return;
    file_.flush();
    if (!file_) {
      status_ = kWriteError;
    }
  }

  void close() override {
    if (status_ == kClosed) return;
    file_.close();
    if (status_ == kOk) {
      status_ = kClosed;
    }
  }

  Status status() const override { return status_; }

 private:
  fs::File file_;
  Status status_;
};

}  // namespace roo_io