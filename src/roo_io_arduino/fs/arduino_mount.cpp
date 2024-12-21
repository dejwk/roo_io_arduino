#include "roo_io_arduino/fs/arduino_mount.h"

#include "roo_io_arduino/fs/arduino_file_input_stream.h"
#include "roo_io_arduino/fs/arduino_file_output_stream.h"

namespace roo_io {

namespace {

Status CheckParentage(FS& fs, const char* path) {
  std::unique_ptr<char[]> dup_path(strdup(path));
  size_t pos = 0;
  while (true) {
    while (dup_path[pos] == '/') ++pos;
    while (dup_path[pos] != '/') {
      if (dup_path[pos] == 0) {
        // We ignore the type and existence of the last segment, since our job
        // is only to check parentage.
        return kOk;
      }
      ++pos;
    }
    dup_path[pos] = 0;
    {
      fs::File f = fs.open(dup_path.get());
      if (!f) return kNotFound;
      if (!f.isDirectory()) return kNotDirectory;
    }
    dup_path[pos] = '/';
    ++pos;
  }
}

}  // namespace

ArduinoMountImpl::ArduinoMountImpl(FS& fs, bool read_only,
                                   std::function<void()> unmount_fn)
    : MountImpl(unmount_fn), fs_(fs), active_(true), read_only_(read_only) {}

bool ArduinoMountImpl::isReadOnly() const { return read_only_; }

Stat ArduinoMountImpl::stat(const char* path) const {
  if (path == nullptr || path[0] != '/') {
    return Stat(kInvalidPath);
  }
  if (!active_) return Stat(kNotMounted);
  if (!fs_.exists(path)) return Stat(kNotFound);
  fs::File f = fs_.open(path);
  if (!f) return Stat(kNotFound);
  return f.isDirectory() ? Stat(Stat::kDir, 0) : Stat(Stat::kFile, f.size());
}

Status ArduinoMountImpl::remove(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (!active_) return kNotMounted;
  if (read_only_) return kReadOnlyFilesystem;
  {
    fs::File f = fs_.open(path);
    if (!f) return kNotFound;
    if (f.isDirectory()) return kNotFile;
  }
  return fs_.remove(path) ? kOk : kUnknownIOError;
}

Status ArduinoMountImpl::rename(const char* pathFrom, const char* pathTo) {
  if (read_only_) return kReadOnlyFilesystem;
  if (fs_.rename(pathFrom, pathTo)) return kOk;
  Stat src = stat(pathFrom);
  if (!src.exists()) {
    return src.status();
  }
  Stat dst = stat(pathTo);
  if (dst.exists()) return dst.isDirectory() ? kDirectoryExists : kFileExists;
  if (dst.status() != kNotFound) return dst.status();
  if (strncmp(pathFrom, pathTo, strlen(pathFrom)) == 0) {
    return kInvalidPath;
  }
  // Check if the destination directory exists.
  std::unique_ptr<char[]> dup(strdup(pathTo));
  char* last_slash = strrchr(dup.get(), '/');
  if (last_slash != nullptr) {
    *last_slash = 0;
    Stat dst_dir = stat(dup.get());
    if (dst_dir.status() == kNotFound) return kNotFound;
    if (!dst_dir.isDirectory()) return kNotDirectory;
  }
  return kUnknownIOError;
}

Status ArduinoMountImpl::mkdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (!active_) return kNotMounted;
  if (read_only_) return kReadOnlyFilesystem;
  if (fs_.mkdir(path)) return kOk;
  if (fs_.exists(path)) {
    fs::File f = fs_.open(path);
    if (f) {
      return f.isDirectory() ? kDirectoryExists : kFileExists;
    } else {
      return kUnknownIOError;
    }
  }
  Status status = CheckParentage(fs_, path);
  if (status != kOk) return status;
  return kUnknownIOError;
}

Status ArduinoMountImpl::rmdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (!active_) return kNotMounted;
  if (read_only_) return kReadOnlyFilesystem;
  {
    fs::File f = fs_.open(path);
    if (!f) return kNotFound;
    if (!f.isDirectory()) return kNotDirectory;
    if (fs_.rmdir(path)) return kOk;

    fs::File child;
    do {
      child = f.openNextFile();
    } while (child && (strcmp(child.name(), ".") == 0 ||
                       strcmp(child.name(), "..") == 0));
    if (child) return kDirectoryNotEmpty;
  }
  return kUnknownIOError;
}

std::unique_ptr<DirectoryImpl> ArduinoMountImpl::opendir(const char* path) {
  if (!active_) return DirectoryError(kNotMounted);
  fs::File f = fs_.open(path, "r");
  roo_io::Status status = roo_io::kOk;
  if (!f) {
    if (!fs_.exists(path)) {
      status = roo_io::kNotFound;
    } else {
      status = roo_io::kOpenError;
    }
  }
  return std::unique_ptr<DirectoryImpl>(
      new ArduinoDirectoryImpl(std::move(f), status));
}

std::unique_ptr<MultipassInputStream> ArduinoMountImpl::fopen(
    const char* path) {
  if (path == nullptr || path[0] != '/') {
    return InputError(kInvalidPath);
  }
  if (!active_) return InputError(kNotMounted);
  fs::File f = fs_.open(path, "r");
  if (!f) {
    if (!fs_.exists(path)) {
      return InputError(kNotFound);
    } else {
      return InputError(kOpenError);
    }
  }
  if (f.isDirectory()) {
    return InputError(kNotFile);
  }
  return std::unique_ptr<MultipassInputStream>(
      new ArduinoFileInputStream(std::move(f)));
}

std::unique_ptr<OutputStream> ArduinoMountImpl::fopenForWrite(
    const char* path, FileUpdatePolicy update_policy) {
  if (path == nullptr || path[0] != '/') {
    return OutputError(kInvalidPath);
  }
  if (!active_) return OutputError(kNotMounted);
  if (read_only_) {
    return OutputError(kReadOnlyFilesystem);
  }
  fs::File f;
  if (update_policy == kFailIfExists) {
    if (fs_.exists(path)) {
      f = fs_.open(path);
      return OutputError(f.isDirectory() ? kDirectoryExists : kFileExists);
    }
    f = fs_.open(path, "w");
  } else {
    // Try to just open, but if it fails, check if not a directory to return a
    // more specific error.
    f = fs_.open(path, update_policy == kTruncateIfExists ? "w" : "a");
    if (!f && fs_.exists(path)) {
      f = fs_.open(path);
      if (f.isDirectory()) {
        return OutputError(kNotFile);
      }
      return OutputError(kOpenError);
    }
  }
  if (!f) {
    return OutputError(kOpenError);
  }
  return std::unique_ptr<OutputStream>(
      new ArduinoFileOutputStream(std::move(f)));
}

void ArduinoMountImpl::deactivate() { active_ = false; }

}  // namespace roo_io