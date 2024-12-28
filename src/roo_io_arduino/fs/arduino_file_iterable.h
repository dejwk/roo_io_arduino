#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/core/buffered_multipass_input_stream_iterator.h"
#include "roo_io/core/resource.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io_arduino/fs/arduino_file_input_iterator.h"
#include "roo_io_arduino/fs/arduino_file_resource.h"

namespace roo_io {

class ArduinoFileIterable {
 public:
  ArduinoFileIterable(fs::FS& fs, String path)
      : fs_(fs), path_(std::move(path)) {}

  ArduinoFileInputIterator iterator() const {
    return ArduinoFileInputIterator(fs_.open(path_));
  }

 private:
  FS& fs_;
  String path_;
};

// Can be used with both Arduino FS and the roo_io::Filesystem.
class ExtendedArduinoFileIterable {
 public:
  class FileIterator {
   public:
    FileIterator(std::unique_ptr<MultipassInputStream> input)
        : input_(std::move(input)), itr_(*input_) {}

    byte read() { return itr_.read(); }

    size_t read(byte* buf, size_t count) { return itr_.read(buf, count); }

    void skip(size_t count) { itr_.skip(count); }

    Status status() const { return itr_.status(); }

    uint64_t size() { return itr_.size(); }

    uint64_t position() { return itr_.position(); }

    void rewind() { itr_.rewind(); }

    void seek(uint64_t position) { itr_.seek(position); }

   private:
    std::unique_ptr<MultipassInputStream> input_;
    BufferedMultipassInputStreamIterator itr_;
  };

  ExtendedArduinoFileIterable(fs::FS& fs, String path)
      : resource_(fs, std::move(path)) {}

  ExtendedArduinoFileIterable(Filesystem& fs, std::string path)
      : resource_(fs, std::move(path)) {}

  FileIterator iterator() const {
    std::unique_ptr<MultipassInputStream> input = resource_.open();
    return FileIterator(std::move(input));
  }

 private:
  ExtendedArduinoFileResource resource_;
};

}  // namespace roo_io