#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/base/byte.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/status.h"

namespace roo_io {

static const int kFileInputIteratorBufferSize = 64;

class ArduinoFileInputIterator {
 public:
  ArduinoFileInputIterator(::fs::File file) : rep_(new Rep(std::move(file))) {}

  byte read() { return rep_->read(); }

  size_t read(byte* buf, size_t count) { return rep_->read(buf, count); }

  void skip(size_t count) { rep_->skip(count); }
  Status status() const { return rep_->status(); }

  uint64_t size() const { return rep_->size(); }
  uint64_t position() const { return rep_->position(); }

  void rewind() { rep_->rewind(); }
  void seek(uint64_t position) { rep_->seek(position); }

  // void reset(::File file) { rep_->reset(&input); }
  // void reset() { rep_->reset(nullptr); }

 private:
  class Rep {
   public:
    Rep(::fs::File file);
    ~Rep();
    byte read();
    size_t read(byte* buf, size_t count);
    void skip(size_t count);
    Status status() const { return status_; }

    uint64_t size() const;
    uint64_t position() const;

    void rewind();
    void seek(uint64_t position);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&) = delete;
    Rep& operator=(const Rep&) = delete;

    ::fs::File file_;
    byte buffer_[kFileInputIteratorBufferSize];
    uint8_t offset_;
    uint8_t length_;
    Status status_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline ArduinoFileInputIterator::Rep::Rep(::fs::File file)
    : file_(std::move(file)),
      offset_(0),
      length_(0),
      status_(file_ ? kOk : kClosed) {}

inline ArduinoFileInputIterator::Rep::~Rep() { file_.close(); }

inline uint64_t ArduinoFileInputIterator::Rep::size() const {
  return file_.size();
}

inline uint64_t ArduinoFileInputIterator::Rep::position() const {
  return file_.position() + offset_ - length_;
}

inline void ArduinoFileInputIterator::Rep::rewind() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  uint64_t file_pos = file_.position();
  if (file_pos <= length_) {
    // Keep the buffer data and length.
    offset_ = 0;
    status_ = kOk;
  } else {
    // Reset the buffer.
    offset_ = 0;
    length_ = 0;
    status_ = file_.seek(0, SeekSet) ? kOk : kSeekError;
  }
}

inline void ArduinoFileInputIterator::Rep::seek(uint64_t position) {
  if (status_ != kOk && status_ != kEndOfStream) return;
  uint64_t file_pos = file_.position();
  if (file_pos <= position + length_ && file_pos >= position) {
    // Seek within the area we have in the buffer.
    offset_ = position + length_ - file_pos;
    status_ = kOk;
  } else {
    // Seek outside the buffer. Just seek in the file and reset the buffer.
    offset_ = 0;
    length_ = 0;
    status_ = file_.seek(position, SeekSet) ? kOk : kSeekError;
  }
}

inline byte ArduinoFileInputIterator::Rep::read() {
  if (offset_ < length_) {
    return buffer_[offset_++];
  }
  if (status_ != kOk) return byte{0};
  size_t len = file_.read((uint8_t*)buffer_, kFileInputIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return byte{0};
  } else if (len == ((size_t)(-1))) {
    offset_ = 0;
    length_ = 0;
    status_ = kReadError;
    return byte{0};
  }
  offset_ = 1;
  length_ = len;
  return buffer_[0];
}

inline size_t ArduinoFileInputIterator::Rep::read(byte* buf, size_t count) {
  if (offset_ < length_) {
    // Have some data still in the buffer; just return that.
    if (count > (length_ - offset_)) count = length_ - offset_;
    memcpy(buf, &buffer_[offset_], count);
    offset_ += count;
    return count;
  }
  if (status_ != kOk) {
    // Already done.
    return 0;
  }
  if (count >= kFileInputIteratorBufferSize) {
    // Skip buffering; read directly into the client's buffer.
    size_t len = file_.read((uint8_t*)buf, count);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = kEndOfStream;
      return 0;
    } else if (len == ((size_t)(-1))) {
      offset_ = 0;
      length_ = 0;
      status_ = kReadError;
      return 0;
    }
    return len;
  }
  size_t len = file_.read((uint8_t*)buffer_, kFileInputIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return 0;
  } else if (len == ((size_t)(-1))) {
    offset_ = 0;
    length_ = 0;
    status_ = kReadError;
    return 0;
  }
  length_ = len;
  if (count > length_) count = length_;
  memcpy(buf, buffer_, count);
  offset_ = count;
  return count;
}

inline void ArduinoFileInputIterator::Rep::skip(size_t count) {
  if (status_ != kOk) return;
  size_t remaining = (length_ - offset_);
  if (count < remaining) {
    offset_ += count;
  } else {
    offset_ = 0;
    length_ = 0;
    if (!file_.seek(count - remaining, SeekCur)) {
      status_ = kSeekError;
    }
    if (file_.position() > file_.size()) {
      status_ = kEndOfStream;
    }
  }
}

}  // namespace roo_io
