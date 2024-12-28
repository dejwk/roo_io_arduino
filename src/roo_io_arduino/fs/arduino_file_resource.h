#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/core/resource.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io_arduino/fs/arduino_file_input_stream.h"

namespace roo_io {

class ArduinoFileResource : public MultipassResource {
 public:
  ArduinoFileResource(fs::FS& fs, String path)
      : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<MultipassInputStream> open() const override {
    return std::unique_ptr<MultipassInputStream>(
        new ArduinoFileInputStream(fs_.open(path_)));
  }

 private:
  FS& fs_;
  String path_;
};

// Can act both as an Arduino file resource, or as a roo_io::Filesystem-based
// file resource.
class ExtendedArduinoFileResource : public roo_io::MultipassResource {
 public:
  ExtendedArduinoFileResource(fs::FS& fs, std::string path)
      : fs_(nullptr), arduino_fs_(&fs), path_(std::move(path)) {}

  ExtendedArduinoFileResource(roo_io::Filesystem& fs, std::string path)
      : fs_(&fs), arduino_fs_(nullptr), path_(std::move(path)) {}

  std::unique_ptr<roo_io::MultipassInputStream> open() const override {
    return fs_ != nullptr ? fs_->mount().fopen(path_.c_str())
                          : std::unique_ptr<roo_io::MultipassInputStream>(
                                new roo_io::ArduinoFileInputStream(
                                    arduino_fs_->open(path_.c_str())));
  }

 private:
  ::fs::FS* arduino_fs_;
  roo_io::Filesystem* fs_;
  std::string path_;
};

}  // namespace roo_io