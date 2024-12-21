#include "fakefs_arduino.h"
#include "fs_mount_p.h"
#include "fs_p.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io_arduino/fs/arduino_sdfs.h"

namespace roo_io {

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFsTest, FsTest, fakefs::FakeArduinoSdFs);

INSTANTIATE_TYPED_TEST_SUITE_P(ArduinoFsTest, FsMountTest,
                               fakefs::FakeArduinoSdFs);

}  // namespace roo_io
