#pragma once

#include <FS.h>

#include <cstring>
#include <memory>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

static const size_t kArduinoFileOutputIteratorBufferSize = 64;

class ArduinoFileOutputIterator {
 public:
  ArduinoFileOutputIterator() : rep_(new Rep()) {}

  ArduinoFileOutputIterator(::fs::File file) : rep_(new Rep(std::move(file))) {}

  ArduinoFileOutputIterator(ArduinoFileOutputIterator&&) = default;

  ~ArduinoFileOutputIterator() { flush(); }

  void write(byte v) { rep_->write(v); }

  size_t write(const byte* buf, size_t count) {
    return rep_->write(buf, count);
  }

  // rep_ can be nullptr after std::move().
  void flush() {
    if (rep_ != nullptr) rep_->flush();
  }

  Status status() const { return rep_->status(); }
  bool ok() const { return status() == roo_io::kOk; }

  //   void reset() { rep_->reset(nullptr); }
  //   void reset(::fs::File file) { rep_->reset(std::move(file)); }

 private:
  class Rep {
   public:
    Rep();
    Rep(::fs::File file);
    // ~Rep();

    void write(byte v);
    size_t write(const byte* buf, size_t count);
    void flush();

    Status status() const { return status_; }

    // void reset(::fs::File file);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&);
    void writeBuffer();

    ::fs::File file_;
    byte buffer_[kArduinoFileOutputIteratorBufferSize];
    uint8_t offset_;
    Status status_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline ArduinoFileOutputIterator::Rep::Rep()
    : file_(),
      offset_(kArduinoFileOutputIteratorBufferSize),
      status_(kClosed) {}

inline ArduinoFileOutputIterator::Rep::Rep(::fs::File file)
    : file_(std::move(file)), offset_(0), status_(file_ ? kOk : kClosed) {
  if (status_ != kOk) offset_ = kArduinoFileOutputIteratorBufferSize;
}

// inline void ArduinoFileOutputIterator::Rep::reset(
//     roo_io::OutputStream* output) {
//   output_ = output;
//   offset_ = 0;
//   status_ = (output != nullptr) ? output->status() : kClosed;
// }

inline void ArduinoFileOutputIterator::Rep::writeBuffer() {
  if (file_.write((const uint8_t*)buffer_, offset_) < offset_) {
    status_ = kWriteError;
  }
  offset_ = 0;
}

inline void ArduinoFileOutputIterator::Rep::write(byte v) {
  if (offset_ >= kArduinoFileOutputIteratorBufferSize) {
    if (status_ != kOk) return;
    writeBuffer();
  }
  buffer_[offset_++] = v;
}

inline size_t ArduinoFileOutputIterator::Rep::write(const byte* buf,
                                                    size_t len) {
  if (offset_ > 0 || len < kArduinoFileOutputIteratorBufferSize) {
    int cap = kArduinoFileOutputIteratorBufferSize - offset_;
    if (len > cap) len = cap;
    memcpy(&buffer_[offset_], buf, len);
    offset_ += len;
    if (offset_ >= kArduinoFileOutputIteratorBufferSize) {
      if (status_ != kOk) return 0;
      writeBuffer();
    }
    return len;
  }
  if (status_ != kOk) return 0;
  int result = file_.write((const uint8_t*)buf, len);
  if (result < len) {
    status_ = kWriteError;
  }
  return result;
}

inline void ArduinoFileOutputIterator::Rep::flush() {
  if (status_ == kOk) {
    if (offset_ > 0) writeBuffer();
    file_.flush();
    if (!file_) status_ = kWriteError;
  }
}

}  // namespace roo_io
