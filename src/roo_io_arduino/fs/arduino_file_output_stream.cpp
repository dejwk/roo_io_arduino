#include "roo_io_arduino/fs/arduino_file_output_stream.h"

namespace roo_io {

ArduinoFileOutputStream::ArduinoFileOutputStream(Status error)
    : file_(), status_(error) {}

ArduinoFileOutputStream::ArduinoFileOutputStream(fs::File file)
    : ArduinoFileOutputStream(nullptr, std::move(file)) {}

ArduinoFileOutputStream::ArduinoFileOutputStream(
    std::shared_ptr<MountImpl> mount, fs::File file)
    : mount_(std::move(mount)),
      file_(std::move(file)),
      status_(file_ ? kOk : kClosed) {}

size_t ArduinoFileOutputStream::write(const byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  size_t result = file_.write((const uint8_t*)buf, count);
  if (result < count) {
    status_ = roo_io::kWriteError;
    mount_.reset();
  }
  return result;
}

void ArduinoFileOutputStream::flush() {
  if (status_ == kClosed) return;
  file_.flush();
  if (!file_) {
    status_ = kWriteError;
    mount_.reset();
  }
}

void ArduinoFileOutputStream::close() {
  mount_.reset();
  if (status_ == kClosed) return;
  file_.close();
  if (status_ == kOk) {
    status_ = kClosed;
  }
}

}  // namespace roo_io