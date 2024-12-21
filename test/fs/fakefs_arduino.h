#pragma once

#include <Arduino.h>

#include <cstring>
#include <string>

#include "FS.h"
#include "FSImpl.h"
#include "fakefs.h"
#include "gtest/gtest.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io_arduino/fs/arduino_mount.h"

namespace roo_io {
namespace fakefs {

class FakeArduinoFile : public ::fs::FileImpl {
 public:
  FakeArduinoFile() {}

  void openFile(std::string path, int basename_pos, FileStream f);

  void openDir(std::string path, int basename_pos, DirIterator dir);

  size_t write(const uint8_t* buf, size_t size) override;

  size_t read(uint8_t* buf, size_t size) override;

  void flush() override {}

  bool seek(uint32_t pos, SeekMode mode) override;

  size_t position() const override;

  size_t size() const override { return f_ == nullptr ? 0 : f_->size(); }

  bool setBufferSize(size_t size) override { return true; }

  void close() override;

  time_t getLastWrite() override { return 1; }

  const char* path() const override { return path_.c_str(); }

  const char* name() const override { return path_.c_str() + basename_pos_; }

  boolean isDirectory(void) override { return (dir_ != nullptr); }

  ::fs::FileImplPtr openNextFile(const char* mode) override;

  void rewindDirectory(void) override { dir_->rewind(); }

  operator bool() override;

 private:
  std::string path_;
  int basename_pos_;
  std::unique_ptr<FileStream> f_;
  std::unique_ptr<DirIterator> dir_;
};

class FakeArduinoFsImpl : public ::fs::FSImpl {
 public:
  FakeArduinoFsImpl(FakeFs& fs) : fs_(fs) {}

  bool exists(const char* path) override;

  bool rename(const char* pathFrom, const char* pathTo) override;

  bool remove(const char* path) override;

  bool mkdir(const char* path) override;

  bool rmdir(const char* path) override;

  ::fs::FileImplPtr open(const char* path, const char* mode,
                         const bool create) override;

 private:
  FakeFs& fs_;
  //   std::string mount_point_;
};

class FakeArduinoSdFsImpl : public ::fs::FS {
 public:
  FakeArduinoSdFsImpl(FakeFs& fake);

  bool begin(const char* mountpoint = "/sd");

  void end();
};

class FakeArduinoSdFs : public Filesystem {
 public:
  static constexpr bool strict = false;

  FakeArduinoSdFs(FakeFs& fs) : sd_(fs) {}

  MediaPresence checkMediaPresence() override { return kMediaPresent; }

  ::fs::FS& fs() { return sd_; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override { sd_.end(); }

  FakeArduinoSdFsImpl sd_;
};

}  // namespace fakefs
}  // namespace roo_io