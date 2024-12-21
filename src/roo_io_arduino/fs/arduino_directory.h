#pragma once

#include <memory>

#include "Arduino.h"
#include "FS.h"
#include "roo_io/fs/directory_impl.h"

namespace roo_io {

class ArduinoDirectoryImpl : public DirectoryImpl {
 public:
  ArduinoDirectoryImpl(fs::File file, Status status)
      : file_(std::move(file)), status_(status) {}

  bool close() override {
    entry_.close();
    file_.close();
    if (status_ == kOk) status_ = kClosed;
    return true;
  }

  const char* path() const override { return file_.path(); }

  // const char* name() const override { return file_.name(); }

  Status status() const override { return status_; }

  void rewind() override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    file_.rewindDirectory();
    status_ = file_ ? kOk : kUnknownIOError;
    next_ = "";
  }

  bool read(Directory::Entry& entry) override {
    if (status_ != kOk) return false;
    if (!file_) {
      status_ = kClosed;
      return false;
    }
    entry_ = file_.openNextFile();
    if (!entry_) {
      status_ = kEndOfStream;
      return false;
    }
    setEntry(entry, entry_.path(),
             strlen(entry_.path()) - strlen(entry_.name()),
             entry_.isDirectory());
    return true;
  }

 private:
  mutable fs::File file_;
  fs::File entry_;
  String next_;
  Status status_;
};

}  // namespace roo_io