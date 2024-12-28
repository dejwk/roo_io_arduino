#pragma once

#include <memory>

#include "Arduino.h"
#include "FS.h"
#include "roo_io/fs/directory_impl.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

class ArduinoDirectoryImpl : public DirectoryImpl {
 public:
  ArduinoDirectoryImpl(std::shared_ptr<MountImpl> mount, fs::File file,
                       Status status);

  bool close() override;

  const char* path() const override { return file_.path(); }

  // const char* name() const override { return file_.name(); }

  Status status() const override { return status_; }

  void rewind() override;

  bool read(Directory::Entry& entry) override;

 private:
  std::shared_ptr<MountImpl> mount_;
  mutable fs::File file_;
  fs::File entry_;
  String next_;
  Status status_;
};

}  // namespace roo_io