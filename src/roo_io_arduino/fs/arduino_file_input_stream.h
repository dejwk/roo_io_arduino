#pragma once

#include <FS.h>

#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

class ArduinoFileInputStream : public MultipassInputStream {
 public:
  ArduinoFileInputStream(Status error) : file_(), status_(error) {}

  ArduinoFileInputStream(fs::File file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  size_t read(byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    size_t result = file_.read((uint8_t*)buf, count);
    if (result == 0) {
      status_ = kEndOfStream;
      return 0;
    } else if (result == ((size_t)(-1))) {
      // Indicates an error.
      status_ = kReadError;
      return 0;
    }
    return result;
  }

  void seek(uint64_t offset) override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    status_ = file_.seek(offset) ? kOk : kSeekError;
  }

  void skip(uint64_t count) override {
    if (count < 64) {
      byte buf[count];
      readFully(buf, count);
      return;
    }
    if (!file_.seek(count, SeekCur)) {
      status_ = kSeekError;
      return;
    }
    if (file_.position() > file_.size()) {
      status_ = kEndOfStream;
    }
  }

  uint64_t position() const override { return file_.position(); }

  uint64_t size() override { return file_.size(); }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override {
    file_.close();
    status_ = kClosed;
  }

  Status status() const override { return status_; }

 private:
  fs::File file_;
  Status status_;
};

}  // namespace roo_io