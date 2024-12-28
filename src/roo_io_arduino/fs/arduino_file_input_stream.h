#pragma once

#include <FS.h>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

class ArduinoFileInputStream : public MultipassInputStream {
 public:
  ArduinoFileInputStream(Status error);

  // Use only if you know that the filesystem is and will remain mounted.
  ArduinoFileInputStream(fs::File file);

  ArduinoFileInputStream(std::shared_ptr<MountImpl> mount, fs::File file);

  size_t read(byte* buf, size_t count) override;

  void seek(uint64_t offset) override;

  void skip(uint64_t count) override;

  uint64_t position() const override { return file_.position(); }

  uint64_t size() override { return file_.size(); }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override;

  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  fs::File file_;
  Status status_;
};

}  // namespace roo_io