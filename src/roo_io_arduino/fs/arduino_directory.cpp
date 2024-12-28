#include "roo_io_arduino/fs/arduino_directory.h"

namespace roo_io {

ArduinoDirectoryImpl::ArduinoDirectoryImpl(std::shared_ptr<MountImpl> mount,
                                           fs::File file, Status status)
    : mount_(std::move(mount)), file_(std::move(file)), status_(status) {}

bool ArduinoDirectoryImpl::close() {
  mount_.reset();
  entry_.close();
  file_.close();
  if (status_ == kOk) status_ = kClosed;
  return true;
}

void ArduinoDirectoryImpl::rewind() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  file_.rewindDirectory();
  if (file_) {
    status_ = kOk;
  } else {
    status_ = kUnknownIOError;
    mount_.reset();
  }
  next_ = "";
}

bool ArduinoDirectoryImpl::read(Directory::Entry& entry) {
  if (status_ != kOk) return false;
  if (!file_) {
    status_ = kClosed;
    mount_.reset();
    return false;
  }
  entry_ = file_.openNextFile();
  if (!entry_) {
    status_ = kEndOfStream;
    return false;
  }
  setEntry(entry, entry_.path(), strlen(entry_.path()) - strlen(entry_.name()),
           entry_.isDirectory());
  return true;
}

}  // namespace roo_io