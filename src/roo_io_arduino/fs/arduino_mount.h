#pragma once

#include <FS.h>

#include <functional>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io_arduino/fs/arduino_directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class ArduinoMountImpl : public MountImpl {
 public:
  ArduinoMountImpl(FS& fs, bool read_only, std::function<void()> unmount_fn);

  bool isReadOnly() const override;

  Stat stat(const char* path) const override;

  Status remove(const char* path) override;

  Status rename(const char* pathFrom, const char* pathTo) override;

  Status mkdir(const char* path) override;

  Status rmdir(const char* path) override;

  std::unique_ptr<DirectoryImpl> opendir(const char* path) override;

  std::unique_ptr<MultipassInputStream> fopen(const char* path) override;

  std::unique_ptr<OutputStream> fopenForWrite(
      const char* path, FileUpdatePolicy update_policy) override;

  bool active() const override { return active_; }

  void deactivate() override;

 private:
  FS& fs_;
  bool active_;
  bool read_only_;
};

}  // namespace roo_io