#pragma once

#include <FS.h>

#include "roo_io/core/output_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

class ArduinoFileOutputStream : public OutputStream {
 public:
  ArduinoFileOutputStream(Status error);

  // Use only if you know that the filesystem is and will remain mounted.
  ArduinoFileOutputStream(fs::File file);

  ArduinoFileOutputStream(std::shared_ptr<MountImpl> mount, fs::File file);

  size_t write(const byte* buf, size_t count) override;

  void flush() override;

  void close() override;

  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  fs::File file_;
  Status status_;
};

}  // namespace roo_io