#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/core/resource.h"
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

}  // namespace roo_io