#include "roo_io_arduino/fs/arduino_file_input_stream.h"

namespace roo_io {

ArduinoFileInputStream::ArduinoFileInputStream(Status error)
    : file_(), status_(error) {}

ArduinoFileInputStream::ArduinoFileInputStream(fs::File file)
    : ArduinoFileInputStream(nullptr, std::move(file)) {}

ArduinoFileInputStream::ArduinoFileInputStream(std::shared_ptr<MountImpl> mount,
                                               fs::File file)
    : mount_(std::move(mount)),
      file_(std::move(file)),
      status_(file_ ? kOk : kClosed) {}

size_t ArduinoFileInputStream::read(byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  size_t result = file_.read((uint8_t*)buf, count);
  if (result == 0) {
    status_ = kEndOfStream;
    return 0;
  } else if (result == ((size_t)(-1))) {
    // Indicates an error.
    status_ = kReadError;
    mount_.reset();
    return 0;
  }
  return result;
}

void ArduinoFileInputStream::seek(uint64_t offset) {
  if (status_ != kOk && status_ != kEndOfStream) return;
  if (file_.seek(offset)) {
    status_ = kOk;
  } else {
    status_ = kSeekError;
    mount_.reset();
  }
}

void ArduinoFileInputStream::skip(uint64_t count) {
  if (count < 64) {
    byte buf[count];
    readFully(buf, count);
    return;
  }
  if (!file_.seek(count, SeekCur)) {
    status_ = kSeekError;
    mount_.reset();
    return;
  }
  if (file_.position() > file_.size()) {
    status_ = kEndOfStream;
  }
}

void ArduinoFileInputStream::close() {
  file_.close();
  mount_.reset();
  status_ = kClosed;
}

}  // namespace roo_io