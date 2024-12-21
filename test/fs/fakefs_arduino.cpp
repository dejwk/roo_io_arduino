#include "fakefs_arduino.h"

namespace roo_io {
namespace fakefs {

void FakeArduinoFile::openFile(std::string path, int basename_pos,
                               FileStream f) {
  path_ = std::move(path);
  basename_pos_ = basename_pos;
  f_ = std::unique_ptr<FileStream>(new FileStream(std::move(f)));
}

void FakeArduinoFile::openDir(std::string path, int basename_pos,
                              DirIterator dir) {
  path_ = std::move(path);
  basename_pos_ = basename_pos;
  dir_ = std::unique_ptr<DirIterator>(new DirIterator(std::move(dir)));
}

size_t FakeArduinoFile::write(const uint8_t* buf, size_t size) {
  return f_->write((const byte*)buf, size);
}

size_t FakeArduinoFile::read(uint8_t* buf, size_t size) {
  return f_->read((byte*)buf, size);
}

bool FakeArduinoFile::seek(uint32_t pos, SeekMode mode) {
  uint64_t position;
  switch (mode) {
    case SeekCur: {
      position = f_->position() + pos;
      break;
    }
    case SeekSet: {
      position = pos;
      break;
    }
    default: {
      position = f_->size() + pos;
      break;
    }
  }
  f_->seek(position);
  return true;
}

size_t FakeArduinoFile::position() const {
  return f_ == nullptr ? 0 : f_->position();
}

void FakeArduinoFile::close() {
  if (f_ != nullptr) {
    f_->close();
  }
}

::fs::FileImplPtr FakeArduinoFile::openNextFile(const char* mode) {
  if (!dir_->next()) {
    return ::fs::FileImplPtr(new FakeArduinoFile());
  }
  std::string next = path_ + "/" + dir_->entry().name();
  int basename_pos = path_.size() + 1;
  std::shared_ptr<FakeArduinoFile> f(new FakeArduinoFile());
  if (dir_->entry().isDir()) {
    DirIterator itr;
    itr.open(dir_->entry().dir());
    f->openDir(next, basename_pos, std::move(itr));
  } else {
    FileStream fs;
    fs.open(&dir_->entry().file(), false, false);
    f->openFile(next, basename_pos, std::move(fs));
  }
  return f;
}

FakeArduinoFile::operator bool() {
  return (f_ != nullptr && f_->isOpen()) || (dir_ != nullptr && dir_->isOpen());
}

bool FakeArduinoFsImpl::exists(const char* path) {
  auto stat = fs_.stat(path);
  return stat.status == kOk && stat.type != StatResult::kUnknown;
}

bool FakeArduinoFsImpl::rename(const char* pathFrom, const char* pathTo) {
  return fs_.rename(pathFrom, pathTo) == kOk;
}

bool FakeArduinoFsImpl::remove(const char* path) {
  return fs_.remove(path) == kOk;
}

bool FakeArduinoFsImpl::mkdir(const char* path) {
  return fs_.mkdir(path) == kOk;
}

bool FakeArduinoFsImpl::rmdir(const char* path) {
  return fs_.rmdir(path) == kOk;
}

::fs::FileImplPtr FakeArduinoFsImpl::open(const char* path, const char* mode,
                                          const bool create) {
  int flags = 0;
  if (strcmp(mode, FILE_READ) == 0) flags |= FakeFs::kRead;
  if (strcmp(mode, FILE_WRITE) == 0)
    flags |= (FakeFs::kWrite | FakeFs::kTruncate);
  if (strcmp(mode, FILE_APPEND) == 0) flags |= FakeFs::kAppend;
  std::shared_ptr<FakeArduinoFile> f(new FakeArduinoFile());
  std::string p(path);
  ResolvedPath resolved = fs_.resolvePath(path, (create && mode[0] != 'r'));
  if (resolved.status != kOk) {
    return f;
  }
  Entry* existing;
  if (resolved.basename.empty() && resolved.parent == nullptr) {
    existing = fs_.root();
  } else {
    existing = resolved.parent->dir().find(resolved.basename);
  }
  if (existing != nullptr) {
    if (existing->isFile()) {
      f->openFile(p, p.find_last_of("/") + 1, fs_.open(path, flags));
      return f;
    } else {
      DirIterator itr;
      itr.open(existing->dir());
      f->openDir(p, p.find_last_of("/") + 1, std::move(itr));
      return f;
    }
  }
  // Does not exist. Create a file if we can.
  if (mode[0] != 'r') {
    f->openFile(p, p.find_last_of("/") + 1, fs_.open(path, flags));
  }
  return f;
}

FakeArduinoSdFsImpl::FakeArduinoSdFsImpl(FakeFs& fake)
    : ::fs::FS(
          std::shared_ptr<FakeArduinoFsImpl>(new FakeArduinoFsImpl(fake))) {}

bool FakeArduinoSdFsImpl::begin(const char* mountpoint) {
  _impl->mountpoint(mountpoint);
  return true;
}

void FakeArduinoSdFsImpl::end() { _impl->mountpoint(nullptr); }

MountImpl::MountResult FakeArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin()) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new ArduinoMountImpl(sd_, false, unmount_fn)));
}

}  // namespace fakefs
}  // namespace roo_io